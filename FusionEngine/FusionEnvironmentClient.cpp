
#include "FusionEnvironmentClient.h"

/// Fusion
#include "FusionStatePackSync.h"
#include "FusionShipResponse.h"
#include "FusionShipDrawable.h"
#include "FusionShipEngine.h"
#include "FusionShipHealth.h"
/// Gui states
#include "FusionEngineGUI_Options.h"
#include "FusionEngineGUI_Console.h"

using namespace FusionEngine;

ClientEnvironment::ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options)
: m_Hostname(hostname),
m_Port(port),
m_Options(options)
{
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
	// Setup and run the package syncroniser
	PackSyncState *ps = new PackSyncState();
	ps->MakeClient(m_Hostname, m_Port);
	_pushMessage(new StateMessage(StateMessage::ADDSTATE, ps));

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

	// Show menu
	if (FusionInput::getSingleton().GetGlobalInputs().menu)
	{
		// Add the options state
		_pushMessage(new StateMessage(StateMessage::ADDSTATE, new GUI_Options()));
	}
	// Show console
	if (FusionInput::getSingleton().GetGlobalInputs().console)
	{
		// Add the console state
		_pushMessage(new StateMessage(StateMessage::ADDSTATE, new GUI_Console()));
	}

	// Setup local frames
	gatherLocalInput();

	// Update the states of all local objects based on the gathered input
	updateAllPositions(split);

	// Check the network for packets
	m_NetworkManager->run();

	// Send everything in the message queue
	send();

	// Read any frames / messages received between this update and the last
	if (!receive())
		return false;

	m_Scene->UpdateDynamics(split);

	//! \todo ClientEnvironment#Update() and ServerEnvironment#Update() should return false,
	//! or perhaps an error, when update fails... Or perhaps I should use exceptions here?
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
	ShipResource *res = m_ShipResources[m_PlayerResourceIds[state.PID]];

	// Create children and their drawables
	//  Engines
	if (state.engines & LEFT)
	{
		FusionNode *node_len = node->CreateChildNode(res->Positions.LeftEngine);
		// Attach Drawable
		FusionShipEngine *draw = new FusionShipEngine;
		draw->SetImage(res->Images.LeftEngine);
		node_len->AttachDynamicDrawable(draw);
	}
	if (state.engines & RIGHT)
	{
		FusionNode *node_ren = node->CreateChildNode(res->Positions.RightEngine);
		// Attach Drawable
		FusionShipEngine *draw = new FusionShipEngine;
		draw->SetImage(res->Images.RightEngine);
		node_ren->AttachDynamicDrawable(draw);
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
	FusionPhysicsBody *pbod = new FusionPhysicsBody(m_PhysicsWorld, new FusionShipResponse);
	m_PhysicsWorld->AddBody(pbod);

	// Create a ship and add it to the list
	m_Ships.push_back(new FusionShip(state, pbod, node));
}

void ClientEnvironment::send()
{
	// Send local ship state
	for (unsigned int i =0; i<m_Options->NumPlayers; i++)
	{
		FusionMessage *m = FusionMessageBuilder::BuildMessage(
			m_Ships[m_PlayerIDs[i]]->GetShipState(), m_PlayerIDs[i]
			);
		m_NetworkManager->QueueMessage(m, CID_GAME);
	}
	// And local input state
	for (unsigned int i =0; i<m_NumPlayers; i++)
	{
		FusionMessage *m = FusionMessageBuilder::BuildMessage(
			m_Ships[m_PlayerIDs[i]]->GetInputState(), m_PlayerIDs[i]
			);
		m_NetworkManager->QueueMessage(m, CID_GAME);
	}

	//! \todo chat
	//m_NetworkManager->QueueMessage(m, CID_CHAT);
}

bool ClientEnvironment::receive()
{
	// Check events (important messages)
	FusionMessage *e = m_NetworkManager->GetNextEvent();

	while (e)
	{
		const unsigned char type = e->GetType();
		switch (type)
		{
		case ID_REMOTE_CONNECTION_LOST:
			_error(new Error(Error::UNEXPECTEDDISCONNECT, "Remote Connection Lost"));
			break;
		}

		e = m_NetworkManager->GetNextEvent();
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
