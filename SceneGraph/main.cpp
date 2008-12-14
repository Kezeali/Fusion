#include "..\FusionEngine\Common.h"
#include "..\FusionEngine\FusionCommon.h"

#include "..\FusionEngine\FusionLog.h"
#include "..\FusionEngine\FusionLogger.h"

#include "..\FusionEngine\FusionBitmask.h"
#include "..\FusionEngine\FusionPhysicsCollisionGrid.h"
#include "..\FusionEngine\FusionPhysicsWorld.h"
#include "..\FusionEngine\FusionPhysicsBody.h"
#include "..\FusionEngine\FusionPhysicsUtils.h"
#include "..\FusionEngine\FusionPhysicsTypes.h"
#include "..\FusionEngine\FusionPhysicsCallback.h"

#include "..\FusionEngine\FusionScene.h"
#include "..\FusionEngine\FusionNode.h"
#include "..\FusionEngine\FusionShipDrawable.h"
#include "..\FusionEngine\FusionShipDrawable.h"
#include "..\FusionEngine\FusionShipHealth.h"

const int g_NumDrones = 4;
const float g_ThrustForce = 0.14f;

using namespace FusionEngine;

class OutputUserData : public ICollisionHandler
{
private:
	FusionPhysicsBody* m_MyBody;

public:
	OutputUserData(FusionPhysicsBody* body)
		: m_MyBody(body)
	{
	}

	bool CanCollideWith(const FusionPhysicsBody *other)
	{
		return (other != m_MyBody);
	}

	void CollisionWith(const FusionPhysicsBody *other, const Vector2 &point)
	{
		if (other->GetUserData() != NULL)
		{
			char *data = (char *)(other->GetUserData());
			std::cout << "Egads! " << data << " got me!" << std::endl;
		}
	}

};


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

	FusionScene *m_Scene;

	FusionNode* m_Ship;
	FusionNode* m_Drones[g_NumDrones];

	bool Update(unsigned int split)
	{
		if (CL_Keyboard::get_keycode('R'))
		{
			m_ShipPhysical->_setForce(Vector2::zero());
			m_ShipPhysical->_setVelocity(Vector2::zero());
			m_ShipPhysical->_setAcceleration(Vector2::zero());

			m_ShipPhysical->_setPosition(Vector2(50.0f,50.0f));
		}

		// Warp
		if (CL_Keyboard::get_keycode('W'))
		{
			float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			Vector2 force = m_ShipPhysical->GetPosition();
			force.x += sinf(a)*4.0f;
			force.y += -cosf(a)*4.0f;

			m_ShipPhysical->_setPosition(force);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
		{
			m_ShipPhysical->ApplyForceRelative(-g_ThrustForce);
		}
		else if (CL_Keyboard::get_keycode(CL_KEY_UP))
		{
			m_ShipPhysical->ApplyForceRelative(g_ThrustForce);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
			m_ShipPhysical->SetRotationalVelocity(-0.3f);

		else if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
			m_ShipPhysical->SetRotationalVelocity(0.3f);

		else
			m_ShipPhysical->SetRotationalVelocity(0);

		// Homing mode for drone 0
		if (CL_Keyboard::get_keycode('H'))
		{
			m_DronePhysical[0]->SetRotationalVelocity(0.0f);

			Vector2 aim = m_ShipPhysical->GetPosition() - m_DronePhysical[0]->GetPosition();
			aim.normalize();

			m_DronePhysical[0]->ApplyForce(aim * g_ThrustForce);
		}
		else
		{
			for (int i = 0; i < g_NumDrones; i++)
			{
				m_DronePhysical[i]->SetRotationalVelocity(0.25f);
			}
		}

		m_World->RunSimulation(split);

		m_Ship->SetPosition(m_ShipPhysical->GetPosition());
		for (int i = 0; i < g_NumDrones; i++)
		{
			m_Drones[i]->SetPosition(m_DronePhysical[i]->GetPosition());
		}

		return true;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("Console", 80, 10);
		console.redirect_stdio();

		CL_DisplayWindow display("Display", 800, 600);

		new Console;
		Logger logger = new Logger(true);
		logger->SetUseDating(true);

		// World
		m_World = new FusionPhysicsWorld();
		m_World->SetConsole(Console::getSingletonPtr());
		m_World->Initialise(1280, 1024);
		m_World->SetMaxVelocity(20);
		m_World->SetBodyDeactivationPeriod(10000);
		m_World->SetDeactivationVelocity(0.05f);
		m_World->SetBitmaskRes(2);
		// Wrap around causes all sorts of problems at the moment...
		m_World->DeactivateWrapAround();

		// Ship
		m_ShipGraphical = new CL_Surface("Body.png");
		m_ShipGraphical->set_rotation_hotspot(origin_center);

		m_ShipBitmask = new FusionBitmask;
		//m_ShipBitmask->SetFromSurface(m_ShipGraphical, m_World->GetBitmaskRes());
		m_ShipBitmask->SetFromRadius(m_ShipGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 50.0f;
			props.position = Vector2(48.f, 120.f);
			props.rotation = 0;
			props.use_dist = true;
			props.dist = m_ShipGraphical->get_width() * 0.5f;
			props.radius = props.dist;
			props.use_bitmask = true;
			props.bitmask = m_ShipBitmask;

			m_ShipPhysical = m_World->CreateBody(PB_SHIP, props);
		}
		m_ShipPhysical->SetCoefficientOfFriction(0.25f);
		m_ShipPhysical->SetCoefficientOfRestitution(0.25f);

		m_ShipPhysical->SetICollisionHandler(new OutputUserData(m_ShipPhysical));

		// -- Create ship scene node --
		m_Ship = m_Scene->GetRootNode()->CreateChildNode(m_ShipPhysical->GetPosition(), m_ShipPhysical->GetRotation());

		// Drones
		for (int i = 0; i < g_NumDrones; i++)
		{
			m_DroneGraphical[i] = new CL_Surface("Body.png");
			m_DroneGraphical[i]->set_rotation_hotspot(origin_center);

			m_DroneBitmask[i] = new FusionBitmask;
			m_DroneBitmask[i]->SetFromRadius(16.0f, m_World->GetBitmaskRes());
			//m_DroneBitmask->SetFromRadius(m_DroneGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

			{
				PhysicalProperties props;
				props.mass = 50.0f;
				props.position = Vector2(460.f + 10.0f * i, 120.f);
				props.rotation = 0;
				props.use_dist = true;
				props.dist = m_DroneGraphical[i]->get_width() * 0.5;
				props.radius = props.dist;
				props.use_bitmask = true;
				props.bitmask = m_DroneBitmask[i];

				m_DronePhysical[i] = m_World->CreateBody(PB_SHIP, props);
			}

			m_DronePhysical[i]->SetCoefficientOfFriction(0.25f);
			m_DronePhysical[i]->SetCoefficientOfRestitution(0.3f);

			std::string ud = CL_String::format("Drone %1", i);
			char *userdata = (char *)malloc( ud.size() +1 );
			memcpy( userdata, ud.c_str(), ud.size() +1 );
			m_DronePhysical[i]->SetUserData(userdata);

			// -- Create Drone scene node --
			m_Drones[i] = m_Scene->GetRootNode()->CreateChildNode(m_DronePhysical[i]->GetPosition(), m_DronePhysical[i]->GetRotation());
		}

		// Terrain
		m_TerrainGraphical = new CL_Surface("Terrain.png", CL_Surface::flag_keep_pixelbuffer);
		m_TerrainGraphical->set_rotation_hotspot(origin_center);

		m_TerrainCanvas = new CL_Canvas(*m_TerrainGraphical);

		m_TerrainBitmask = new FusionBitmask;
		bool bitmaskLoaded = false;
		{
			{
				CL_InputSourceProvider *p = CL_InputSourceProvider::create_file_provider(".");
				bitmaskLoaded = m_TerrainBitmask->Load("bitmask", p);
			}
		}
		// Create and save the mask if it couldn't be loaded from a cache
		if (!bitmaskLoaded)
		{
			m_TerrainBitmask->SetFromSurface(m_TerrainGraphical, m_World->GetBitmaskRes(), 25);

			{
				CL_OutputSource_File file("bitmask");
				m_TerrainBitmask->Save("bitmask", &file);
			}
		}

		{
			PhysicalProperties props;
			props.mass = 0.0f;
			props.position = Vector2(0.0f, 220.0f);
			props.rotation = 0;
			props.use_bitmask = true;
			props.bitmask = m_TerrainBitmask;

			m_TerrainPhysical = m_World->CreateStatic(PB_TERRAIN, props);
		}
		m_TerrainPhysical->SetUserData((void *)"Terrain");
		m_TerrainPhysical->SetCoefficientOfFriction(0.8f);

		// -- Create Terrain scene node --
		m_Scene->GetRootNode()->CreateChildNode(m_DronePhysical[i]->GetPosition(), m_DronePhysical[i]->GetRotation());
		

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


			///////////////////////
			// Modify various params
			// Current time
			unsigned int time = CL_System::get_time();

			// Increace bounce
			if (inputTimer <= time && CL_Keyboard::get_keycode('Q'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution() + 0.1f;
				m_ShipPhysical->SetCoefficientOfRestitution( fe_min(cor, 1.0f) );

				std::cout << cor << std::endl;
			}
			// Reduce bounce
			if (inputTimer <= time && CL_Keyboard::get_keycode('A'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution() - 0.1f;
				m_ShipPhysical->SetCoefficientOfRestitution( fe_max(cor, 0.0f) );

				std::cout << cor << std::endl;
			}

			// Toggle bounce
			if (inputTimer <= time && CL_Keyboard::get_keycode('B'))
			{
				inputTimer = time + 500;

				// -- off --
				if (m_ShipPhysical->CheckCollisionFlag(C_BOUNCE))
				{

					int flags = m_ShipPhysical->GetCollisionFlags();
					m_ShipPhysical->_setCollisionFlags(flags ^ C_BOUNCE);

					for (int i = 0; i < g_NumDrones; i++)
					{
						int flags = m_DronePhysical[i]->GetCollisionFlags();
						m_DronePhysical[i]->_setCollisionFlags(flags ^ C_BOUNCE);
					}

					std::cout << "Bounce off" << std::endl;

				}
				// -- on --
				else
				{

					int flags = m_ShipPhysical->GetCollisionFlags();
					m_ShipPhysical->_setCollisionFlags(flags | C_BOUNCE);

					for (int i = 0; i < g_NumDrones; i++)
					{
						int flags = m_DronePhysical[i]->GetCollisionFlags();
						m_DronePhysical[i]->_setCollisionFlags(flags | C_BOUNCE);
					}

					std::cout << "Bounce on" << std::endl;

				}
			}
			// Toggle debug mode
			if (inputTimer <= time && CL_Keyboard::get_keycode('D'))
			{
				inputTimer = time + 500;
				debug = !debug;
			}

			//////////////
			// Draw (debug or scene)
			if (debug)
			{
				m_World->GetCollisionGrid()->DebugDraw();

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
				m_Scene->Draw();

			display.flip();
			CL_System::keep_alive();
		}

		delete logger;
		delete FusionEngine::Console::getSingletonPtr();

		return 0;
	}
} app;
