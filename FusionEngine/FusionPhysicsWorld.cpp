
#include "FusionPhysicsWorld.h"

/// Fusion
#include "FusionPhysicsCollisionGrid.h"
#include "FusionPhysicsUtils.h"

namespace FusionEngine
{

	FusionPhysicsWorld::FusionPhysicsWorld()
		: m_BitmaskRes(1)
	{
		m_CollisionGrid = new FusionPhysicsCollisionGrid();
	}

	FusionPhysicsWorld::~FusionPhysicsWorld()
	{
	}

	void FusionPhysicsWorld::AddBody(FusionPhysicsBody *body)
	{
		m_Bodies.push_back(body);
	}
	
	void FusionPhysicsWorld::RemoveBody(FusionPhysicsBody *body)
	{
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Bodies.erase(it);
				break;
			}
		}
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(FusionPhysicsResponse *response, int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);

		m_Bodies.push_back(body);
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(FusionPhysicsResponse *response, int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		m_Bodies.push_back(body);
	}

	void FusionPhysicsWorld::DestroyBody(FusionPhysicsBody *body)
	{
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Bodies.erase(it);
				break;
			}
		}

		delete body;
	}

	void FusionPhysicsWorld::Clear()
	{	
		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			delete (*it);
		}

		m_Bodies.clear();
	}

	void FusionPhysicsWorld::RunSimulation(unsigned int split)
	{
		// Move bodies
		//  All objects bounce elastically on impact - this shouldn't effect 
		//  projectile impacts, as they wont have a chance to bounce, being 
		//  destroyed on impact.

		PhysicsBodyList::iterator it = m_Bodies.begin();
		for (;it != m_Bodies.end(); ++it)
		{
			FusionPhysicsBody *cBod = (*it);

			///////////////////
			// Prepare Movement
			CL_Vector2 force = cBod->m_AppliedForce;
			CL_Vector2 damping_vector;

			//  Calculate the damping on the x and y components of the force
			float damping = -(cBod->m_LinearDamping * cBod->m_Velocity);
#ifdef FUSION_PHYS_USE_TRIG
			double a = atan(force.x/force.y);
			damping_vector.x = damping * float(sin(a));
			damping_vector.y = damping * float(cos(a));
#else
			float nx = force.x / force.length();
			float ny = force.y / force.length();

			damping_vector.x = damping * nx;
			damping_vector.y = damping * ny;
#endif

			//  Calculate velocity
			CL_Vector2 accel = (force + damping_vector) / cBod->m_Mass;
			CL_Vector2 veloc = cBod->m_Velocity + accel;

			// m_MaxVelocity is used to prevent objects from reaching crazy speeds
			//  because a lazy level designer didn't put damping triggers in the level.
			if (veloc.squared_length() > m_MaxVelocitySquared)
			{
				// Set acceleration to zero
				cBod->m_Acceleration = 0;

				//! \todo See which method is faster - trig or non-tirg.
				//  First Method (trig):
#ifdef FUSION_PHYS_USE_TRIG
				// Calculate the maximum x and y velocities for this angle
				double a = atan(veloc.x/veloc.y);
				veloc.x = m_MaxVelocity * float(sin(a));
				veloc.y = m_MaxVelocity * float(cos(a));
#else
				//  Alternative method (without trig):
				// Calculate the maximum x and y velocities for this vector
				float nx = veloc.x / veloc.length();
				float ny = veloc.y / veloc.length();

				veloc.x = m_MaxVelocity * nx;
				veloc.y = m_MaxVelocity * ny;
#endif //FUSION_PHYS_USE_TRIG
			}
			else
				// Set acceleration to the calculated value
				cBod->m_Acceleration = accel;

			// Set velocity to the calculated (but capped) value.
			cBod->m_Velocity = veloc;

			// All forces applied in the previous step have been converted to motion.
			cBod->m_AppliedForce = 0;

			//////////////////////
			// Collision detection
			//  Check for collisions of moving objects.
			if (veloc.x + veloc.y != 0)
			{
				PhysicsBodyList bodies = m_CollisionGrid->FindAdjacentBodies(cBod);
				PhysicsBodyList::iterator it = bodies.begin();

				for (; it != bodies.end(); ++it)
				{
					CL_Vector2 point; 

					if (PhysUtil::FindCollisions(&point, veloc, (*it)->GetVelocity(), cBod, (*it)))
					{
						CL_Vector2 norm = PhysUtil::CalculateNormal(point, (*it));
						// Verify the normal
						//  (the normal should be at least 90deg away from the velocity vector
						//  in both directions, assuming the object is not already intersecting)
						if (norm*veloc < 0)
						{
						}

						cBod->CollisionResponse((*it), point);
					}
				}
			}

			////////////////
			// Apply motion
			// After taking collisions into account, apply the calculated motion vectors
			// Linear
			cBod->m_Position += cBod->m_Velocity;
			// Rotation
			cBod->m_Rotation += cBod->m_RotationalVelocity;

		}
	}

	void FusionPhysicsWorld::LevelChange(int level_x, int level_y)
	{
		m_Width = level_x;
		m_Height = level_y;
	}

	void FusionPhysicsWorld::SetMaxVelocity(float maxvel)
	{
		m_MaxVelocity = maxvel;
		m_MaxVelocitySquared = maxvel * maxvel;
	}

	float FusionPhysicsWorld::GetMaxVelocity() const
	{
		return m_MaxVelocity;
	}

	void FusionPhysicsWorld::SetBitmaskRes(int ppb)
	{
		m_BitmaskRes = ppb;
	}

	int FusionPhysicsWorld::GetBitmaskRes() const
	{
		return m_BitmaskRes;
	}

}
