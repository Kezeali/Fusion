
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
	FusionNode *node = m_Scene->CreateNode();

	FusionPhysicsBody *pbod = new FusionPhysicsBody(m_PhysicsWorld, new FusionShipResponse);
	m_PhysicsWorld->AddBody(pbod);

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
