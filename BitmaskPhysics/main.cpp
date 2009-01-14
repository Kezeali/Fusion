#include "../FusionEngine/Common.h"
#include "../FusionEngine/FusionCommon.h"

//#include "../FusionEngine/FusionShapeMesh.h"
//#include "../FusionEngine/FusionPhysicsCollisionGrid.h"
#include "../FusionEngine/FusionPhysicsWorld.h"
#include "../FusionEngine/FusionPhysicsBody.h"
//#include "../FusionEngine/FusionPhysicsUtils.h"
#include "../FusionEngine/FusionPhysicsTypes.h"
#include "../FusionEngine/FusionPhysicsCallback.h"

//#include "../LinearParticle/include/L_Extended.h"

#include "../rstream/mersenne.h"

#include <boost/smart_ptr.hpp>

const int g_NumDrones = 4;
const float g_ThrustForce = 2.6f;

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

class Projectile
{
	PhysicsBodyPtr m_Body;

public:
	Projectile(PhysicsBodyPtr body)
		: m_Body(body),
		m_Detonated(false)
	{}

	bool m_Detonated;

	const Vector2& GetPosition() const
	{
		return m_Body->GetPosition();
	}

	bool IsDetonated() const
	{
		return m_Detonated;
	}

	PhysicsBodyPtr GetBody() const
	{
		return m_Body;
	}

};

typedef std::tr1::shared_ptr<Projectile> ProjectilePtr;

class Explosive : public ICollisionHandler
{
	BitmaskTest* m_Env;
	ProjectilePtr m_MyBody;
	bool m_HasExploded;
	float m_Payload;

public:
	Explosive(BitmaskTest* env, ProjectilePtr body, float payload)
		: m_Env(env),
		m_MyBody(body),
		m_Payload(payload),
		m_HasExploded(false)
	{
	}

	bool CanCollideWith(const PhysicsBody *other)
	{
		return true;
	}

	void AddContact(const PhysicsBody *other, const std::vector<Contact> &contacts);
};

class BitmaskTest : public CL_ClanApplication
{
	//Bitmask *m_ShipBitmask;
	//Bitmask *m_DroneBitmask[4];

	PhysicsWorld *m_World;

	CL_Surface* m_ProjectileGFX;

	typedef std::list<ProjectilePtr> ProjectileList;
	ProjectileList m_Projectiles;
	ProjectileList m_DetonationQueue;

	class Explosion //: public Entity
	{
	public:
		//! Speed is the fraction of the radius to fill per mili-second
		Explosion(const Vector2& position, float radius, float speed, BitmaskTest* env)
			: m_Position(position), m_Radius(radius), m_Speed(speed), m_Env(env), m_Extent(0.f) {}

		void Simulate(unsigned int dt) { m_Extent += m_Radius * m_Speed * dt + 0.5f; if (m_Extent <= m_Radius) m_Env->DetonateWithin(m_Position, m_Extent); }
		bool Done() { return (m_Extent >= m_Radius); }

		const Vector2& GetPosition() const { return m_Position; }
		float GetExtent() const { return m_Extent; }

	protected:
		Vector2 m_Position;
		float m_Radius;
		float m_Speed;
		float m_Extent;

		BitmaskTest* m_Env;
	};

	typedef std::list<Explosion> ExplosionList;
	ExplosionList m_Explosions;

	CL_GraphicContext m_GfxCtx;
	CL_InputContext m_InCtx;

	CL_InputDevice m_Keyboard;

	CL_Surface m_ShipGraphical;
	PhysicsBodyPtr m_ShipPhysical;

	CL_Surface m_DroneGraphical[g_NumDrones];
	PhysicsBodyPtr m_DronePhysical[g_NumDrones];

	CL_Surface m_TerrainGraphical;
	PhysicsBodyPtr m_TerrainPhysical;
	Terrain m_Terrain;
	CL_Canvas m_TerrainCanvas;

	PhysicsBodyPtr m_DamageBody;
	CL_Surface m_Damage;

	int m_ReloadTime;

	//L_ExplosionEffect* m_ExplosionEffect;
	//L_ExplosionEffect* m_SmokeEffect;
	//L_DroppingEffect* m_DroppingEffect;
	//MultipleEffectEmitter* m_ParticleEmitter;
	//L_MotionController m_ParticleMoCon;
	//L_MotionController m_GravityMoCon;
	//L_MotionController m_SmokeMoCon;
	RStream *m_Random;

public:
	void Detonate(ProjectilePtr projectile, float radius, const Vector2& position)
	{
		if (!projectile->IsDetonated())
		{
			float scale = (1.0/8.0f)*(radius);
			int x_offset = m_TerrainPhysical->GetPosition().x;
			int y_offset = m_TerrainPhysical->GetPosition().y;

			m_Damage->set_scale(scale, scale);
			m_Damage->draw(position.x-x_offset, position.y-y_offset, m_TerrainCanvas->get_gc() );
			m_DamageBody->_setPosition(Vector2::zero());
			m_Terrain->Erase(position, radius);

			m_Explosions.push_back(Explosion(position, radius, 0.005f, this));

			//m_ParticleEmitter->emit(position.x, position.y);
			//for (int i = 0; i < 32; ++i)
			//{
			//	float r_x = m_Random->ranged(-radius, radius);
			//	float r_y = m_Random->ranged(-radius, radius);
			//	L_ParticleEffect* new_effect = m_SmokeEffect->new_clone();
			//	new_effect->set_position(position.x+r_x, position.y+r_y);
			//	m_ParticleEmitter->add(new_effect);
			//}

			projectile->m_Detonated = true;
		}
	}

	void DetonateWithin(const Vector2& position, float radius)
	{
		if (m_Projectiles.empty())
			return;

		Vector2 v; float radius2 = radius * radius;
		for (ProjectileList::iterator it = m_Projectiles.begin(), end = m_Projectiles.end(); it != end; ++it)
		{
			v2Subtract((*it)->GetPosition(), position, v);
			if (v.squared_length() < radius2)
			{
				Detonate((*it), radius, (*it)->GetPosition());
			}

		}
	}

	void RunDetonations()
	{
		//if (m_DetonationQueue.empty())
		//	return;
		if (m_Projectiles.empty())
			return;

		for (ProjectileList::iterator it = m_Projectiles.begin(), end = m_Projectiles.end(); it != end; ++it)
		{
			if ((*it)->IsDetonated())
			{
				m_World->RemoveBody((*it)->GetBody());

				it = m_Projectiles.erase(it);
				end = m_Projectiles.end();
				if (it == end)
					break;
			}
		}
		//m_DetonationQueue.clear();
	}

	void RunExplosions(unsigned int dt)
	{
		for (ExplosionList::iterator it = m_Explosions.begin(), end = m_Explosions.end(); it != end; ++it)
		{
			(*it).Simulate(dt);
			if ((*it).Done())
			{
				it = m_Explosions.erase(it);
				end = m_Explosions.end();
				if (it == end)
					break;
			}
		}
	}

private:
	bool Update(unsigned int split)
	{
		m_ReloadTime -= split;

		if (m_Keyboard.get_keycode('R'))
		{
			//m_ShipPhysical->_setForce(Vector2::zero());
			m_ShipPhysical->_setVelocity(Vector2::zero());
			//m_ShipPhysical->_setAcceleration(Vector2::zero());

			m_ShipPhysical->_setPosition(Vector2(50.0f,50.0f));
		}

		// Warp
		if (m_Keyboard.get_keycode(CL_KEY_UP))
		{
			float a = m_ShipPhysical->GetAngle();
			Vector2 force = m_ShipPhysical->GetPosition();
			force.x += sinf(a)*8.0f;
			force.y += -cosf(a)*8.0f;

			m_ShipPhysical->_setPosition(force);
		}

		if (m_Keyboard.get_keycode('S'))
		{
			//float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			//Vector2 force;
			//force.x = -sinf(a)*g_ThrustForce;
			//force.y = cosf(a)*g_ThrustForce;

			//m_ShipPhysical->ApplyForce(force);
			m_ShipPhysical->ApplyForceRelative(-g_ThrustForce);
		}
		else if (m_Keyboard.get_keycode('W'))
		{
			//float a = fe_degtorad( m_ShipPhysical->GetRotation() );
			//Vector2 force;
			//force.x = sinf(a)*g_ThrustForce;
			//force.y = -cosf(a)*g_ThrustForce;

			//m_ShipPhysical->ApplyForce(force);
			m_ShipPhysical->ApplyForceRelative(g_ThrustForce);

			const Vector2 &p = m_ShipPhysical->GetPosition();
			const Vector2 &v = m_ShipPhysical->GetVelocity();

			//m_DroppingEffect->set_position(p.x, p.y);
			//m_DroppingEffect->set_velocity(v.x, v.y);
			//m_DroppingEffect->trigger();
		}

		if (m_Keyboard.get_keycode('A'))
			m_ShipPhysical->SetAngularVelocity(-2.f);

		else if (m_Keyboard.get_keycode('D'))
			m_ShipPhysical->SetAngularVelocity(2.f);

		else
			m_ShipPhysical->SetAngularVelocity(0);

		// Homing mode for drone 0
		if (m_Keyboard.get_keycode('H'))
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

		if (m_Keyboard.get_keycode(CL_KEY_LCONTROL))
			m_ReloadTime -= 51;
		if (m_ReloadTime <= 0 && m_Keyboard.get_keycode(CL_KEY_SPACE))
		{
			m_ReloadTime = 550;

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
			props.dist = rad-1.0f;
			props.radius = props.dist;

			PhysicsBody* body = m_World->CreateBody(PB_PROJECTILE, props);
			ProjectilePtr projectile(new Projectile(body));
			body->SetCollisionHandler(new Explosive(this, projectile, 24));
			body->_setVelocity(m_ShipPhysical->GetVelocity());
			body->ApplyForceRelative(250.0f);

			m_Projectiles.push_back(projectile);
		}

		for (int i = (int)(split*0.1f+0.5f); i > 0; i--)
			m_World->RunSimulation(10);

		RunExplosions(split);
		RunDetonations();

		if (m_Keyboard.get_keycode(CL_KEY_SHIFT))
			split = (int)(split*0.1f);
		//m_ParticleEmitter->run(split);
		//m_DroppingEffect->run(split);

		return true;
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupCore core;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("Console", 80, 10);
		console.redirect_stdio();

		CL_DisplayWindow display("Display", 1024, 768);

		// Random
		m_Random = new RStream(CL_System::get_time());

		m_GfxCtx = display.get_gc();
		m_InCtx = display.get_ic();
		m_Keyboard = m_InCtx.get_keyboard();

		// Particle system
		//L_ParticleSystem::init();

		//m_ParticleMoCon.set_1d_acceleration(-0.0004f);
		//m_GravityMoCon.set_2d_acceleration(L_Vector(0.f, 0.0015f));

		CL_Surface explosionGfx("explosion.png");
		explosionGfx.set_alignment(origin_center);
		CL_Surface smokeGfx("smoke.png");
		smokeGfx.set_alignment(origin_center);
		CL_Surface lightGfx("light16p.png");
		lightGfx.set_alignment(origin_center);

		CL_Surface shockWaveGfx("light16p.png");
		shockWaveGfx.set_alignment(origin_center);

		//L_Particle exPart1(&explosionGfx, 150);
		//exPart1.set_color( L_Color(255,110,60,255) );
		//exPart1.coloring2( L_Color(255,255,250,100), L_Color(60,0,255,60));
		//exPart1.sizing2( 1.6, 0.4 );
		//exPart1.set_motion_controller(&m_ParticleMoCon);

		//L_Particle exPart2(&lightGfx, 355);
		//exPart2.set_color( L_Color(140,130,130,10) );
		//exPart2.coloring2( L_Color(140,130,130,10), L_Color(255,150,20,1), -0.1f);
		//exPart2.set_size(0.6f);
		//exPart2.sizing2( 0.6, 0.00001f);
		//exPart2.rotating4();
		//exPart2.set_motion_controller(&m_GravityMoCon);

		//L_Particle smoke(&smokeGfx, 600);
		//smoke.set_color( L_Color(255,200,110,110) );
		//smoke.coloring2( L_Color(250,180,110,110), L_Color(1,60,60,65) );
		//smoke.set_size(0.2f);
		//smoke.rotating1(0.0001f);
		//smoke.sizing3 ( 0.002f, 50);
		//smoke.set_motion_controller(&m_ParticleMoCon);

		//L_Particle engineTrail(&lightGfx, 240);
		//engineTrail.set_color( L_Color(255,110,110,255) );
		//engineTrail.coloring2( L_Color(255,110,110,255), L_Color(80,100,1,1), 0.001f);
		//engineTrail.sizing2( 1.4, 0.1f );
		////engineTrail.rotating3();

		//m_ExplosionEffect = new L_ExplosionEffect(0,0,18,30,38,0.5f);
		//m_ExplosionEffect->add(&exPart1, 0.3f);
		//m_ExplosionEffect->add(&exPart2, 0.7f);
		//m_ExplosionEffect->set_life(100); //set life of this effect
		//m_ExplosionEffect->set_rotation_distortion(L_2PI);
		//m_ExplosionEffect->set_size_distortion(0.8);
		//m_ExplosionEffect->set_life_distortion(50); //set life distortion for particles
		//m_ExplosionEffect->set_speed_distortion(0.1f);
		//m_ExplosionEffect->initialize();

		//m_SmokeEffect = new L_ExplosionEffect(0, 0, 606, 1, 2, 0.f);
		//m_SmokeEffect->add(&smoke);
		//m_SmokeEffect->set_life(600);
		//m_SmokeEffect->set_rotation_distortion(1.f);
		////m_SmokeEffect->set_speed_distortion(0.00001f);
		//m_SmokeEffect->initialize();

		//m_DroppingEffect = new L_DroppingEffect(0, 0, 30);
		//m_DroppingEffect->add(&engineTrail);
		//m_DroppingEffect->initialize();

		//m_ParticleEmitter  = new MultipleEffectEmitter();
		//m_ParticleEmitter->add_type(m_ExplosionEffect);
		//m_ParticleEmitter->add_type(m_SmokeEffect);

		// World
		m_World = new PhysicsWorld();
		m_World->Initialise(2000, 2000);
		m_World->SetMaxVelocity(20);
		m_World->SetDamping(0.8);
		m_World->SetBodyDeactivationPeriod(10000);
		m_World->SetDeactivationVelocity(0.05f);
		m_World->SetBitmaskRes(4);
		m_World->DeactivateWrapAround();

		//m_World->SetGravity(Vector2(0, 98));

		// Ship
		m_ShipGraphical = new CL_Surface("Body.png");
		m_ShipGraphical->set_rotation_hotspot(origin_center);
		m_ShipGraphical->set_alignment(origin_center);

		//m_ShipBitmask = new Bitmask;
		//m_ShipBitmask->SetFromSurface(m_ShipGraphical, m_World->GetBitmaskRes());
		//m_ShipBitmask->SetFromRadius(m_ShipGraphical->get_width()*0.5f, m_World->GetBitmaskRes());

		{
			PhysicalProperties props;
			props.mass = 0.2f;
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

		m_ProjectileGFX = new CL_Surface("light16p.png");
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

		m_TerrainShapes = new ShapeMesh(m_World, m_TerrainPhysical);
		bool bitmaskLoaded = false;
		// Profiling
		unsigned int loadStart = CL_System::get_time();
		{
			{
				CL_InputSourceProvider *p = CL_InputSourceProvider::create_file_provider(".");
				try
				{
					bitmaskLoaded = m_TerrainShapes->Load("bitmask", p);
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
			m_TerrainShapes->SetFromSurface(m_TerrainGraphical, m_World->GetBitmaskRes(), 25);
			{
				CL_OutputSource_File file("bitmask");
				m_TerrainShapes->Save("bitmask", &file);
			}
		}
		unsigned int timeToLoad = CL_System::get_time() - loadStart;
		if (bitmaskLoaded)
			std::cout << "Took " << timeToLoad << " milis to load the terrain from a binary file" << std::endl;
		else
			std::cout << "Took " << timeToLoad << " milis to load the terrain from a texture in memory" << std::endl;
		//m_TerrainBitmask->SetFromDimensions(::CL_Size(600, 200), m_World->GetBitmaskRes());
		
		int back_pos = 0;
		float sur_y = 250.f;
		float sur_x = 400.f;

		unsigned int inputTimer = 0;
		bool debug = false;

		m_ReloadTime = 0;
		
		unsigned int lastframe = CL_System::get_time();
		unsigned int split = 0;

		while (!m_Keyboard.get_keycode(CL_KEY_ESCAPE))
		{
			if (m_Keyboard.get_keycode('L'))
				display.get_gc()->clear(CL_Color(255, 255, 255));
			else if (m_Keyboard.get_keycode('K'))
				display.get_gc()->clear(CL_Color(128, 200, 236));
			else
				display.get_gc()->clear(CL_Color(0, 0, 0));

			if (m_Keyboard.get_keycode('G'))
				m_World->SetGravity(Vector2(0, 98));
			if (m_Keyboard.get_keycode('T'))
				m_World->SetGravity(Vector2(0.f, 0.f));

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

			if (inputTimer <= time && m_Keyboard.get_keycode('F'))
			{
				inputTimer = time + 1000;
				if (display.is_fullscreen())
					display.set_windowed();
				else
					display.set_fullscreen();
			}

			// Increace bounce
			if (inputTimer <= time && m_Keyboard.get_keycode('Q'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution() + 0.1f;
				m_ShipPhysical->SetCoefficientOfRestitution( fe_min(cor, 1.0f) );

				std::cout << cor << std::endl;
			}
			// Reduce bounce
			if (inputTimer <= time && m_Keyboard.get_keycode('Z'))
			{
				inputTimer = time + 500;

				float cor = m_ShipPhysical->GetCoefficientOfRestitution() - 0.1f;
				m_ShipPhysical->SetCoefficientOfRestitution( fe_max(cor, 0.0f) );

				std::cout << cor << std::endl;
			}

			// Toggle bounce
			if (inputTimer <= time && m_Keyboard.get_keycode('B'))
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
			if (inputTimer <= time && m_Keyboard.get_keycode('P'))
			{
				inputTimer = time + 500;
				debug = !debug;
			}

			const Vector2& tv = m_ShipPhysical->GetPosition();
			float x = fe_clamped(tv.x - display.get_gc()->get_width() *0.5f, 0.f, (float)(m_TerrainGraphical->get_width() - display.get_gc()->get_width()));
			float y = fe_clamped(tv.y - display.get_gc()->get_height()*0.5f, 0.f, (float)(m_TerrainGraphical->get_height() - display.get_gc()->get_height()));
			display.get_gc()->set_translate(-x, -y);
			if (debug)
			{
				m_World->DebugDraw(false);

				for (ExplosionList::iterator it = m_Explosions.begin(), end = m_Explosions.end(); it != end; it++)
				{
					float scale = (*it).GetExtent()/8.f;
					const Vector2& pos = (*it).GetPosition();
					shockWaveGfx.set_scale(scale, scale);
					shockWaveGfx.draw(pos.x, pos.y);
				}
			}
			else
			{

				// Draw the terrain
				m_TerrainGraphical->draw(
					m_TerrainPhysical->GetPosition().x, m_TerrainPhysical->GetPosition().y
					);

				/*params.rotate_origin = origin_center;
				params.rotate_angle = m_TerrainPhysical->GetRotation();
				params.destX = m_TerrainPhysical->GetPosition().x;
				params.destY = m_TerrainPhysical->GetPosition().y;

				m_TerrainGraphical->draw(params);*/

				for (ProjectileList::iterator it = m_Projectiles.begin(), end = m_Projectiles.end(); it != end; ++it)
				{
					PhysicsBody* body = (*it)->GetBody();
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

				// Draw particles
				m_ParticleEmitter->draw();
				m_DroppingEffect->draw();
			}

			display.flip();
			CL_System::keep_alive(2);
		}

		return 0;
	}
} app;

void Explosive::CollisionWith(const PhysicsBody *other, const std::vector<Contact> &contacts)
{
	if (!m_HasExploded && !m_Keyboard.get_keycode(CL_KEY_RCONTROL))
	{
		m_HasExploded = true;
		m_Env->Detonate(m_MyBody, m_Payload, m_MyBody->GetPosition());
	}
}
