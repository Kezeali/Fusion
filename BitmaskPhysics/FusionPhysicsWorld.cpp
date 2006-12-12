
#include "FusionPhysicsWorld.h"

/// Fusion
#include "FusionPhysicsCollisionGrid.h"
#include "FusionPhysicsUtils.h"

namespace FusionEngine
{

	FusionPhysicsWorld::FusionPhysicsWorld()
		: m_BitmaskRes(1),
		m_Wrap(false),
		m_Width(1), m_Height(1)
	{
		m_CollisionGrid = new FusionPhysicsCollisionGrid();
	}

	FusionPhysicsWorld::~FusionPhysicsWorld()
	{
		Clear();
		delete m_CollisionGrid;
	}

	void FusionPhysicsWorld::AddBody(FusionPhysicsBody *body)
	{
		m_Bodies.push_back(body);

		m_CollisionGrid->AddBody(body);
	}
	
	void FusionPhysicsWorld::RemoveBody(FusionPhysicsBody *body)
	{
		m_CollisionGrid->RemoveBody(body);

		BodyList::iterator it = m_Bodies.begin();
		for (; it != m_Bodies.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Bodies.erase(it);
				break;
			}
		}
	}

	///////////
	// Dynamic
	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);

		m_Bodies.push_back(body);

		m_CollisionGrid->AddBody(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		// BM
		body->SetUsePixelCollisions(props.use_bitmask);
		if (props.bitmask)
			body->SetColBitmask(props.bitmask);
		// AABB
		body->SetUseAABBCollisions(props.use_aabb);
		body->SetColAABB(props.aabb_x, props.aabb_y);
		// DIST
		body->SetUseDistCollisions(props.use_dist);
		body->SetColDist(props.dist);

		m_Bodies.push_back(body);

		m_CollisionGrid->AddBody(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(CollisionCallback response, int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);

		m_Bodies.push_back(body);

		m_CollisionGrid->AddBody(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateBody(CollisionCallback response, int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		// BM
		body->SetUsePixelCollisions(props.use_bitmask);
		if (props.bitmask)
			body->SetColBitmask(props.bitmask);
		// AABB
		body->SetUseAABBCollisions(props.use_aabb);
		body->SetColAABB(props.aabb_x, props.aabb_y);
		// DIST
		body->SetUseDistCollisions(props.use_dist);
		body->SetColDist(props.dist);

		m_Bodies.push_back(body);

		m_CollisionGrid->AddBody(body);

		return body;
	}

	void FusionPhysicsWorld::DestroyBody(FusionPhysicsBody *body)
	{
		m_CollisionGrid->RemoveBody(body);

		BodyList::iterator it = m_Bodies.begin();
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

	//////////
	// Static
	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);

		m_Static.push_back(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		// BM
		body->SetUsePixelCollisions(props.use_bitmask);
		if (props.bitmask)
			body->SetColBitmask(props.bitmask);
		// AABB
		body->SetUseAABBCollisions(props.use_aabb);
		body->SetColAABB(props.aabb_x, props.aabb_y);
		// DIST
		body->SetUseDistCollisions(props.use_dist);
		body->SetColDist(props.dist);

		m_Static.push_back(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(CollisionCallback response, int type)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);

		m_Static.push_back(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(CollisionCallback response, int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		// BM
		body->SetUsePixelCollisions(props.use_bitmask);
		if (props.bitmask)
			body->SetColBitmask(props.bitmask);
		// AABB
		body->SetUseAABBCollisions(props.use_aabb);
		body->SetColAABB(props.aabb_x, props.aabb_y);
		// DIST
		body->SetUseDistCollisions(props.use_dist);
		body->SetColDist(props.dist);

		m_Static.push_back(body);

		return body;
	}

	void FusionPhysicsWorld::DestroyStatic(FusionPhysicsBody *body)
	{
		BodyList::iterator it = m_Static.begin();
		for (; it != m_Static.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Static.erase(it);
				break;
			}
		}

		delete body;
	}

	void FusionPhysicsWorld::Clear()
	{
		{
			BodyList::iterator it = m_Bodies.begin();
			for (; it != m_Bodies.end(); ++it)
			{
				delete (*it);
			}

			m_Bodies.clear();
		}

		{
			BodyList::iterator it = m_Static.begin();
			for (; it != m_Static.end(); ++it)
			{
				delete (*it);
			}

			m_Static.clear();
		}

		// Clear the collision grid, JIC Clear isn't being called in the PhysWorld destructor
		m_CollisionGrid->Clear();
	}

	void FusionPhysicsWorld::RunSimulation(unsigned int split)
	{
		// Move bodies
		//  All objects bounce elastically on impact - this shouldn't effect 
		//  projectile impacts, as they wont have a chance to bounce, being 
		//  destroyed on impact.

		BodyList::iterator a_it = m_Bodies.begin();
		for (;a_it != m_Bodies.end(); ++a_it)
		{
			FusionPhysicsBody *cBod = (*a_it);

			if (cBod->IsActive())
			{
				// This body will probably move, so update it when we resort the gird below...
				m_CollisionGrid->_updateThis(cBod);

				///////////////////
				// Prepare Movement
				CL_Vector2 position = cBod->GetPosition();
				CL_Vector2 force = cBod->GetForce();

				CL_Vector2 accel;
				CL_Vector2 veloc = cBod->GetVelocity();
				float linDamping = cBod->GetCoefficientOfFriction();

				// Calculate the damping on the x and y components of the force
				//CL_Vector2 nveloc = veloc / speed; // normalise
				CL_Vector2 dampForce = veloc * linDamping;

				accel = (force - dampForce) * cBod->GetInverseMass() * split;
				veloc = veloc + accel;

				// [depreciated] by _deactivate()
				//if (veloc.squared_length() < 0.001f)
				//{
				//	accel = CL_Vector2::ZERO;
				//	veloc = CL_Vector2::ZERO;
				//}

				if (m_Wrap)
				{
						// Wrap around
						cBod->_setPosition(CL_Vector2(
							fe_wrap<int>(position.x, 1, m_Width-1), fe_wrap<int>(position.y, 1, m_Height-1)
							));
				}


				// m_MaxVelocity is used to prevent objects from reaching crazy speeds
				//  because a lazy level designer didn't put damping triggers in the level.
				if (veloc.squared_length() > m_MaxVelocitySquared)
				{
					// Set acceleration to zero
					cBod->_setAcceleration(CL_Vector2::ZERO);

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
				{
					// Set acceleration to the calculated value
					cBod->_setAcceleration(accel);
				}

				// Set velocity to the calculated (but capped) value.
				cBod->_setVelocity(veloc);

				// All forces applied in the previous step have been converted to motion.
				cBod->_setForce(CL_Vector2::ZERO);

				///////////////
				// Apply motion

				// Move along velocity vector, or move to point of collision - veloc will be set to
				//  the relavant one
				cBod->m_Position += veloc;
				// Rotation
				cBod->m_Rotation += cBod->m_RotationalVelocity;


				//////////////////////
				// Collision detection
				//  Check for collisions of active objects.

				// Find collidable dynamic bodies
				BodyList check_list = m_CollisionGrid->FindAdjacentBodies(cBod);

				// Append static bodies
				//  Store the length before resize
				size_t length = check_list.size();
				//  Resize and copy
				check_list.resize(check_list.size() + m_Static.size());
				std::copy(m_Static.begin(), m_Static.end(), check_list.begin() + length);

				BodyList::iterator b_it = check_list.begin();
				for (; b_it != check_list.end(); ++b_it)
				{
					// Don't let objects collide against themselves!
					if ((*b_it) == (*a_it))
						continue;

					FusionPhysicsBody *Other = (*b_it);

					// Points of collision will be stored here
					CL_Vector2 cBod_poc; 
					CL_Vector2 Other_poc;

					bool collision = false;

					if (PhysUtil::CollisionCheck(
						cBod->GetPosition(), Other->GetPosition(),
						cBod, Other))
					{
						// The collision is at the current positions.
						cBod_poc = cBod->GetPosition();
						Other_poc = Other->GetPosition();
						collision = true;
					}

					//if (PhysUtil::FindCollisions(
					//	&cBod_poc, &Other_poc, 
					//	veloc, Other->GetVelocity(), 
					//	cBod, Other))
					//	collision = true;

					// If any of the collision checks found collisions
					if (collision)
					{
						CL_Vector2 normal;
						PhysUtil::CalculateNormal(&normal, cBod_poc, Other_poc, cBod, Other);

						//! \todo Collision response for dynamic -> static collisions
						//  Use collision props flag system like bullet

						///////////////////////
						// (simple) Collision response
						CL_Vector2 position = cBod->GetPosition();
						float bounce = cBod->GetCoefficientOfRestitution();

						CL_Vector2 poc;
						PhysUtil::GuessPointOfCollision(
							&poc,
							cBod_poc, Other_poc,
							cBod, Other);

						// Find the perpendicular vector to the vector from the object
						//  to the point of collision
						// perpendicular(position - point-of-collision)
						CL_Vector2 pos_collision_perp = position - poc;
						pos_collision_perp.y = -pos_collision_perp.y;

						// -elasticity * velocity o collision normal
						float impulse_numerator = -(1.0f + bounce) * veloc.dot(normal);

						// perpendicular(position - point-of-collision) o collision normal
						float perp_dot = pos_collision_perp.dot(normal);

						// pos_collision_perp o collision normal / mass
						float impulse_denominator = cBod->GetInverseMass() * perp_dot * perp_dot;

						if (impulse_denominator == 0) // oh noes
							continue;

						float impulse = impulse_numerator / impulse_denominator;

						// velocity + impulse / mass * collision normal
						veloc += normal * ( impulse * cBod->GetInverseMass() );

						//cBod->ApplyForce( normal * ( impulse * cBod->GetInverseMass() ) );


						///////////
						// Finally, call the objects collision response
						//cBod->CollisionResponse(Other, poc);

					} // if (collision)
				} // for (it_b)


				// Check whether this object should be deactivate
				if (CL_System::get_time() > cBod->GetDeactivationTime())
					cBod->_deactivate();

				// Another (better?) way to get the deactivation time
				/*cBod->_setDeactivationTime(cBod->GetDeactivationTime() - split);
				if (cBod->GetDeactivationTime() <= 0)*/

			} // if( IsActive() )

		} // for (it_a)

		// Resort all updated bodies
		m_CollisionGrid->Resort();
	}

	void FusionPhysicsWorld::Initialise(int level_x, int level_y)
	{
		Clear();

		m_CollisionGrid->SetScale(g_PhysGridScale, level_x, level_y);

		m_Width = level_x;
		m_Height = level_y;
	}

	void FusionPhysicsWorld::SetBodyDeactivationPeriod(unsigned int millis)
	{
		// Only update if necessary (as this could be time consuming with a lot of bodies)
		if (m_DeactivationPeriod != millis)
		{
			BodyList::iterator it = m_Bodies.begin();
			for (; it != m_Bodies.end(); ++it)
			{
				(*it)->SetDeactivationPeriod(millis);
			}

			m_DeactivationPeriod = millis;
		}
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
