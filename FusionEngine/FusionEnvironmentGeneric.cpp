
#include "FusionEnvironmentClient.h"

using namespace FusionEngine;

GenericEnvironment::GenericEnvironment(const std::string &hostname, const std::string &port, ClientOptions *options)
: m_Options(options)
{
	m_InputManager = new FusionInput((*m_Options));
	m_NetworkManager = new FusionNetworkClient(hostname, port, options);
	m_Scene = new FusionScene();
}

GenericEnvironment::~GenericEnvironment()
{
	delete m_InputManager;
	delete m_NetworkManager;
	delete m_Scene;
}

bool GenericEnvironment::Initialise(ResourceLoader *resources)
{
	m_InputManager->Initialise();

	// Not doing threading any more, cause that's the way we roll
	//m_NetManThread = new CL_Thread(m_NetworkManager, false);
	//m_NetManThread->start();

	//! \todo START FILE SYNC STATE HERE!!! (that will load verified ships)

	return true;
}

bool GenericEnvironment::Update(unsigned int split)
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

	//! \todo GenericEnvironment#Update() and ServerEnvironment#Update() should return false,
	//! or perhaps an error, when update fails... Or perhaps I should use exceptions here?
	return true;
}

void GenericEnvironment::Draw()
{
	m_Scene->Draw();
}

ShipResource *GenericEnvironment::GetShipResourceByID(const std::string &id)
{
	//! \todo catch errors.
	return m_ShipResources[id];
}

void GenericEnvironment::_quit(ErrorType *e)
{
	m_LastError = e;
	m_Quit = true;
}

void GenericEnvironment::installShipFrameFromMessage(FusionMessage *m)
{
	ShipState state;

	RakNet::BitStream bs(m->Read(), m->GetLength(), false);

	// Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
	bs.Read(state.PID);

	bs.Read(state.Position.x);
	bs.Read(state.Position.y);

	bs.Read(state.Velocity.x);
	bs.Read(state.Velocity.y);

	bs.Read(state.Rotation);
	bs.Read(state.RotationalVelocity);

	bs.Read(state.health);

	bs.Read(state.current_primary);
	bs.Read(state.current_secondary);
	bs.Read(state.current_bomb);

	bs.Read(state.engines);
	bs.Read(state.weapons);

	m_Ships[state.PID]->SetShipState(state);
}

void GenericEnvironment::installShipInputFromMessage(FusionMessage *m)
{
	ShipInput state;

	RakNet::BitStream bs(m->Read(), m->GetLength(), false);

	// Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
	bs.Read(state.pid);

	bs.Read(state.thrust);
	bs.Read(state.reverse);
	bs.Read(state.left);
	bs.Read(state.right);

	bs.Read(state.primary);
	bs.Read(state.secondary);
	bs.Read(state.bomb);

	m_Ships[state.pid]->SetInputState(state);
}

void GenericEnvironment::installProjectileFrameFromMessage(FusionMessage *m)
{
	ProjectileState state;

	RakNet::BitStream bs(m->Read(), m->GetLength(), false);

	// Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
	bs.Read(state.PID);

	bs.Read(state.OID);

	bs.Read(state.Position.x);
	bs.Read(state.Position.y);

	bs.Read(state.Velocity.x);
	bs.Read(state.Velocity.y);

	bs.Read(state.Rotation);
	bs.Read(state.RotationalVelocity);

	m_Projectiles[state.OID]->SetState(state);
}

void GenericEnvironment::updateAllPositions(unsigned int split)
{
	m_PhysicsWorld->RunSimulation(split);
}


		/* [depreciated] See FusionPhysicsWorld
	    //Begin main server loop
    for(int i=0; i<numPlayers; i++) //move and uncheck collision
    {
        m_Ships[i].CurrentState.Velocity.x += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * sin(m_Ships[i].CurrentState.Rotation);
        m_Ships[i].CurrentState.Velocity.y += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * cos(m_Ships[i].CurrentState.Rotation);
    }

    for(int i=0; i<numPlayers; i++)
    {
        for(int j=0; j<numPlayers; i++)
        {
            if(i==j)
                break;

            //Ship Ship collision
            if(sqrt((m_Ships[i].CurrentState.Position.x - m_Ships[j].CurrentState.Position.x)^2 + (m_Ships[i].CurrentState.Position.y - m_Ships[j].CurrentState.Position.y)^2)=<(m_Ships[i].CurrentState.Radius+m_Ships[j].CurrentState.Radius))
            {
                CL_Vector tempvelocity;
                float temprotation;

                temprotation = m_Ships[i].CurrentState.Rotation;
                tempvelocity.x = m_ShipResource[i].Velocity.x;
                tempvelocity.y = m_ShipResource[i].Velocity.y;

                m_Ships[i].CurrentState.Rotation = m_Ships[j].CurrentState.Rotation;
                m_Ships[i].CurrentState.Velocity.x = m_Ships[j].CurrentState.Velocity.x;
                m_Ships[i].CurrentState.Velocity.y = m_Ships[j].CurrentState.Velocity.y;

                m_Ships[j].CurrentState.Rotation = temprotation;
                m_Ships[j].CurrentState.Velocity.x = tempvelocity.x;
                m_Ships[j].CurrentState.Velocity.y = tempvelocity.y;

                m_Ships[i].CurrentState.Velocity.x += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * sin(m_Ships[i].CurrentState.Rotation);
                m_Ships[i].CurrentState.Velocity.y += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * cos(m_Ships[i].CurrentState.Rotation);
                m_Ships[j].CurrentState.Velocity.x += m_Ships[j].CurrentState.ShipMass / m_Ships[j].CurrentState.EngineForce * sin(m_Ships[j].CurrentState.Rotation);
                m_Ships[j].CurrentState.Velocity.y += m_Ships[j].CurrentState.ShipMass / m_Ships[j].CurrentState.EngineForce * cos(m_Ships[j].CurrentState.Rotation);
            }
        }
    }

            //ship - projectile
            //ship - terrain

        //If hit terrain move back + damage -- MrCai: damage? 
		        //   Well, that should be an option... eventually :P
	*/

//void GenericEnvironment::updateSceneGraph()
//{
//}

// IGNORE THE FOLLOWING CODE, the scene now draws everthing!
//
//void GenericEnvironment::drawLevel()
//{
//}

//void GenericEnvironment::drawShip(FusionShip ship)
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
