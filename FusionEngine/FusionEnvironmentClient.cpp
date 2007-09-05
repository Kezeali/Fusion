/*
  Copyright (c) 2006-2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#include "FusionEnvironmentClient.h"

/// Fusion
#include "FusionPhysicsCallback.h"
#include "FusionShipDrawable.h"
#include "FusionShipEngine.h"
#include "FusionShipHealth.h"
/// Gui states
#include "FusionEngineGUI_Options.h"
#include "FusionEngineGUI_Console.h"

namespace FusionEngine
{

	ClientEnvironment::ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options)
		: FusionState(true), 
		m_Hostname(hostname),
		m_Port(port),
		m_Options(options),
		m_FrameTime(g_DefaultFrameTime)
	{
		new ResourceManager();
		new FusionInput(m_Options); // initialises the fusion input singleton
		m_NetworkManager = new FusionNetworkClient(hostname, port, options);
		m_Scene = new FusionScene();
	}

	ClientEnvironment::~ClientEnvironment()
	{
		delete FusionInput::getSingletonPtr();
		delete m_NetworkManager;
		delete m_Scene;
	}

	bool ClientEnvironment::Initialise()
	{
		// Particle system
		L_ParticleSystem::init();

		// Input manager
		FusionInput::getSingleton().Initialise();
		FusionInput::getSingleton().Test();

		// Not doing threading any more, 'cause that's just how we roll
		//m_NetManThread = new CL_Thread(m_NetworkManager, false);
		//m_NetManThread->start();

		return true;
	}

	bool ClientEnvironment::Update(unsigned int split)
	{
		// Tells the game to abort the client environment (by returning false)
		if (m_Abort)
			return false;

		// Don't do anything till we're going slow enough
		limitFrames();

		// Show menu
		if (FusionInput::getSingleton().IsButtonDown("global.menu"))
		{
			// Add the options state
			_pushMessage(new StateMessage(StateMessage::ADDSTATE, new Menu()));
		}
		// Show console
		if (FusionInput::getSingleton().IsButtonDown("global.console"))
		{
			// Add the console state
			_pushMessage(new StateMessage(StateMessage::ADDSTATE, new ConsoleGUI()));
		}

		// Check the network for packets
		m_NetworkManager->run();

		// Read any messages received between this update and the last
		receive();

		// Setup local inputs
		gatherInput();

		// Run the simulation (iterates through all entity states since 
		//  the last verified)
		simulate(split);

		// Send the final states to the server
		send();


		return true;


		// Move/rotate ships based on the received/predicted frames
		//updateSceneGraph();
		// Update all the client only stuff (particle systems, falling engines, etc.)
		//updateNonSynced();
	}

	void ClientEnvironment::Draw()
	{
		m_Scene->Draw();
	}

	void ClientEnvironment::CleanUp()
	{
		// Particle system
		L_ParticleSystem::deinit();
	}

	void ClientEnvironment::CreateShip(const ShipState &state)
	{
		// Create the main node
		FusionNode *node = m_Scene->CreateNode();

		// Get the resource for the ship
		ShipResourceBundle *res = m_ShipResources[m_PlayerResourceIds[state.PID]];

		// Create children and their drawables
		//  Engines
		if (state.engines & LEFT)
		{
			FusionNode *node_len = node->CreateChildNode(res->Positions.LeftEngine);
			// Attach Drawable
			FusionShipEngine *draw = new FusionShipEngine;
			draw->SetImage(res->Images.LeftEngine);
			node_len->AttachDrawable(draw);
		}
		if (state.engines & RIGHT)
		{
			FusionNode *node_ren = node->CreateChildNode(res->Positions.RightEngine);
			// Attach Drawable
			FusionShipEngine *draw = new FusionShipEngine;
			draw->SetImage(res->Images.RightEngine);
			node_ren->AttachDrawable(draw);
		}
		//  Weapons
		FusionNode *node_priw = node->CreateChildNode(res->Positions.PrimaryWeapon);
		//! \todo Weapon drawables
		FusionNode *node_secw = node->CreateChildNode(res->Positions.SecondaryWeapon);


		// Create and attach main drawables
		//  Ship
		FusionShipDrawable *draw_ship = new FusionShipDrawable;
		draw_ship->SetImage(res->Images.Body);
		//  Health
		ShipHealthDrawable *draw_health = new ShipHealthDrawable;
		draw_health->SetHealth(state.health);
		draw_health->SetMax(m_MaxHealth);
		draw_health->SetWidth(res->Images.Body->get_width());

		//  Attach
		node->AttachDrawable(draw_ship);
		node->AttachDynamicDrawable(draw_health);	

		// Create the physical body
		FusionPhysicsBody *pbod = new FusionPhysicsBody(m_PhysicsWorld);
		m_PhysicsWorld->AddBody(pbod);

		// Create a ship and add it to the list
		m_Ships.push_back(new FusionShip(state, pbod, node));
	}

	void ClientEnvironment::send()
	{
		// Don't send more messages than the client's network settings allow
		if (m_NetworkManager->SendAllowed())
		{

			for (unsigned int i =0; i<m_Options->NumPlayers; i++)
			{
				FusionShip* currentShip = m_Ships[m_SystemAddresses[i]];

				// Send local ship states
				// -Check whether an update is necessary for the current ship:
				if (currentShip->StateHasChanged())
				{
					// _stateSynced() makes StateHasChanged() return false until the state
					//  has actually changed from what it is now:
					currentShip->_stateSynced(); 

					m_NetworkManager->SendShipState(currentShip->GetShipState());
				}

				// ... And local input states
				//  Check whether an update is necessary for the current ship:
				if (m_Ships[m_SystemAddresses[i]]->InputHasChanged())
				{
					m_Ships[m_SystemAddresses[i]]->_inputSynced(); 
					m_MessagesSent++;

					FusionMessage *m = MessageBuilder::BuildMessage(
						m_Ships[m_SystemAddresses[i]]->GetInputState(), m_SystemAddresses[i]
					);
					m_NetworkManager->QueueMessage(m, CID_GAME);
				}
			}

		}

		//! \todo GUI_Chat
		// In GUI_Chat: (pass the network man to GUI_Chat on construction)
		//  m_NetworkManager->QueueMessage(m, CID_CHAT);
	}

	bool ClientEnvironment::receive()
	{
		// Check events (important messages)
		Packet *e = m_NetworkManager->PopNextEvent();

		while (e)
		{
			const unsigned char type = e->GetType();
			switch (type)
			{
			case ID_REMOTE_CONNECTION_LOST:
				throw Exception(ExceptionType::NETWORK, "Remote Connection Lost");
				break;
			}

			e = m_NetworkManager->PopNextEvent();
		}

		// Gameplay Messages
		FusionMessage *m = m_NetworkManager->GetNextMessage(CID_GAME);
		while (m)
		{
			const unsigned char type = m->GetType();
			switch (type)
			{
			case MTID_SHIPFRAME:
				// I call another method here, because, well... because that's how I roll ;)
				installShipFrameFromMessage(m);
				break;
			}

			m = m_NetworkManager->GetNextMessage(CID_GAME);
		}

		//! \todo System, etc. messages in the ClientEnvironment
		return true;
	}

	void ClientEnvironment::gatherLocalInput()
	{
		ShipInputList ship_input = FusionInput::getSingleton().GetAllShipInputs();

		for (unsigned int i = 0; i < m_Options->NumPlayers; i++)
		{
			m_Ships[i]->SetInputState(ship_input[i]);
		}
	}


	ObjectID ClientEnvironment::getNextPID()
	{
		return (ObjectID)(m_NextPID++);
	}

	ObjectID ClientEnvironment::getNextOID()
	{
		return (ObjectID)(m_NextOID++);
	}

}
