#include "FusionServerEnvironment.h"
#include <math.h>

using namespace FusionEngine;

ServerEnvironment(ServerOptions *options)
: m_Options(options)
{
	m_Scene = new FusionScene();
}

bool ServerEnvironment::Initialise(ResourceLoader *resources)
{
	m_ShipResources = resources->GetLoadedShips();
	return true;
}

void ServerEnvironment::Update(unsigned int split) /*!The main game loop calls this
                                                     before it draws each frame. Every
                                                     thing gets updated here, and data
                                                     gets read from the things that
                                                     update themselves (InputManager
                                                     and NetworkManager.)*/
{
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


}

void ServerEnvironment::updateShipStates()  /*!this is a method called by Update,
                                            it takes the data received from the
                                            clients (or for the ClientEnv version,
                                            it takes the data from the keyboard)
                                            and puts it into FusionShip.shipstate*/
{
}

void ServerEnvironment::updateAllPositions() /*!This tells each ship to update it's
                                              nodes position (the node is the thing
                                              that handles drawing, it's seperate
                                              form the rest of the ship cause that
                                              makes it more generic.)*/
{
}