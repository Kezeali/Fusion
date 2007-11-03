#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

#include "../FusionEngine/FusionBitmask.h"
//#include "../FusionEngine/FusionPhysicsCollisionGrid.h"
#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionPhysicsBody.h"
//#include "../FusionEngine/FusionPhysicsUtils.h"
#include "../FusionEngine/FusionPhysicsTypes.h"
#include "../FusionEngine/FusionPhysicsCallback.h"

const int g_NumDrones = 4;
const float g_ThrustForce = 3.0f;

using namespace FusionEngine;

class OutputUserData : public ICollisionHandler
{
private:
	PhysicsBody* m_MyBody;

public:
	OutputUserData(PhysicsBody* body)
		: m_MyBody(body)
	{
	}

	bool CanCollideWith(const PhysicsBody *other)
	{
		return (other != m_MyBody);
	}

	void CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts)
	{
		if (other->GetUserData() != NULL)
		{
			char *data = (char *)(other->GetUserData());
			std::cout << "Egads! " << data << " got me!" << std::endl;
		}
	}

};

class BitmaskTest;

class Explosive : public ICollisionHandler
{
	BitmaskTest* m_Env;
	PhysicsBody* m_MyBody;

public:
	Explosive(BitmaskTest* env, PhysicsBody* body)
		: m_Env(env),
		m_MyBody(body)
	{
	}

	bool CanCollideWith(const PhysicsBody *other)
	{
		return true;
	}

	void CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts);
};

class BitmaskTest : public CL_ClanApplication
{
	//Bitmask *m_ShipBitmask;
	//Bitmask *m_DroneBitmask[4];

	PhysicsWorld *m_World;

	CL_Surface* m_ProjectileGFX;
	std::vector<PhysicsBody*> m_Projectiles;
	std::vector<PhysicsBody*> m_DeleteQueue;

	CL_Surface *m_ShipGraphical;
	PhysicsBody *m_ShipPhysical;

	CL_Surface *m_DroneGraphical[g_NumDrones];
	PhysicsBody *m_DronePhysical[g_NumDrones];

	CL_Surface *m_TerrainGraphical;
	PhysicsBody *m_TerrainPhysical;
	Bitmask *m_TerrainBitmask;
	CL_Canvas *m_TerrainCanvas;

	PhysicsBody* m_DamageBody;
	CL_Surface *m_Damage;

	bool firing;

public:
	void Detonate(PhysicsBody* body, float radius, const Vector2& position)
	{
		float scale = (1.0/8.0f)*(radius+2);
		int x_offset = m_TerrainPhysical->GetPosition().x;
		int y_offset = m_TerrainPhysical->GetPosition().y;

		m_Damage->set_scale(scale, scale);
		m_Damage->draw(position.x-x_offset, position.y-y_offset, m_TerrainCanvas->get_gc() );
		m_DamageBody->_setPosition(Vector2::ZERO);
		CircleShape shape(m_DamageBody, 0.f, radius, cpvzero);
		m_TerrainBitmask->Erase(&shape, position);

		for (std::vector<PhysicsBody*>::iterator it = m_Projectiles.begin(), end = m_Projectiles.end(); it != end; ++it)
		{
			if ((*it) == body)
			{
				m_Projectiles.erase(it);
				m_DeleteQueue.push_back(body);
				break;
			}
		}
	}

private:
	bool Update(unsigned int split)
	{
		// Delete detonated bodies
		for (std::vector<PhysicsBody*>::iterator it = m_DeleteQueue.begin(), end = m_DeleteQueue.end(); it != end; ++it)
		{
			m_World->DestroyBody((*it));
		}
		m_DeleteQueue.clear();

		if (CL_Keyboard::get_keycode('R'))
		{
			m_ShipPhysical->_setForce(Vector2::ZERO);
			m_ShipPhysical->_setVelocity(Vector2::ZERO);
			m_ShipPhysical->_setAcceleration(Vector2::ZERO);

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
			//float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			//Vector2 force;
			//force.x = -sinf(a)*g_ThrustForce;
			//force.y = cosf(a)*g_ThrustForce;

			//m_ShipPhysical->ApplyForce(force);
			m_ShipPhysical->ApplyForceRelative(-g_ThrustForce);
		}
		else if (CL_Keyboard::get_keycode(CL_KEY_UP))
		{
			//float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			//Vector2 force;
			//force.x = sinf(a)*g_ThrustForce;
			//force.y = -cosf(a)*g_ThrustForce;

			//m_ShipPhysical->ApplyForce(force);
			m_ShipPhysical->ApplyForceRelative(g_ThrustForce);
		}

		if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
			m_ShipPhysical->SetRotationalVelocity(-2.f);

		else if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
			m_ShipPhysical->SetRotationalVelocity(2.f);

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
				m_DronePhysical[i]->SetRotationalVelocity(0.5f);
				//m_DronePhysical[i]->ApplyEngineForce(g_ThrustForce);
			}
		}

		if (CL_Keyboard::get_keycode(CL_KEY_SPACE) != firing && (firing = !firing))
		{
			float rad =  m_ProjectileGFX->get_width() * 0.5f;

			float a = m_ShipPhysical->GetRotation();
			Vector2 position = m_ShipPhysical->GetPosition();
			position.x += sinf(a) * (m_ShipPhysical->GetRadius() + rad);
			position.y += -cosf(a) * (m_ShipPhysical->GetRadius() + rad);

			PhysicalProperties props;
			props.mass = 1.f;
			props.position = position;
			props.rotation = a;
			props.use_dist = true;
			props.dist = rad-1.0;
			props.radius = props.dist;

			PhysicsBody* body = m_World->CreateBody(PB_PROJECTILE, props);
			body->SetCollisionHandler(new Explosive(this, body));
			body->_setVelocity(m_ShipPhysical->GetVelocity());
			body->ApplyForceRelative(20.0f);

			m_Projectiles.push_back(body);
		}

		//for (int i = 0; i < (int)(split*0.1f)+1; i++)
			m_World->RunSimulation(10);

		return true;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("Console", 80, 10);
		console.redirect_stdio();

		CL_DisplayWindow display("Display", 1024, 580);

		firing = false;

		// World
		m_World = new PhysicsWorld();
		m_World->Initialise(1024, 580);
		m_World->SetMaxVelocity(20);
		m_World->SetDamping(0.8);
		m_World->SetBodyDeactivationPeriod(10000);
		m_World->SetDeactivationVelocity(0.05f);
		m_World->SetBitmaskRes(4);
		m_World->DeactivateWrapAround();

		// Ship
		m_ShipGraphical = new CL_Surface("Body.png");
		m_ShipGraphical->set_rotation_hotspot(origin_center);
		m_ShipGraphical->set_alignment(origin_center);

		//m_ShipBitmask = new Bitmask;
		//m_ShipBitmask->SetFromSurface(m_ShipGraphical, m_World->GetBitmaskRes());
		//m_ShipBitmask->SetFromRadius(m_ShipGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 0.4f;
			props.position = Vector2(48.f, 120.f);
			props.rotation = 0;
			props.use_dist = true;
			props.dist = m_ShipGraphical->get_width() * 0.5f;
			props.radius = props.dist;

			m_ShipPhysical = m_World->CreateBody(PB_SHIP, props);
		}
		m_ShipPhysical->SetCoefficientOfFriction(0.25f);
		m_ShipPhysical->SetCoefficientOfRestitution(0.25f);

		m_ShipPhysical->SetCollisionHandler(new OutputUserData(m_ShipPhysical));

		// Drones
		for (int i = 0; i < g_NumDrones; i++)
		{
			m_DroneGraphical[i] = new CL_Surface("Body.png");
			m_DroneGraphical[i]->set_rotation_hotspot(origin_center);
			m_DroneGraphical[i]->set_alignment(origin_center);

			//m_DroneBitmask[i] = new FusionBitmask;
			//m_DroneBitmask[i]->SetFromRadius(16.0f, m_World->GetBitmaskRes());
			//m_DroneBitmask->SetFromRadius(m_DroneGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

			{
				PhysicalProperties props;
				props.mass = 1.0f;
				props.position = Vector2(460.f + 10.0f * i, 120.f);
				props.rotation = 0;
				props.use_dist = true;
				props.dist = m_DroneGraphical[i]->get_width() * 0.5;
				props.radius = props.dist;
				//props.use_bitmask = true;
				//props.bitmask = m_DroneBitmask[i];

				m_DronePhysical[i] = m_World->CreateBody(PB_SHIP, props);
			}

			m_DronePhysical[i]->SetCoefficientOfFriction(0.25f);
			m_DronePhysical[i]->SetCoefficientOfRestitution(0.0f);

			std::string ud = CL_String::format("Drone %1", i);
			char *userdata = (char *)malloc( ud.size() +1 );
			memcpy( userdata, ud.c_str(), ud.size() +1 );
			m_DronePhysical[i]->SetUserData(userdata);
		}

		// Hole
		m_Damage = new CL_Surface("./circle.png");
		m_Damage->set_alignment(origin_center);
		m_Damage->set_blend_func(blend_zero, blend_one_minus_src_alpha);
		m_DamageBody = m_World->CreateStatic(0);

		m_ProjectileGFX = new CL_Surface("./circle.png");
		m_ProjectileGFX->set_alignment(origin_center);

		// Terrain
		m_TerrainGraphical = new CL_Surface("Terrain.png", CL_Surface::flag_keep_pixelbuffer);
		m_TerrainCanvas = new CL_Canvas(*m_TerrainGraphical);

		{
			PhysicalProperties props;
			props.mass = g_PhysStaticMass;
			props.position = Vector2(0.0f, 220.0f);
			props.rotation = 0;

			m_TerrainPhysical = m_World->CreateStatic(PB_TERRAIN, props);
		}
		m_TerrainPhysical->SetUserData((void *)"Terrain");
		m_TerrainPhysical->SetCoefficientOfFriction(0.8f);

		m_TerrainBitmask = new Bitmask(m_World->GetChipSpace(), m_TerrainPhysical);
		bool bitmaskLoaded = false;
		{
			{
				CL_InputSourceProvider *p = CL_InputSourceProvider::create_file_provider(".");
				try
				{
					bitmaskLoaded = m_TerrainBitmask->Load("bitmask", p);
				}
				catch (FileTypeException& ex)
				{
					bitmaskLoaded = false;
					std::cout << ex.ToString() << std::endl;
				}
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
		//m_TerrainBitmask->SetFromDimensions(::CL_Size(600, 200), m_World->GetBitmaskRes());
		
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

			if (debug)
			{
				//m_World->GetCollisionGrid()->DebugDraw();

				//m_TerrainBitmask->DisplayBits(
				//	m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y);

				m_World->DebugDraw();

				//m_ShipBitmask->DisplayBits(
				//	m_ShipPhysical->GetPosition().x, m_ShipPhysical->GetPosition().y);

				//for (int i = 0; i < g_NumDrones; i++)
				//{
				//	m_DroneBitmask[i]->DisplayBits(
				//		m_DronePhysical[i]->GetPosition().x, m_DronePhysical[i]->GetPosition().y);
				//}
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

				for (std::vector<PhysicsBody*>::iterator it = m_Projectiles.begin(), end = m_Projectiles.end(); it != end; ++it)
				{
					PhysicsBody* body = (*it);
					m_ProjectileGFX->set_angle(body->GetRotation());
					m_ProjectileGFX->draw(body->GetPosition().x, body->GetPosition().y);
				}

				// Draw the ship
				m_ShipGraphical->set_angle(m_ShipPhysical->GetRotationDeg());
				m_ShipGraphical->draw(
					m_ShipPhysical->GetPosition().x, m_ShipPhysical->GetPosition().y);

				/*CL_Surface_DrawParams2 params;
				params.rotate_origin = origin_center;
				params.rotate_angle = m_ShipPhysical->GetRotation();
				params.destX = m_ShipPhysical->GetPosition().x;
				params.destY = m_ShipPhysical->GetPosition().y;

				m_ShipGraphical->draw(params);*/

				//m_ProjectileGFX->set_angle(m_ProjectileGFX


				// Draw the drones
				for (int i = 0; i < g_NumDrones; i++)
				{
					m_DroneGraphical[i]->set_angle(m_DronePhysical[i]->GetRotationDeg());
					m_DroneGraphical[i]->draw(
						m_DronePhysical[i]->GetPosition().x, m_DronePhysical[i]->GetPosition().y);
				}
			}

			display.flip();
			CL_System::keep_alive(2);
		}

		return 0;
	}
} app;

void Explosive::CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts)
{
	m_Env->Detonate(m_MyBody, 32, contacts.front().GetPosiion());
}
