#include "FusionClientEnvironment.h"

using namespace FusionEngine;

ClientEnvironment(ClientOptions *options)
: m_Options(options)
{
	m_InputManager = new FusionInput();
	m_Scene = new FusionScene();
}

bool ClientEnvironment::Initialise(ResourceLoader *resources)
{
	m_ShipResources = resources->GetLoadedShips();
	return true;
}

void ClientEnvironment::Update(unsigned int split)
{
  // Setup local frames
	gatherLocalInput();

  // Update the states of all local objects based on the gathered input
  predictLocal();

  // Send everything in the message queue
  send();

  // Read any frames / messages received between this update and the last
  if (receive())
  {
		// Build all local frames
		buildFrames();
    // Update the states of all sync'ed objects if any remote frames were received
    //  (this code will also check the time on each frame individually to make sure it has changed)
    updateObjects;
  }

  // Move/rotate ships based on the received/predicted frames
  finaliseObjectMovement();

  // Update all the client only stuff (particle systems, falling engines, etc.)
  updateNonSynced();
}

void ClientEnvironment::Draw()
{
	m_Scene->Draw();
}

ShipResource *ClientEnvironment::GetShipResourceByID(std::string id)
{
	//!\todo catch errors.
	return m_ShipResources[id];
}

void ClientEnvironment::gatherLocalInput()
{
}

void ClientEnvironment::updateShipStates()
{
}

void ClientEnvironment::updateAllPositions(unsigned int split)
{
}

void ClientEnvironment::updateSceneGraph()
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
