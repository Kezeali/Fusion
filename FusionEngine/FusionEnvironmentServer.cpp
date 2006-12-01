
#include "FusionEnvironmentServer.h"

#include "FusionShipDrawable.h"

using namespace FusionEngine;

ServerEnvironment::ServerEnvironment(const std::string &port, ServerOptions *options)
: m_Port(port),
m_Options(options)
{
	m_PhysicsWorld = new FusionPhysicsWorld();
	m_NetworkManager = new FusionNetworkServer(port, options);
	m_Scene = new FusionScene();
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
	return true; // mwahaha
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
	ShipResource *res = m_ShipResources[m_PlayerResourceIds[state.PID]];

	// Create children
	//  Engines
	if (state.engines & LEFT)
		FusionNode *node_len = node->CreateChildNode(res->Positions.LeftEngine);
	if (state.engines & RIGHT)
		FusionNode *node_ren = node->CreateChildNode(res->Positions.RightEngine);
	//  Weapons
	FusionNode *node_priw = node->CreateChildNode(res->Positions.PrimaryWeapon);
	FusionNode *node_secw = node->CreateChildNode(res->Positions.SecondaryWeapon);

	// Create the physical body
	FusionPhysicsBody *pbod = new FusionPhysicsBody(m_PhysicsWorld, new FusionShipResponse);
	m_PhysicsWorld->AddBody(pbod);

	// Create a ship and add it to the list
	m_Ships.push_back(new FusionShip(state, pbod, node));
}

void ServerEnvironment::send()
{
}

bool ServerEnvironment::receive()
{
	return false;
}

void ServerEnvironment::updateAllPositions(unsigned int split)
{
	m_PhysicsWorld->RunSimulation(split);
}
