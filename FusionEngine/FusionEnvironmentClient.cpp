
#include "FusionEnvironmentClient.h"

#include "FusionEngineGUI_Options.h"

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

	//! \todo START FILE SYNC STATE HERE!!! (that will load verified ships)

	return true;
}

bool ClientEnvironment::Update(unsigned int split)
{
	// Tells the game to exit the client environment (by returning false)
	if (m_Quit)
		return false;

	// Show menu
	if (m_InputManager->GetGlobalInputs().menu)
	{
		// Add the options state, and tell it where to find the InputManager
		//  (this is only a temporary hack, until I bother to make ImputManager a singleton.)
		_pushMessage(new StateMessage(StateMessage::ADDSTATE, new GUI_Options(m_InputManager)));
	}
	// Show console
	if (m_InputManager->GetGlobalInputs().console)
	{
		// Add the console state, and tell it where to find the InputManager
		//  (this is only a temporary hack, until I bother to make ImputManager a singleton.)
		_pushMessage(new StateMessage(StateMessage::ADDSTATE, new GUI_Console(m_InputManager)));
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

ShipResource *ClientEnvironment::GetShipResourceByID(const std::string &id)
{
	//! \todo catch errors.
	return m_ShipResources[id];
}

void ClientEnvironment::send()
{
	FusionMessage *m = FusionMessageBuilder::BuildMessage((*it)->m_CurrentState, m_PlayerID);
	m_NetworkManager->QueueMessage(m, CID_GAME);

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
			_quit(new Error(Error::UNEXPECTEDDISCONNECT, "Remote Connection Lost"));
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
	ShipInputList ship_input = m_InputManager->GetAllShipInputs();

	for (unsigned int i = 0; i < m_Options->NumPlayers; i++)
	{
		m_Ships[i]->SetInputState(ship_input[i]);
	}
}
