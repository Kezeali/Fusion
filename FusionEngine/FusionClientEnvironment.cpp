
#include "FusionClientEnvironment.h"

using namespace FusionEngine;

ClientEnvironment::ClientEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options)
: m_Options(options)
{
	m_InputManager = new FusionInput((*m_Options));
	m_NetworkManager = new FusionNetworkClient(hostname, port, options);
	m_Scene = new FusionScene();
}

bool ClientEnvironment::Initialise(ResourceLoader *resources)
{
	m_InputManager->Initialise();

	m_NetManThread = new CL_Thread(m_NetworkManager, false);
	m_NetManThread->start();

	m_ShipResources = resources->GetLoadedShips();
	return true;
}

void ClientEnvironment::Update(unsigned int split)
{
	// Setup local frames
	gatherLocalInput();

	// Update the states of all local objects based on the gathered input
	updateAllPositions(split);

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
}

void ClientEnvironment::Draw()
{
	m_Scene->Draw();
}

ShipResource *ClientEnvironment::GetShipResourceByID(std::string id)
{
	//! \todo catch errors.
	return m_ShipResources[id];
}

void ClientEnvironment::send()
{
	m_NetworkManager->GetEvents();
}

void ClientEnvironment::receive()
{
}

void ClientEnvironment::gatherLocalInput()
{
	ShipInputList ship_input = m_InputManager->GetAllShipInputs();

	for (int i = 0; i < m_Options->NumPlayers; i++)
	{
		m_Ships[i].m_Input.thrust = ship_input[i].thrust;
		m_Ships[i].m_Input.left = ship_input[i].left;
		m_Ships[i].m_Input.right = ship_input[i].right;
		m_Ships[i].m_Input.primary = ship_input[i].primary;
		m_Ships[i].m_Input.secondary = ship_input[i].secondary;
		m_Ships[i].m_Input.bomb = ship_input[i].bomb;
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
{
}

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
