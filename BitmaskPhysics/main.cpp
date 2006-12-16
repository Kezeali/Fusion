#include "Common.h"
#include "FusionEngineCommon.h"

#include "FusionBitmask.h"
#include "FusionPhysicsWorld.h"
#include "FusionPhysicsBody.h"
#include "FusionPhysicsUtils.h"
#include "FusionPhysicsTypes.h"
#include "FusionPhysicsCallback.h"

const int g_NumDrones = 4;
const float g_ThrustForce = 0.2f;

using namespace FusionEngine;

class BitmaskTest : public CL_ClanApplication
{
	FusionBitmask *m_ShipBitmask;
	FusionBitmask *m_DroneBitmask[4];
	FusionBitmask *m_TerrainBitmask;

	FusionPhysicsWorld *m_World;

	CL_Surface *m_ShipGraphical;
	FusionPhysicsBody *m_ShipPhysical;

	CL_Surface *m_DroneGraphical[g_NumDrones];
	FusionPhysicsBody *m_DronePhysical[g_NumDrones];

	CL_Surface *m_TerrainGraphical;
	FusionPhysicsBody *m_TerrainPhysical;
	CL_Canvas *m_TerrainCanvas;

	CL_Surface *m_Damage;

	bool Update(unsigned int split)
	{
		if (CL_Keyboard::get_keycode('R'))
		{
			m_ShipPhysical->_setForce(CL_Vector2::ZERO);
			m_ShipPhysical->_setVelocity(CL_Vector2::ZERO);
			m_ShipPhysical->_setAcceleration(CL_Vector2::ZERO);

			m_ShipPhysical->_setPosition(CL_Vector2(50.0f,50.0f));
		}

		// Warp
		if (CL_Keyboard::get_keycode('W'))
		{
			float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			CL_Vector2 force = m_ShipPhysical->GetPosition();
			force.x += sinf(a)*g_ThrustForce;
			force.y += -cosf(a)*g_ThrustForce;

			m_ShipPhysical->_setPosition(force);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
		{
			float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			CL_Vector2 force;
			force.x = -sinf(a)*g_ThrustForce;
			force.y = cosf(a)*g_ThrustForce;

			m_ShipPhysical->ApplyForce(force);
		}
		else if (CL_Keyboard::get_keycode(CL_KEY_UP))
		{
			float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			CL_Vector2 force;
			force.x = sinf(a)*g_ThrustForce;
			force.y = -cosf(a)*g_ThrustForce;

			m_ShipPhysical->ApplyForce(force);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
			m_ShipPhysical->SetRotationalVelocity(-0.2f);

		else if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
			m_ShipPhysical->SetRotationalVelocity(0.2f);

		else
			m_ShipPhysical->SetRotationalVelocity(0);

		// Homing mode for drone 0
		if (CL_Keyboard::get_keycode('H'))
		{
			m_DronePhysical[0]->SetRotationalVelocity(0.0f);

			CL_Vector2 aim = m_ShipPhysical->GetPosition() - m_DronePhysical[0]->GetPosition();
			aim.unitize();

			m_DronePhysical[0]->ApplyForce(aim * g_ThrustForce);
		}
		else
		{
			for (int i = 0; i < g_NumDrones; i++)
			{
				m_DronePhysical[i]->SetRotationalVelocity(0.2f);
				m_DronePhysical[i]->ApplyForce(g_ThrustForce);
			}
		}

		

		m_World->RunSimulation(split);

		return true;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("Console", 80, 10);
		console.redirect_stdio();

		CL_DisplayWindow display("Display", 1024, 580);

		// World
		m_World = new FusionPhysicsWorld();
		m_World->Initialise(1024, 580);
		m_World->SetMaxVelocity(20);
		m_World->SetBodyDeactivationPeriod(20000);
		m_World->SetDeactivationVelocity(0.05f);
		m_World->SetBitmaskRes(2);
		m_World->DeactivateWrapAround();

		// Ship
		m_ShipGraphical = new CL_Surface("../Body.png");
		m_ShipGraphical->set_rotation_hotspot(origin_center);

		m_ShipBitmask = new FusionBitmask;
		//m_ShipBitmask->SetFromSurface(m_ShipGraphical, m_World->GetBitmaskRes());
		m_ShipBitmask->SetFromRadius(m_ShipGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 20.0f;
			props.position = CL_Vector2(48.f, 120.f);
			props.radius = 50;
			props.rotation = 0;
			props.use_dist = true;
			props.dist = m_ShipGraphical->get_width() * 0.5;
			props.use_bitmask = true;
			props.bitmask = m_ShipBitmask;

			m_ShipPhysical = m_World->CreateBody(PB_SHIP, props);
			m_ShipPhysical->SetCoefficientOfFriction(0.4f);
			m_ShipPhysical->SetCoefficientOfRestitution(0.6f);
		}

		// Drones
		for (int i = 0; i < g_NumDrones; i++)
		{
			m_DroneGraphical[i] = new CL_Surface("./Body.png");
			m_DroneGraphical[i]->set_rotation_hotspot(origin_center);

			m_DroneBitmask[i] = new FusionBitmask;
			m_DroneBitmask[i]->SetFromRadius(16.0f, m_World->GetBitmaskRes());
			//m_DroneBitmask->SetFromRadius(m_DroneGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

			{
				PhysicalProperties props;
				props.mass = 15.0f;
				props.position = CL_Vector2(480.f, 120.f);
				props.radius = 50;
				props.rotation = 0;
				props.use_dist = true;
				props.dist = m_DroneGraphical[i]->get_width() * 0.5;
				props.use_bitmask = true;
				props.bitmask = m_DroneBitmask[i];

				m_DronePhysical[i] = m_World->CreateBody(PB_SHIP, props);
			}

			m_DronePhysical[i]->SetCoefficientOfFriction(0.6f);
			m_DronePhysical[i]->SetCoefficientOfRestitution(0.6f);
		}

		// Hole
		/*m_Damage = new CL_Surface("./circle.png");
		m_Damage->set_rotation_hotspot(origin_center);
		m_Damage->set_blend_func(blend_zero, blend_one_minus_src_alpha);*/

		// Terrain
		m_TerrainGraphical = new CL_Surface("./Terrain.png", CL_Surface::flag_keep_pixelbuffer);
		m_TerrainGraphical->set_rotation_hotspot(origin_center);

		m_TerrainCanvas = new CL_Canvas(*m_TerrainGraphical);

		m_TerrainBitmask = new FusionBitmask;
		bool bitmaskLoaded = false;
		{
			std::ifstream ifs("bitmask");
			if (ifs.is_open())
			{
				bitmaskLoaded = m_TerrainBitmask->SetFromStream(ifs);

				//ifs >> m_TerrainBitmask;
			}
		}
		// Create and save the mask if it couldn't be loaded from a cache
		if (!bitmaskLoaded)
		{
			m_TerrainBitmask->SetFromSurface(m_TerrainGraphical, m_World->GetBitmaskRes(), 25);

			std::ofstream ofs("bitmask");
			{
				m_TerrainBitmask->ToStream(ofs);

				//ofs << m_TerrainBitmask;
			}
		}

		//m_TerrainBitmask->SetFromDimensions(::CL_Size(600, 200), m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 0.0f;
			props.position = CL_Vector2(0.0f, 220.0f);
			props.rotation = 0;
			props.use_bitmask = true;
			props.bitmask = m_TerrainBitmask;

			m_TerrainPhysical = m_World->CreateStatic(PB_TERRAIN, props);
		}
		
		int back_pos = 0;
		float sur_y = 250.f;
		float sur_x = 400.f;

		unsigned int inputTimer = 0;
		bool debug = true;
		
		unsigned int lastframe = CL_System::get_time();
		unsigned int split = 0;

		while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
		{
			display.get_gc()->clear(CL_Color(128, 200, 236));

			//(back_pos > 1000) ? back_pos = 0 : back_pos++;
			//sur_x = sinf(back_pos / 100.0f) * 100.0f + 400.0f;


			split = CL_System::get_time() - lastframe;
			lastframe = CL_System::get_time();

			// Move the ship
			Update(split);


			// Modify various params
			unsigned int time = CL_System::get_time();

			if (inputTimer <= time && CL_Keyboard::get_keycode('Q'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution();
				m_ShipPhysical->SetCoefficientOfRestitution(cor + 0.1f);
			}
			if (inputTimer <= time && CL_Keyboard::get_keycode('A'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution();
				m_ShipPhysical->SetCoefficientOfRestitution(cor - 0.1f);
			}

			if (inputTimer <= time && CL_Keyboard::get_keycode('D'))
			{
				inputTimer = time + 500;
				debug = !debug;
			}
			if (debug)
			{
				m_TerrainBitmask->DisplayBits(
					m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y);

				m_ShipBitmask->DisplayBits(
					m_ShipPhysical->GetPosition().x, m_ShipPhysical->GetPosition().y);

				for (int i = 0; i < g_NumDrones; i++)
				{
					m_DroneBitmask[i]->DisplayBits(
						m_DronePhysical[i]->GetPosition().x, m_DronePhysical[i]->GetPosition().y);
				}
			}
			else
			{
				// Draw the terrain
				// The following should be in the terrain impact callback (make hole)
				//  m_Damage.draw(rect, m_TerrainCanvas.get_gc());
				//  m_TerrainCanvas->sync_surface();

				m_TerrainGraphical->draw(
					m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y
					);

				/*params.rotate_origin = origin_center;
				params.rotate_angle = m_TerrainPhysical->GetRotation();
				params.destX = m_TerrainPhysical->GetPosition().x;
				params.destY = m_TerrainPhysical->GetPosition().y;

				m_TerrainGraphical->draw(params);*/

				// Draw the ship
				m_ShipGraphical->set_angle(m_ShipPhysical->GetRotation());
				m_ShipGraphical->draw(
					m_ShipPhysical->GetPosition().x, m_ShipPhysical->GetPosition().y);

				/*CL_Surface_DrawParams2 params;
				params.rotate_origin = origin_center;
				params.rotate_angle = m_ShipPhysical->GetRotation();
				params.destX = m_ShipPhysical->GetPosition().x;
				params.destY = m_ShipPhysical->GetPosition().y;

				m_ShipGraphical->draw(params);*/


				// Draw the drones
				for (int i = 0; i < g_NumDrones; i++)
				{
					m_DroneGraphical[i]->set_angle(m_DronePhysical[i]->GetRotation());
					m_DroneGraphical[i]->draw(
						m_DronePhysical[i]->GetPosition().x, m_DronePhysical[i]->GetPosition().y);
				}
			}

			display.flip();
			CL_System::keep_alive();
		}

		return 0;
	}
} app;