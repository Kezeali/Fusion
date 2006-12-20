
#include "FusionPhysicsWorld.h"

/// Fusion
#include "FusionPhysicsCollisionGrid.h"
#include "FusionPhysicsUtils.h"

namespace FusionEngine
{
	bool CollisionBefore(Collision *lhs, Collision *rhs)
	{
		if (lhs->First < rhs->First && lhs->Second < rhs->Second)
		{
			if (lhs->First < rhs->Second && lhs->Second < rhs->First)
			{
				return true;
			}
		}

		return false;
	}
	bool CollisionAfter(Collision *lhs, Collision *rhs)
	{
		if (lhs->First < rhs->Second && lhs->Second < rhs->First)
		{
			if (lhs->First < rhs->First && lhs->Second < rhs->Second)
			{
				return true;
			}
		}

		return false;
	}
	bool CollisionEqual(Collision *lhs, Collision *rhs)
	{
		// Both collisions are exactly the same
		if (lhs->First < rhs->First && lhs->Second < rhs->Second)
		{
			return true;
		}

		// Both collisions are essentually the same, with the first and second inverted
		if (lhs->First == rhs->Second && lhs->Second == rhs->First)
		{
			return true;
		}

		return false;
	}


	FusionPhysicsWorld::FusionPhysicsWorld()
		: m_BitmaskRes(1),
		m_Wrap(false),
		m_Width(1), m_Height(1),
		m_DeactivationPeriod(100),
		m_DeactivationVelocity(0.001f),
		m_MaxVelocity(100.0f)
	{
		m_DeactivationVelocitySquared = m_DeactivationVelocity * m_DeactivationVelocity;
		m_MaxVelocitySquared = m_MaxVelocity * m_MaxVelocity;

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
		body->SetCoefficientOfRestitution(props.bounce);

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
		body->SetCoefficientOfRestitution(props.bounce);

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
		body->SetMass(0.0f);

		m_Static.push_back(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this);
		body->SetType(type);
		body->SetMass(0.0f);

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
		body->SetMass(0.0f);

		m_Static.push_back(body);

		return body;
	}

	FusionPhysicsBody *FusionPhysicsWorld::CreateStatic(CollisionCallback response, int type, const PhysicalProperties &props)
	{
		FusionPhysicsBody *body = new FusionPhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(0.0f);

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

		// All collisions found in the Prepare Movement stage will be listed here
		CollisionList collisions;

		///////////////////
		// Prepare Movement

		// In the following section, we check for collisions and calculate the velocity
		//  for the body
		BodyList::iterator a_it = m_Bodies.begin();
		for (;a_it != m_Bodies.end(); ++a_it)
		{
			FusionPhysicsBody *cBod = (*a_it);

			if (cBod->IsActive())
			{
				///////////////////
				// Prepare Movement
				CL_Vector2 position = cBod->GetPosition();
				CL_Vector2 force = cBod->GetForce();

				// Apply engine force
				{
					// Find the direction to apply the engine force
					float direction =
						cBod->GetRotation() + cBod->GetRotationalVelocity() * split * 0.5;

					CL_Vector2 force_vector(
						sinf(fe_degtorad( direction )) * cBod->GetEngineForce(),
						-cosf(fe_degtorad( direction )) * cBod->GetEngineForce()
						);
					force += force_vector;
				}

				CL_Vector2 accel;
				CL_Vector2 veloc = cBod->GetVelocity();
				float linDamping = cBod->GetCoefficientOfFriction();

				// Calculate the damping on the x and y components of the force
				//CL_Vector2 nveloc = veloc / speed; // normalise
				CL_Vector2 dampForce = veloc * linDamping;

				// Finally, calculate the velocity
				accel = (force - dampForce) * cBod->GetInverseMass();
				veloc = veloc + accel;


				// Wrap or pop back at boundries
				if (position.x >= m_Width-1 || position.x <= 1
					|| position.y >= m_Height-1 || position.y <= 1)
				{
					if (m_Wrap)
					{
						cBod->_setPosition(CL_Vector2(
							fe_wrap<int>(position.x, 1, m_Width-1), fe_wrap<int>(position.y, 1, m_Height-1)
							));
					}
					else
					{
						cBod->_setPosition(CL_Vector2(
							fe_clamped<int>(position.x, 1, m_Width-1), fe_clamped<int>(position.y, 1, m_Height-1)
							));
					}
				}


				// m_MaxVelocity is used to prevent objects from reaching crazy speeds
				//  because a lazy level designer didn't put damping triggers in the level.
				if (veloc.squared_length() > m_MaxVelocitySquared)
				{
					// Set acceleration to zero
					cBod->_setAcceleration(CL_Vector2::ZERO);

					//! \todo See which method is faster - trig or non-tirg.
#ifdef FUSION_PHYS_USE_TRIG
					//  First Method (trig):
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
				//  This may not be the final value if a collision is detected
				cBod->_setVelocity(veloc);

				// All forces applied in the previous step have been converted to motion.
				cBod->_setForce(CL_Vector2::ZERO);
				cBod->_setEngineForce(0);


				///////////////////////
				// Collision detection
				//  Check for collisions of active objects.

				// Find the movement vector for current body
				CL_Vector2 cBod_movement = cBod->GetVelocity() * split;

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

					if ( cBod->CanCollideWith(Other) )
					{

						// Find the movement vector for other body
						CL_Vector2 Other_movement = Other->GetVelocity() * split;

						// Position at collisions for each body
						CL_Vector2 cBod_pac; 
						CL_Vector2 Other_pac;


						// Search for collisions
						if (PhysUtil::FindCollisions(
							&cBod_pac, &Other_pac, 
							cBod_movement, Other_movement, 
							cBod, Other))
						{
							CL_Vector2 normal;
							PhysUtil::CalculateNormal(&normal, cBod_pac, Other_pac, cBod, Other);


							///////////////////
							// Error correction
							if (normal == CL_Vector2::ZERO)
							{
								// No need to warp out of non-statics, they should move away eventually
								if (Other->CheckCollisionFlag(C_STATIC))
								{
									// Try to find a valid normal (we don't want to warp if we don't need to)
									PhysUtil::CalculateNormal(
										&normal,
										cBod_pac + cBod_movement, Other_pac,
										cBod, Other);
									if (normal == CL_Vector2::ZERO)
									{

										CL_Vector2 jump_point; 
										CL_Vector2 o_point; // Not used

										float facing = cBod->GetRotation();

										// Get a short vector
										CL_Vector2 escape_ray;

										for (float a = 0.0f; a < 2*PI; a+=0.1f)
										{
											escape_ray.x = -sinf(a) * 50.0f;
											escape_ray.y = cosf(a) * 50.0f;

											if (PhysUtil::FindCollisions(
												&jump_point, &o_point, 
												escape_ray, CL_Vector2::ZERO, 
												cBod, Other, 0.1f, false))
											{
												cBod->m_Position = jump_point;

												continue;
											}
										}

										// If the short jump failed failed, get a really long vector
										escape_ray.x = -sinf(facing) * 500.0f;
										escape_ray.y = cosf(facing) * 500.0f;
										if (PhysUtil::FindCollisions(
											&jump_point, &o_point, 
											escape_ray, CL_Vector2::ZERO, 
											cBod, Other, 0.1f, false))
										{
											cBod->m_Position = jump_point;

											continue;
										}

									}

								}
							} // if (normal == CL_Vector2::ZERO)


							// Normal * veloc should multiply to a nevative (should be opposite
							//  directions) if they don't, the normal is invalid.
							if (veloc != CL_Vector2::ZERO && normal.x * veloc.x > 0 && normal.y * veloc.y > 0)
							{
								// Pop back
								cBod->m_Position = cBod_pac + normal * -g_PhysCollisionJump;

								// Stop movement
								veloc = CL_Vector2::ZERO;
								cBod->_setAcceleration(CL_Vector2::ZERO);
								cBod->_setVelocity(CL_Vector2::ZERO);

								continue;
							} // if (wrong normal direction)

							// End error correction
							///////////////////////

							// Make sure both bodies are active
							cBod->_activate();
							Other->_activate();

							////////////////////
							// Add the collision
							// If there were no errors, add the detected collision to the list
							collisions.push_back(
								new Collision(normal, cBod, Other, cBod_pac, Other_pac)
								);

						} // if (collision)

					} // if ( CanCollideWith() )

				} // for (it_b)

				// End of collision detection
				/////////////////////////////

			} // if( IsActive() )

		} // for (it_a)

		// End of movement preperation
		//////////////////////////////

		//////////////////////
		// Collision response
		// Make sure we don't respond to the same collision twice.
		collisions.sort(CollisionBefore);
		collisions.unique(CollisionEqual);
		collisions.sort(CollisionAfter);
		collisions.unique(CollisionEqual);

		CollisionList::iterator col_it = collisions.begin();
		for (;col_it != collisions.end(); ++col_it)
		{
			CL_Vector2 normal         = (*col_it)->Normal;

			FusionPhysicsBody *first  = (*col_it)->First;
			FusionPhysicsBody *second = (*col_it)->Second;

			CL_Vector2 first_pac      = (*col_it)->First_Position;
			CL_Vector2 second_pac     = (*col_it)->Second_Position;

						
			float first_elasticity = first->GetCoefficientOfRestitution();
			float second_elasticity = second->GetCoefficientOfRestitution();

			// Pop back a bit
			first->m_Position = first_pac + normal * g_PhysCollisionJump;
			first->_setAcceleration(CL_Vector2::ZERO);
			//first->_setVelocity(CL_Vector2::ZERO);

			if (second->GetCollisionFlags() & C_STATIC)
			{
				// --Collision with static--

				// Get the speed of the object
				CL_Vector2 veloc = first->GetVelocity();
				float speed = veloc.length();

				// speed / mass
				float bounce_force = speed * first->GetInverseMass();

				// Calculate the deflection velocity
				veloc = normal * bounce_force * first_elasticity * first->GetMass();

				first->_setVelocity(veloc);
				//first->m_Position += veloc;


				// --OLD METHOD--
				//CL_Vector2 bounce_force = veloc * first->GetInverseMass();

				//float speed = bounce_force.unitize(); //normalise

				// Compute deflection
				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * first_elasticity;

				// *mass to conserve momentum
				//veloc = bounce_force * first->GetMass();

			}
			else
			{
				// --Collision with non-static--

				// Get the speed of both objects
				CL_Vector2 first_veloc = first->GetVelocity();
				CL_Vector2 second_veloc = second->GetVelocity();
				float first_speed = first_veloc.length();
				float second_speed = second_veloc.length();

				/////////
				// FIRST:
				// (speed - other speed) / (mass + other mass)
				float bounce_force = ( first_speed - second_speed ) * 
					( first->GetInverseMass() + second->GetInverseMass() );

				first_veloc = normal * bounce_force * first_elasticity * first->GetMass();

				first->_setVelocity(first_veloc);


				//////////
				// SECOND:
				// (speed - other speed) / (mass + other mass)
				bounce_force = ( second_speed - first_speed ) * 
					( second->GetInverseMass() + first->GetInverseMass() );

				second_veloc = normal * bounce_force * second_elasticity * first->GetMass();

				second->_setVelocity(second_veloc);

				// --OLD METHOD--
				// First:
				//CL_Vector2 bounce_force = ( first_veloc - second_veloc ) *
				// ( first->GetInverseMass() + second->GetInverseMass() );
				//float speed = bounce_force.unitize(); //normalise

				// Compute deflection for first 
				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				// reverse the normalisation of bounce_force
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * first_elasticity;

				// *other mass only, to conserve momentum
				//first_veloc = bounce_force * second->GetMass();

				// Second:
				//bounce_force = ( second_veloc - first_veloc ) * 
				// ( second->GetInverseMass() + first->GetInverseMass() );

				//speed = bounce_force.unitize(); //normalise

				// Compute deflection for second
				//normal *= -1; // invert the normal

				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				// reverse the normalisation of bounce_force
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * second_elasticity;

				// *other mass only, to conserve momentum
				//second_veloc = bounce_force * first->GetMass();

			} // else


			// Finally, call each object's collision callback
			// Find the point where the two objects touch
			CL_Vector2 poc = CL_Vector2(0,0);

			PhysUtil::GuessPointOfCollision(
				&poc,
				first_pac, second_pac,
				first, second);

			first->CollisionWith(second, poc);
			second->CollisionWith(first, poc);


		} // for (it_a)

		// End of collision response
		////////////////////////////


		/////////////////
		// Apply movement

		// All bodies have now been allowed to check for collisions in their
		//  current state, so we can now move them to a new state.
		//  This must be done in a seperate loop, or only one body in each collision
		//  will be able to detect the impact.
		a_it = m_Bodies.begin();
		for (;a_it != m_Bodies.end(); ++a_it)
		{
			FusionPhysicsBody *cBod = (*a_it);

			if (cBod->IsActive())
			{
				// This body will probably move, so update it when we resort the gird below...
				m_CollisionGrid->_updateThis(cBod);

				CL_Vector2 velocity = cBod->GetVelocity();


				// Move along velocity vector
				cBod->m_Position += velocity * split;
				// Rotation
				cBod->m_Rotation += cBod->GetRotationalVelocity() * split;


				////////////////////////////////
				// Deactivate stationary objects
				 
				if (velocity.squared_length() > m_DeactivationVelocitySquared)
				{
					// Reset the deactivation counter
					cBod->_activate();
				}
				else
				{
					cBod->_deactivateAfterCountdown(split);
				}

			}

		} // for (it_a)

		// End of movement application
		//////////////////////////////

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

	void FusionPhysicsWorld::ActivateWrapAround()
	{
		m_Wrap = true;
	}

	void FusionPhysicsWorld::DeactivateWrapAround()
	{
		m_Wrap = false;
	}

	bool FusionPhysicsWorld::UseWrapAround() const
	{
		return m_Wrap;
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

	unsigned int FusionPhysicsWorld::GetBodyDeactivationPeriod() const
	{
		return m_DeactivationPeriod;
	}

	void FusionPhysicsWorld::SetDeactivationVelocity(float minvel)
	{
		m_DeactivationVelocity = minvel;
		m_DeactivationVelocitySquared = minvel * minvel;
	}

	float FusionPhysicsWorld::GetDeactivationVelocity() const
	{
		return m_DeactivationVelocity;
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
