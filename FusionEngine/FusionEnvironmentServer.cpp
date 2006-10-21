/*
  Copyright (c) 2006 FusionTeam

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "FusionServerEnvironment.h"

#include "FusionPhysicsWorld.h"

using namespace FusionEngine;

ServerEnvironment::ServerEnvironment(const std::string &port, ServerOptions *options)
: m_Options(options)
{
	// The server doesn't need an input handler; uses commandline
	//m_InputManager = new FusionInput((*m_Options));
	m_NetworkManager = new FusionNetworkServer(port, options);
	m_Scene = new FusionScene();
}

bool ServerEnvironment::Initialise(ResourceLoader *resources)
{
	m_ShipResources = resources->GetLoadedShips();
	return true;
}

bool ServerEnvironment::Update(unsigned int split)
{
	return true; // mwahaha
}

void ServerEnvironment::updateShipStates()
{
}

void ServerEnvironment::updateAllPositions(unsigned int split)
{
	m_PhysicsWorld->RunSimulation(split);
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
}