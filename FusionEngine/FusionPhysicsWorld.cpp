/*
  Copyright (c) 2006 Fusion Project Team

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


	File Author(s):

		Elliot Hayward

		Dimitrios Christopoulos (dynamic-dynamic object collision response)
*/

#include "FusionPhysicsWorld.h"

/// Fusion
#include "FusionPhysicsCollisionGrid.h"
#include "FusionPhysicsUtils.h"

namespace FusionEngine
{
	//! Returns true if the first address is lower than the second
	/*!
	 * This (along with CollisionAfter()) is used to sort Collision collections
	 * so that equivilant Collisions are adjcent.
	 */
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
	//! Returns true if the first address is higher than the second
	/*!
	 * This (along with CollisionBefore()) is used to sort Collision collections
	 * so that equivilant Collisions are adjcent.
	 */
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
	//! Returns true if the given Collisions are equal
	/*!
	 * Collisions are considered equal if both the First and Second pointers
	 * have the same address (First and Second point to physical bodies).
	 */
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

		// Clear the collision grid, JIC this isn't being called from the PhysWorld destructor
		m_CollisionGrid->Clear();
	}

	void FusionPhysicsWorld::RunSimulation(unsigned int split)
	{
		float delta = (float)split;// * 0.1f;

		// All collisions found in the Prepare Movement stage will be listed here
		CollisionList collisions;

		///////////////////
		// Prepare Movement

		// In the following section, we check for collisions and calculate the velocity
		//  for the body
		BodyList::iterator a_it = m_Bodies.begin();
		for (;a_it != m_Bodies.end(); ++a_it)
		{
			FusionPhysicsBody *b1 = (*a_it);

			if (b1->IsActive())
			{
				Vector2 position = b1->GetPosition();
				Vector2 force = b1->GetForce();

				// Apply engine force
				{
					// Find the direction to apply the engine force
					float direction =
						b1->GetRotation();// + b1->GetRotationalVelocity() * delta;

					float engine_force = b1->GetEngineForce();

					Vector2 force_vector(
						sinf(fe_degtorad( direction )) * engine_force,
						-cosf(fe_degtorad( direction )) * engine_force
						);
					force += force_vector;
				}

				Vector2 accel;
				Vector2 veloc = b1->GetVelocity();
				float linDamping = b1->GetCoefficientOfFriction();

				// Calculate the damping on the force
				//Vector2 nveloc = veloc / speed; // normalise
				Vector2 dampForce = veloc * linDamping;

				// Finally, calculate the acceleration
				accel = (force - dampForce) * b1->GetInverseMass();

				///////////////////
				// Cap the velocity
				// m_MaxVelocity is used to prevent objects from reaching crazy speeds
				//  because a lazy level designer didn't put damping triggers in the level.
				if (veloc.squared_length() > m_MaxVelocitySquared)
				{
					// Set acceleration to zero
					b1->_setAcceleration(Vector2::ZERO);

					//! \todo See which method is faster - trig or non-tirg.
#ifdef FUSION_PHYS_USE_TRIG
					// Method1 (trig):
					// Calculate the maximum x and y velocities for this angle
					double a = atan(veloc.x/veloc.y);
					veloc.x = m_MaxVelocity * float(sin(a));
					veloc.y = m_MaxVelocity * float(cos(a));
#else
					// Method2 (without trig):
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
					b1->_setAcceleration(accel);
				}

				// Set velocity to the capped value.
				//  This may not be the final value if a collision is detected
				b1->_setVelocity(veloc);
				// End Cap the velocity
				///////////////////////

				// Prepare forces for next step.
				b1->_setForce(Vector2::ZERO);
				b1->_setEngineForce(0);


				// Wrap or pop back at boundries
				if (position.x >= m_Width-1 || position.x <= 1
					|| position.y >= m_Height-1 || position.y <= 1)
				{
					if (m_Wrap)
					{
						b1->_setPosition(Vector2(
							fe_wrap<float>(position.x, 1.f, (float)m_Width-1), fe_wrap<float>(position.y, 1.f, (float)m_Height-1.f)
							));
					}
					else
					{
						b1->_setPosition(Vector2(
							fe_clamped<float>(position.x, 1.f, (float)m_Width-1.f), fe_clamped<float>(position.y, 1.f, (float)m_Height-1.f)
							));
					}
				}


				///////////////////////
				// Collision detection
				//  Check for collisions of active objects.

				// Find the movement vector for current body
				Vector2 b1_velocity = b1->GetVelocity() * delta + b1->GetAcceleration()*0.5f*delta*delta;

				// Find collidable dynamic bodies
				BodyList check_list = m_CollisionGrid->FindAdjacentBodies(b1);

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

					FusionPhysicsBody *b2 = (*b_it);

					if ( b1->CanCollideWith(b2) )
					{

						// Find the movement vector for b2 body
						Vector2 b2_velocity = b2->GetVelocity() * delta + b2->GetAcceleration()*0.5f*delta*delta;

						// Contact positions for each body will go here
						Vector2 b1_ct; 
						Vector2 b2_ct;
						// Non-contact positions
						Vector2 b1_nc;
						Vector2 b2_nc;

						// Search for collisions
						if (PhysUtil::FindCollisions(
							&b1_ct, &b2_ct, 
							&b1_nc, &b2_nc, 
							b1_velocity, b2_velocity, 
							b1, b2))
						{
							Vector2 normal;
							PhysUtil::CalculateNormal(&normal, b1_ct, b2_ct, b1, b2);

							///////////////////
							// Error correction
							if (normal == Vector2::ZERO)
							{
								// No need to warp out of non-statics, they should move away eventually
								if (b2->CheckCollisionFlag(C_STATIC))
								{
									// Try to find a valid normal (we don't want to warp if we don't need to)
									PhysUtil::CalculateNormal(
										&normal,
										b1_ct + b1_velocity, b2_ct,
										b1, b2);
									if (normal == Vector2::ZERO)
									{

										Vector2 jump_point; 
										Vector2 nu; // Not used

										float facing = b1->GetRotation();

										// Get a short vector
										Vector2 escape_ray;

										for (float a = 0.0f; a < 2*PI; a+=0.1f)
										{
											escape_ray.x = -sinf(a) * 50.0f;
											escape_ray.y = cosf(a) * 50.0f;

											if (PhysUtil::FindCollisions(
												&jump_point, &nu, 
												&nu, &nu,
												escape_ray, Vector2::ZERO, 
												b1, b2, 0.1f, false))
											{
												b1->m_Position = jump_point;

												continue;
											}
										}

										// If the short jump failed failed, get a really long vector
										escape_ray.x = -sinf(facing) * 500.0f;
										escape_ray.y = cosf(facing) * 500.0f;
										if (PhysUtil::FindCollisions(
											&jump_point, &nu,
											&nu, &nu,
											escape_ray, Vector2::ZERO, 
											b1, b2, 0.1f, false))
										{
											b1->m_Position = jump_point;

											continue;
										}

									}

								}
							} // if (normal == Vector2::ZERO)

							// End error correction
							///////////////////////

							// Make sure both bodies are active
							b1->_activate();
							b2->_activate();

							////////////////////
							// Add the collision
							// If there were no errors, add the detected collision to the list
							//  Notice the non-contact (_nc) positions are used here, this
							//  will be used to correct the penetration later
							collisions.push_back(
								new Collision(normal, b1, b2, b1_nc, b2_nc)
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
			Vector2 normal         = (*col_it)->Normal;

			FusionPhysicsBody *cb1 = (*col_it)->First;
			FusionPhysicsBody *cb2 = (*col_it)->Second;

			Vector2 b1_pos         = (*col_it)->First_Position;
			Vector2 b2_pos         = (*col_it)->Second_Position;

						
			float cb1_elasticity = cb1->GetCoefficientOfRestitution();
			float cb2_elasticity = cb2->GetCoefficientOfRestitution();

			float cb1_friction = cb1->GetCoefficientOfFriction(); // not used
			float cb2_friction = cb2->GetCoefficientOfFriction();

			// Pop back a bit
			//Vector2 l_accel = normal * cb1->GetAcceleration().length();
			//Vector2 speed = normal * cb1->GetVelocity().length();
			cb1->_setPosition(b1_pos);// + (speed * delta + l_accel*0.5f*delta*delta));// (speed * delta + g_PhysCollisionJump * delta));

			// --Collision with static--
			if (cb2->GetCollisionFlags() & C_STATIC)
			{
				//cb1->GetAcceleration() * delta;
				Vector2 v = cb1->GetVelocity();

				// Calculate the deflection velocity
				if (v.dot(normal) < 0)
				{
					Vector2 bounce = v.project((-normal)) * cb1_elasticity;
					Vector2 frictn = v.project((-normal).perpendicular()) * cb2_friction;

					cb1->_setVelocity((-bounce) + frictn);
				}
				else
				{
					Vector2 bounce = v.project(normal) * cb1_elasticity;
					Vector2 frictn = v.project(normal.perpendicular()) * cb2_friction;
					cb1->_setVelocity(-bounce + frictn);
				}
			}
			// --Collision with non-static--
			else
			{
				Vector2 v1 = cb1->GetVelocity();
				Vector2 v2 = cb1->GetVelocity();
				float m1 = cb1->GetMass();
				float m2 = cb2->GetMass();
				float im1 = cb1->GetInverseMass();
				float im2 = cb2->GetInverseMass();

				// Get coeff. of elasticity
				float e = cb1_elasticity * cb2_elasticity;
				float mt = 1.0f/(m1 + m2);

				float s1 = -(v1.length());
				float s2 = v2.length();

				s1 = s1*((m1-m2)*mt) + s2*((2*m2)*mt);
				s2 = s1*((2*m1)*mt) + s2*((m2-m1)*mt);

				// --Dimitrios Christopoulos's Solution--
				Vector2 pb1,pb2,dpos,U1x,U1y,U2x,U2y,V1x,V1y,V2x,V2y,
					vf1,vf2;
				double a,b;

				dpos=(b2_pos-b1_pos).normalized();  // Find X-Axis (delta-position, normalized)
				a=dpos.dot(v1);                     // Find Projection
				U1x=dpos*a;                         // Find Projected Vectors
				U1y=v1-U1x;

				dpos=(pb1-pb2).normalized();        // Same as above, for b2
				b=dpos.dot(v2);                     // Find Projection
				U2x=dpos*b;                         // Vectors For The Other Object
				U2y=v2-U2x;

				V1x=(U1x+U2x-(U1x-U2x))*0.5* m2*im1;  // Now Find New Velocities
				V2x=(U1x+U2x-(U2x-U1x))*0.5* m1*im2;
				V1y=U1y*e;//*m2*im1;
				V2y=U2y*e;//*m1*im2;

				vf1=V1x+V1y;                  // Set New Velocity Vectors
				vf2=V2x+V2y;                  // To The Colliding Balls


				cb1->_setVelocity((vf1));//.normalized()) );
				cb2->_setVelocity((vf2));//.normalized()) );
				

			} // else


			//////////
			// Finally, call each object's collision callback

			// Find the point where the two objects touch
			Vector2 poc = Vector2(0,0);

			PhysUtil::GuessPointOfCollision(
				&poc,
				b1_pos, b2_pos,
				cb1, cb2);

			cb1->CollisionWith(cb2, poc);
			cb2->CollisionWith(cb1, poc);


		} // for (coll_it)

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
			FusionPhysicsBody *b1 = (*a_it);

			if (b1->IsActive())
			{
				// This body will probably move, so update it when we resort the gird below...
				m_CollisionGrid->_updateThis(b1);

				Vector2 velocity = b1->GetVelocity();
				Vector2 acceleration = b1->GetAcceleration();
				float rot_velocity = b1->GetRotationalVelocity();

				/////////
				// Move along velocity vector
				b1->m_Position += velocity * delta + acceleration*0.5f*delta*delta;
				b1->m_Velocity += acceleration * delta;
				/////////
				// Rotate
				b1->m_Rotation += rot_velocity * delta;


				////////////////////////////////
				// Deactivate stationary objects
				 
				if (velocity.squared_length() > m_DeactivationVelocitySquared ||
					rot_velocity > 0)
				{
					// Reset the deactivation counter
					b1->_activate();
				}
				else
				{
					b1->_deactivateAfterCountdown(split);
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

				///////////
				//// cb1:
				//// If the velocity is the same direction as the normal, bounce forward
				////float s = v1.dot(normal);
				////assert(s < 0.0f);
				//if (v1.dot(normal) < 0.0f)
				//{
				//	Vector2 bounce = vi1.project((-normal)) * e;
				//	Vector2 frictn = vi1.project((-normal).perpendicular());// * u;

				//	cb1->_setVelocity((-bounce) + frictn);
				//}
				////else
				////{
				////	Vector2 bounce = v1.project(normal) * e;
				////	Vector2 frictn = v1.project(normal.perpendicular());// * u;

				////	cb1->_setVelocity((-bounce) + frictn);
				////}

				////////////
				//// cb2:
				//// If the velocity is the same direction as the normal
				////s = v2.dot(normal) > 0.0f ? 1.0f : -1.0f;
				////if (v2.dot(normal) < 0.0f)
				////{
				////	Vector2 bounce = v2.project((-normal)) * e;
				////	Vector2 frictn = v2.project((-normal).perpendicular());// * u;

				////	cb2->_setVelocity((-bounce) + frictn);
				////}
				////else
				//if (v2.dot(normal) > 0.0f)
				//{
				//	Vector2 bounce = vi2.project(normal) * e;
				//	Vector2 frictn = vi2.project(normal.perpendicular());// * u;

				//	cb2->_setVelocity(bounce + frictn);
				//}

				//cb1:
				//float bounce_force = ( cb1_speed - cb2_speed );

				//cb1_veloc += normal * bounce_force * cb1_elasticity * cb1->GetInverseMass();
				//cb1_veloc += normal * cb1_elasticity * cb1->GetInverseMass();

				//cb1->_setAcceleration(first_veloc);
				

				//cb2:
				// (speed - b2 speed) / (mass + b2 mass)
				/*bounce_force = ( cb2_speed - cb1_speed ) * 
					( cb2->GetInverseMass() + cb1->GetInverseMass() );*/

				//cb2_veloc += normal * bounce_force * cb2_elasticity * cb2->GetInverseMass();
				//cb2_veloc += -normal * cb2_elasticity * cb2->GetInverseMass();

				//cb2->_setAcceleration(-cb2_veloc);


				/////////
				//// Prep
				////
				//// Get the speed of both objects
				//Vector2 cb1_veloc = cb1->GetVelocity();
				//Vector2 cb2_veloc = cb2->GetVelocity();
				//float cb1_speed = cb1_veloc.length();
				//float cb2_speed = cb2_veloc.length();

				//// Get coeff. of elast. and friction
				//float e = cb1_elasticity * cb2_elasticity;
				////float u = cb1_damping * cb2_damping;

				//// Calc. normal mass
				//float mass_sum = cb1->GetInverseMass() + cb2->GetInverseMass();

				//Vector2 r1 = normal * -cb1->GetRadius(); //vectors to contact point
				//Vector2 r2 = normal * cb2->GetRadius();//

				//float r1cn = r1.cross(normal);
				//float r2cn = r2.cross(normal);
				//float kn = mass_sum; // Here we /would/ apply rotational properties, but Fusion doesn't use them :P
				//float nMass = 1.0f/kn;

				//// Difference in velocity
				//Vector2 dv = cb2_veloc - cb1_veloc;

				//// Calc. bounce
				//float bounce = normal.dot(cb2_veloc - cb1_veloc) * e;

				//////////////////
				//// Apply Impluse
				////
				//// Normal impluse
				//float dv_dot_n = dv.dot(normal);

				//float jn = -(bounce + dv_dot_n)*nMass;
				//jn = fe_max<float>(jn, 0.0f);

				//assert(jn >= 0);
				//if (jn > 0)
				//	jn = jn;


				//Vector2 j = (normal * jn);// - (normal * u);

				////assert(j.x * cb2_veloc.x >= 0);
				////assert(j.y * cb2_veloc.y >= 0);

				//cb1_veloc = cb1_veloc + (j * cb1->GetInverseMass());
				//cb2_veloc = cb2_veloc + (-j * cb2->GetInverseMass());

				////cb1_veloc += bounce;
				////cb2_veloc -= bounce;

				// --ANOTHER METHOD OF RESPONSE--
				//// Precompute normal mass
				//float kNormal = cb1->GetInverseMass() + cb2->GetInverseMass();
				//float massNormal = 1.0f / kNormal;

				//// Relative velocity
				//Vector2 dv = cb1_veloc - cb2_veloc;

				//// Compute normal impulse
				//float vn = dv.dot(normal);
				//float normalImpulse = massNormal * -vn;

				//// Apply contact impulse
				//Vector2 impulse = normal * normalImpulse;

				//cb1->m_Velocity -= impulse * cb1->GetInverseMass();

				//cb2->m_Velocity += impulse * cb2->GetInverseMass();

			
				// --OLDER METHOD OF RESPONSE--
				// cb1:
				//Vector2 bounce_force = ( cb1_veloc - cb2_veloc ) *
				// ( cb1->GetInverseMass() + cb2->GetInverseMass() );
				//float speed = bounce_force.unitize(); //normalise

				// Compute deflection for cb1 
				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				// reverse the normalisation of bounce_force
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * cb1_elasticity;

				// *other mass only, to conserve momentum
				//cb1_veloc = bounce_force * cb2->GetMass();

				// cb2:
				//bounce_force = ( cb2_veloc - cb1_veloc ) * 
				// ( cb2->GetInverseMass() + cb1->GetInverseMass() );

				//speed = bounce_force.unitize(); //normalise

				// Compute deflection for cb2
				//normal *= -1; // invert the normal

				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				// reverse the normalisation of bounce_force
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * cb2_elasticity;

				// *other mass only, to conserve momentum
				//cb2_veloc = bounce_force * cb1->GetMass();


				// --OLD METHOD--
				//Vector2 bounce_force = veloc * cb1->GetInverseMass();

				//float speed = bounce_force.unitize(); //normalise

				// Compute deflection
				//bounce_force = (normal*(2*normal.dot(-bounce_force))) + bounce_force;
				//bounce_force.unitize();
				//bounce_force = bounce_force * speed * cb1_elasticity;

				// *mass to conserve momentum
				//veloc = bounce_force * cb1->GetMass();
