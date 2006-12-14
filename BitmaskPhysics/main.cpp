#include "Common.h"
#include "FusionEngineCommon.h"

#include "FusionBitmask.h"
#include "FusionPhysicsWorld.h"
#include "FusionPhysicsBody.h"
#include "FusionPhysicsUtils.h"
#include "FusionPhysicsTypes.h"
#include "FusionPhysicsCallback.h"

const float g_ThrustForce = 1.5f;

using namespace FusionEngine;

class BitmaskTest : public CL_ClanApplication
{
	FusionBitmask *m_ShipBitmask;
	FusionBitmask *m_DroneBitmask;
	FusionBitmask *m_TerrainBitmask;

	FusionPhysicsWorld *m_World;

	CL_Surface *m_ShipGraphical;
	FusionPhysicsBody *m_ShipPhysical;

	CL_Surface *m_DroneGraphical;
	FusionPhysicsBody *m_DronePhysical;

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
			float a = m_ShipPhysical->GetRotation() * PI/180.0f;
			CL_Vector2 force = m_ShipPhysical->GetPosition();
			force.x += sinf(a)*g_ThrustForce;
			force.y += -cosf(a)*g_ThrustForce;

			m_ShipPhysical->_setPosition(force);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
		{
			float a = m_ShipPhysical->GetRotation() * PI/180.0f;
			CL_Vector2 force;
			force.x = -sinf(a)*g_ThrustForce;
			force.y = cosf(a)*g_ThrustForce;

			m_ShipPhysical->ApplyForce(force);
		}
		else if (CL_Keyboard::get_keycode(CL_KEY_UP))
		{
			float a = m_ShipPhysical->GetRotation() * PI/180.0f;
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

		m_World->RunSimulation(split);

		return true;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_DisplayWindow display("Display", 1024, 600);

		CL_ConsoleWindow console("Console");
		console.redirect_stdio();

		// World
		m_World = new FusionPhysicsWorld();
		m_World->Initialise(1024, 768);
		m_World->SetBodyDeactivationPeriod(4000);
		m_World->SetMaxVelocity(20);
		m_World->SetBitmaskRes(4);
		m_World->DeactivateWrapAround();

		// Ship
		m_ShipGraphical = new CL_Surface("../Body.png");
		m_ShipGraphical->set_rotation_hotspot(origin_center);

		m_ShipBitmask = new FusionBitmask;
		//m_ShipBitmask->SetFromSurface(m_ShipGraphical, m_World->GetBitmaskRes());
		m_ShipBitmask->SetFromRadius(m_ShipGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 20;
			props.position = CL_Vector2(48.f, 120.f);
			props.radius = 50;
			props.rotation = 0;
			props.use_dist = true;
			props.dist = m_ShipGraphical->get_width() * 0.5;
			props.use_bitmask = true;
			props.bitmask = m_ShipBitmask;

			m_ShipPhysical = m_World->CreateBody(PB_SHIP, props);
			m_ShipPhysical->SetCoefficientOfFriction(0.3f);
			m_ShipPhysical->SetCoefficientOfRestitution(0.8f);
		}

		// Drone
		m_DroneGraphical = new CL_Surface("../Body.png");
		m_DroneGraphical->set_rotation_hotspot(origin_center);

		m_DroneBitmask = new FusionBitmask;
		m_DroneBitmask->SetFromSurface(m_DroneGraphical, m_World->GetBitmaskRes());
		//m_DroneBitmask->SetFromRadius(m_DroneGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 20;
			props.position = CL_Vector2(480.f, 120.f);
			props.radius = 50;
			props.rotation = 0;
			props.use_dist = true;
			props.dist = m_DroneGraphical->get_width() * 0.5;
			props.use_bitmask = true;
			props.bitmask = m_DroneBitmask;

			m_DronePhysical = m_World->CreateBody(PB_SHIP, props);
			m_DronePhysical->SetCoefficientOfFriction(0.2f);
			m_DronePhysical->SetCoefficientOfRestitution(0.8f);
		}

		// Hole
		m_Damage = new CL_Surface("../circle.png");
		m_Damage->set_rotation_hotspot(origin_center);
		m_Damage->set_blend_func(blend_zero, blend_one_minus_src_alpha);

		// Terrain
		m_TerrainGraphical = new CL_Surface("../Terrain.png", CL_Surface::flag_keep_pixelbuffer);
		m_TerrainGraphical->set_rotation_hotspot(origin_center);

		m_TerrainCanvas = new CL_Canvas(*m_TerrainGraphical);

		m_TerrainBitmask = new FusionBitmask;
		bool bitmaskLoaded = false;
		{
			std::ifstream ifs("bitmask");
			if (ifs.is_open())
			{
				//std::stringstream temp;
				//ifs >> temp;

				m_TerrainBitmask->SetFromStream(ifs);

				//ifs >> m_TerrainBitmask;

				bitmaskLoaded = true;
			}
		}
		// Create and save the mask if it couldn't be loaded from a cache
		if (!bitmaskLoaded)
		{
			m_TerrainBitmask->SetFromSurface(m_TerrainGraphical, m_World->GetBitmaskRes());

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


			// Draw the drone
			m_DroneGraphical->set_angle(m_DronePhysical->GetRotation());
			m_DroneGraphical->draw(
				m_DronePhysical->GetPosition().x, m_DronePhysical->GetPosition().y);


			if (CL_Keyboard::get_keycode('7'))
			{
				float cor = m_ShipPhysical->GetCoefficientOfRestitution();
				m_ShipPhysical->SetCoefficientOfRestitution(cor + 0.1f);
			}
			if (CL_Keyboard::get_keycode('4'))
			{
				float cor = m_DronePhysical->GetCoefficientOfRestitution();
				m_DronePhysical->SetCoefficientOfRestitution(cor - 0.1f);
			}

			if (CL_Keyboard::get_keycode('8'))
			{
				float cor = m_DronePhysical->GetCoefficientOfRestitution();
				m_DronePhysical->SetCoefficientOfRestitution(cor + 0.1f);
			}
			if (CL_Keyboard::get_keycode('2'))
			{
				float cor = m_DronePhysical->GetCoefficientOfRestitution();
				m_DronePhysical->SetCoefficientOfRestitution(cor - 0.1f);
			}

			unsigned int time = CL_System::get_time();
			if (inputTimer <= time && CL_Keyboard::get_keycode('D'))
			{
				inputTimer = time + 1000;
				debug = !debug;
			}
			if (debug)
			{
				m_TerrainBitmask->DisplayBits(
					m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y);

				m_ShipBitmask->DisplayBits(
					m_ShipPhysical->GetPosition().x, m_ShipPhysical->GetPosition().y);

				m_DroneBitmask->DisplayBits(
					m_DronePhysical->GetPosition().x, m_DronePhysical->GetPosition().y);
			}
			else
			{
				// Draw the terrain
				// The following should be in the terrain impact callback (make hole)
				//  m_TerrainCanvas->sync_surface();

				m_TerrainGraphical->draw(
					m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y
					);

				/*params.rotate_origin = origin_center;
				params.rotate_angle = m_TerrainPhysical->GetRotation();
				params.destX = m_TerrainPhysical->GetPosition().x;
				params.destY = m_TerrainPhysical->GetPosition().y;

				m_TerrainGraphical->draw(params);*/
			}

			//damage.draw(rect, canvas.get_gc());
			//canvas.sync_surface();

			display.flip();
			CL_System::keep_alive();
		}

		return 0;
	}
} app;