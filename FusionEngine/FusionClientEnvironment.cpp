
#include "FusionClientEnvironment.h"

using namespace FusionEngine;

ClientEnvironment::ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options)
: m_Options(options)
{
	m_InputManager = new FusionInput((*m_Options));
	m_NetworkManager = new FusionNetworkClient(hostname, port, options);
	m_Scene = new FusionScene();
}

ClientEnvironment::~ClientEnvironment()
{
	delete m_InputManager;
	delete m_NetworkManager;
	delete m_Scene;
}

bool ClientEnvironment::Initialise(ResourceLoader *resources)
{
	m_InputManager->Initialise();

	// Not doing threading any more, cause that's the way we roll
	//m_NetManThread = new CL_Thread(m_NetworkManager, false);
	//m_NetManThread->start();

	m_ShipResources = resources->GetLoadedShips();
	return true;
}

bool ClientEnvironment::Update(unsigned int split)
{
	// Tells the game to exit the client environment (by returning false)
	if (m_Quit)
		return false;

	// Setup local frames
	gatherLocalInput();

	// Update the states of all local objects based on the gathered input
	updateAllPositions(split);

	// Check the network for packets
	m_NetworkManager->run();

	// Send everything in the message queue
	send();

	// Read any frames / messages received between this update and the last
	if (receive())
	{
		// Build all local frames
		//buildFrames();
		// Update the states of all sync'ed objects if any remote frames were received
		//  (this code will also check the time on each frame individually to make sure it has changed)
		updateShipStates();
	}

	// Move/rotate ships based on the received/predicted frames
	//updateSceneGraph();

	// Update all the client only stuff (particle systems, falling engines, etc.)
	//updateNonSynced();

	//! \todo ClientEnvironment#Update() and ServerEnvironment#Update() should return false,
	//! or perhaps an error, when update fails... Or perhaps I should use exceptions here?
	return true;
}

void ClientEnvironment::Draw()
{
	m_Scene->Draw();
}

ShipResource *ClientEnvironment::GetShipResourceByID(std::string id) const
{
	//! \todo catch errors.
	return m_ShipResources[id];
}

void ClientEnvironment::_quit(ErrorType type)
{
	m_Quit = true;
	// TODO: Call a method to set the LastError property here
	//  (so FusionGame can read the property, and display
	//  an error message for the user.)
}

void ClientEnvironment::send()
{
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
			// Perhaps we should show a connection lost message here?
			_quit(UNEXPECTEDDISCONNECT);
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
}

void ClientEnvironment::installShipFrameFromMessage(FusionMessage *m)
{
    ShipState state;
    PlayerInd pid; // unsigned int?

	RakNet::BitStream bs(m->Read(), m->GetLength(), false);

    // Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
    bs.Read(pid);

	bs.Read(state.Position.x);
	bs.Read(state.Position.y);

	bs.Read(state.Velocity.x);
	bs.Read(state.Velocity.y);

	bs.Read(state.Rotation);
	bs.Read(state.RotationalVelocity);

	bs.Read(state.current_primary);
	bs.Read(state.current_secondary);
	bs.Read(state.current_bomb);

	bs.Read(state.engines);
	bs.Read(state.weapons);

	m_Ships[pid]->SetShipState(state);
}

void ClientEnvironment::installShipInputFromMessage(FusionMessage *m)
{
    ShipInput state;
    PlayerInd pid; // unsigned int?

	RakNet::BitStream bs(m->Read(), m->GetLength(), false);

    // Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
    bs.Read(pid);

	m_Ships[pid]->SetInputState(state);
}

void ClientEnvironment::installProjectileFrameFromMessage(FusionMessage *m)
{
}

void ClientEnvironment::gatherLocalInput()
{
	ShipInputList ship_input = m_InputManager->GetAllShipInputs();

	for (unsigned int i = 0; i < m_Options->NumPlayers; i++)
	{
		m_Ships[i]->SetInputState(ship_input[i]);
	}
}

void ClientEnvironment::updateShipStates()
{
}

void ClientEnvironment::updateAllPositions(unsigned int split)
{
	m_PhysicsWorld->RunSimulation(split);
}

//void ClientEnvironment::updateSceneGraph()
//{
//}

// IGNORE THE FOLLOWING CODE, the scene now draws everthing!
//
//void ClientEnvironment::drawLevel()
//{
//}

//void ClientEnvironment::drawShip(FusionShip ship)
//{

//
//	ShipResource *res = m_ShipResources[ship.ResourceID];
//	Node
//
//	res->images.Body.GetImage()->draw(
//		positions.Body.x,
//		positions.Body.y);
//
//	res->images.LeftEngine->draw(
//		positions.LeftEngine.x,
//		positions.LeftEngine.y);
//
//	res->images.RightEngine->draw(
//		positions.RightEngine.x,
//		positions.RightEngine.y);
//
//	res->images.PrimaryWeapon->draw(
//		positions.PrimaryWeapon.x,
//		positions.PrimaryWeapon.y);
//
//	res->images.SecondaryWeapon->draw(
//		positions.SecondaryWeapon.x,
//		positions.SecondaryWeapon.y);
//
//    ;
//}
