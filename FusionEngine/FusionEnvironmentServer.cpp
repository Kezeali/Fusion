
#include "FusionEnvironmentServer.h"

/// Fusion
#include "FusionShipResponse.h"
#include "FusionShipDrawable.h"
#include "FusionShipEngine.h"
#include "FusionShipHealth.h"

using namespace FusionEngine;

ServerEnvironment::ServerEnvironment(const std::string &port, ServerOptions *options)
: FusionState(true),
m_Port(port),
m_Options(options)
{
	m_PhysicsWorld = new FusionPhysicsWorld();
	m_NetworkManager = new FusionNetworkServer(port, options);
	m_Scene = new FusionScene();

	m_FrameTime = (int)(1000/options->mMaxFPS);
}

ServerEnvironment::~ServerEnvironment()
{
	delete m_PhysicsWorld;
	delete m_NetworkManager;
	delete m_Scene;
}

bool ServerEnvironment::Initialise()
{
	return true;
}

bool ServerEnvironment::Update(unsigned int split)
{
	// Tells the game to abort the client environment (by returning false)
	if (m_Abort)
		return false;

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
}

void ServerEnvironment::Draw()
{
}

void ServerEnvironment::CleanUp()
{

}

void ServerEnvironment::CreateShip(const ShipState &state)
{
	// Create the main node
	FusionNode *node = m_Scene->CreateNode();

	// Get the resource for the ship
	ShipResource *res = ResourceLoader::getSingletonPtr()->GetLoadedShips()[m_ShipResources[state.PID]];

	// Create children
	//  Engines
	if (state.engines & LEFT)
		FusionNode *node_len = node->CreateChildNode(res->Positions.LeftEngine);
	if (state.engines & RIGHT)
		FusionNode *node_ren = node->CreateChildNode(res->Positions.RightEngine);
	//  Weapons
	FusionNode *node_priw = node->CreateChildNode(res->Positions.PrimaryWeapon);
	FusionNode *node_secw = node->CreateChildNode(res->Positions.SecondaryWeapon);

	// Create the physical body (without collision handler, as
	//  the ship will add itself as the collision handler on construction)
	FusionPhysicsBody *physbod = new FusionPhysicsBody(m_PhysicsWorld);
	m_PhysicsWorld->AddBody(physbod);

	// Create a ship container and map it to the given PID
	m_Ships.insert(
		ShipList::value_type(state.PID, new FusionShip(state, physbod, node))
		);

	m_NumPlayers += 1;
}

void ServerEnvironment::send()
{
	// Message per second limiter:
	if (CL_System::get_time() > m_MessageDelay)
	{
		// It's been one second, so allow more messages in the next second:
		m_MessageDelay = CL_System::get_time() + 1000;
		m_MessagesSent = 0;
	}
	// Don't send more messages than the client's network settings allow
	if (m_MessagesSent < m_Options->mNetworkOptions.mMaxMessageRate)
	{

		ShipList::iterator it = m_Ships.begin();
		for (; it != m_Ships.end; ++it)
		{
			// Send all ship states
			//  Check whether an update is necessary for the current ship:
			if ((*it)->StateHasChanged())
			{
				// _stateSynced() makes StateHasChanged() return false until the state
				//  has actually changed from what it is now:
				(*it)->_stateSynced(); 
				m_MessagesSent++;

				FusionMessage *m = MessageBuilder::BuildMessage(
					(*it)->GetShipState(), (*it)->GetShipState().PID
					);
				m_NetworkManager->QueueMessage(m, CID_GAME);
			}

			// ... And all input states
			//  Check whether an update is necessary for the current ship:
			if ((*it)->InputHasChanged())
			{
				(*it)->_inputSynced(); 
				m_MessagesSent++;

				FusionMessage *m = MessageBuilder::BuildMessage(
					(*it)->GetInputState(), (*it)->GetShipState().PID
					);
				m_NetworkManager->QueueMessage(m, CID_GAME);
			}
		}

	}

	//! \todo GUI_Chat
	// In GUI_Chat: (pass the network man to GUI_Chat on construction)
	//  m_NetworkManager->QueueMessage(m, CID_CHAT);
}

bool ServerEnvironment::receive()
{
	// Check events (important messages)
	FusionMessage *e = m_NetworkManager->GetNextEvent();
	while (e)
	{
		const unsigned char type = e->GetType();
		switch (type)
		{
		case ID_REMOTE_CONNECTION_LOST:
			e->GetObjectID();
			break;
		}

		e = m_NetworkManager->GetNextEvent();
	}

	// System messages
	{
		FusionMessage *m = m_NetworkManager->GetNextMessage(CID_SYSTEM);
		while (m)
		{
			const unsigned char type = m->GetType();
			switch (type)
			{
			case MTID_NEWPLAYER:
				// It shouldn't matter if someone's sitting on this spawn, they'll
				//  just get pushed out of the way / destroyed.
				ShipState state = getSpawnState(getNextPID());

				CreateShip(state);

				ObjectID oldPID;
				RakNet::BitStream bs(m->ReadWithoutHeader(), m->GetDataLength(), false);
				
				bs.Read(oldPID);

				m_NetworkManager->SendAddPlayer(oldPID, state);
				break;
			}
		}

		m = m_NetworkManager->GetNextMessage(CID_SYSTEM);
	}

	// Gameplay Messages
	{
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
	}

	return true;
}

ObjectID ServerEnvironment::getNextPID()
{
	return (ObjectID)(m_NextPID++);
}

ObjectID ServerEnvironment::getNextOID()
{
	return (ObjectID)(m_NextOID++);
}