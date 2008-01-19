/*
  Copyright (c) 2006-2007 Fusion Project Team

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

		Dimitrios Christopoulos (dynamic object collision response)
*/

#include "Common.h"

#include "FusionPhysicsWorld.h"

/// Fusion
//#include "FusionPhysicsCollisionGrid.h"
//#include "FusionPhysicsUtils.h"
#include "FusionConsole.h"

namespace FusionEngine
{
	////! Returns true if the first address is lower than the second
	///*!
	// * This (along with CollisionAfter()) is used to sort Collision collections
	// * so that equivilant Collisions are adjcent.
	// */
	//bool CollisionBefore(Collision *lhs, Collision *rhs)
	//{
	//	if (lhs->First < rhs->First && lhs->Second < rhs->Second)
	//	{
	//		if (lhs->First < rhs->Second && lhs->Second < rhs->First)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}
	////! Returns true if the first address is higher than the second
	///*!
	// * This (along with CollisionBefore()) is used to sort Collision collections
	// * so that equivilant Collisions are adjcent.
	// */
	//bool CollisionAfter(Collision *lhs, Collision *rhs)
	//{
	//	if (lhs->First < rhs->Second && lhs->Second < rhs->First)
	//	{
	//		if (lhs->First < rhs->First && lhs->Second < rhs->Second)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}
	////! Returns true if the given Collisions are equal
	///*!
	// * Collisions are considered equal if both the First and Second pointers
	// * have the same address (First and Second point to physical bodies).
	// */
	//bool CollisionEqual(Collision *lhs, Collision *rhs)
	//{
	//	// Both collisions are exactly the same
	//	if (lhs->First == rhs->First && lhs->Second == rhs->Second)
	//	{
	//		return true;
	//	}

	//	// Both collisions are essentually the same, with the first and second inverted
	//	if (lhs->First == rhs->Second && lhs->Second == rhs->First)
	//	{
	//		return true;
	//	}

	//	return false;
	//}


	PhysicsWorld::PhysicsWorld()
		: m_BitmaskRes(1),
		m_Wrap(false),
		m_Width(1), m_Height(1),
		m_DeactivationPeriod(100),
		m_DeactivationVelocity(0.001f),
		m_MaxVelocity(100.0f),
		m_RunningSimulation(false)
	{
		m_DeactivationVelocitySquared = m_DeactivationVelocity * m_DeactivationVelocity;
		m_MaxVelocitySquared = m_MaxVelocity * m_MaxVelocity;

		cpInitChipmunk();
		cpResetShapeIdCounter();
		m_ChipSpace = cpSpaceNew();

		//m_CollisionGrid = new CollisionGrid();
	}

	PhysicsWorld::~PhysicsWorld()
	{
		Clear();
		//delete m_CollisionGrid;
	}

	void PhysicsWorld::AddBody(PhysicsBody *body)
	{
		m_Bodies.push_back(body);

		// Don't ad it if it's static
		if (!body->IsStatic())
			cpSpaceAddBody(m_ChipSpace, body->GetChipBody());

		body->SetWorld(this); // :)
		//m_CollisionGrid->AddBody(body);
	}
	
	void PhysicsWorld::RemoveBody(PhysicsBody *body)
	{
		//m_CollisionGrid->RemoveBody(body);
		// Static bodies don't get added to the space
		if (!body->IsStatic())
			cpSpaceRemoveBody(m_ChipSpace, body->GetChipBody());

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

	const BodyList& PhysicsWorld::GetBodies() const
	{
		return m_Bodies;
	}

	///////////
	// Dynamic
	PhysicsBody *PhysicsWorld::CreateBody(int type)
	{
		PhysicsBody *body = new PhysicsBody(this);
		body->SetType(type);

		m_Bodies.push_back(body);

		cpSpaceAddBody(m_ChipSpace, body->GetChipBody());

		//m_CollisionGrid->AddBody(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateBody(int type, const PhysicalProperties &props)
	{
		PhysicsBody *body = new PhysicsBody(this);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		//// BM
		//body->SetUsePixelCollisions(props.use_bitmask);
		//if (props.bitmask)
		//	body->SetColBitmask(props.bitmask);
		//// AABB
		//body->SetUseAABBCollisions(props.use_aabb);
		//body->SetColAABB(props.aabb_x, props.aabb_y);
		//// DIST
		//body->SetUseDistCollisions(props.use_dist);
		//body->SetColDist(props.dist);

		m_Bodies.push_back(body);

		cpSpaceAddBody(m_ChipSpace, body->GetChipBody());

		if (props.use_dist)
		{
			Shape* shape = new CircleShape(body, 0, props.dist, Vector2::ZERO);
			body->AttachShape(shape);
			AddShape(shape);
		}
		//m_CollisionGrid->AddBody(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateBody(ICollisionHandler* response, int type)
	{
		PhysicsBody *body = new PhysicsBody(this, response);
		body->SetType(type);

		m_Bodies.push_back(body);

		cpSpaceAddBody(m_ChipSpace, body->GetChipBody());
		//m_CollisionGrid->AddBody(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateBody(ICollisionHandler* response, int type, const PhysicalProperties &props)
	{
		PhysicsBody *body = new PhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(props.mass);
		body->SetRadius(props.radius);
		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		//// BM
		//body->SetUsePixelCollisions(props.use_bitmask);
		//if (props.bitmask)
		//	body->SetColBitmask(props.bitmask);
		//// AABB
		//body->SetUseAABBCollisions(props.use_aabb);
		//body->SetColAABB(props.aabb_x, props.aabb_y);
		//// DIST
		//body->SetUseDistCollisions(props.use_dist);
		//body->SetColDist(props.dist);

		m_Bodies.push_back(body);
		cpSpaceAddBody(m_ChipSpace, body->GetChipBody());

		//m_CollisionGrid->AddBody(body);

		return body;
	}

	void PhysicsWorld::DestroyBody(PhysicsBody *body)
	{
		if (m_RunningSimulation)
		{
			m_DeleteQueue.push_back(body);
		}
		else
		{
			//m_CollisionGrid->RemoveBody(body);
			cpSpaceRemoveBody(m_ChipSpace, body->GetChipBody());

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
	}

	//////////
	// Static
	PhysicsBody *PhysicsWorld::CreateStatic(int type)
	{
		PhysicsBody *body = new PhysicsBody(this);
		body->SetType(type);
		body->SetMass(g_PhysStaticMass);

		m_Statics.push_back(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateStatic(int type, const PhysicalProperties &props)
	{
		PhysicsBody *body = new PhysicsBody(this);
		body->SetType(type);
		body->SetMass(g_PhysStaticMass);

		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		//// BM
		//body->SetUsePixelCollisions(props.use_bitmask);
		//if (props.bitmask)
		//	body->SetColBitmask(props.bitmask);
		//// AABB
		//body->SetUseAABBCollisions(props.use_aabb);
		//body->SetColAABB(props.aabb_x, props.aabb_y);
		//// DIST
		//body->SetUseDistCollisions(props.use_dist);
		//body->SetColDist(props.dist);

		m_Statics.push_back(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateStatic(ICollisionHandler* response, int type)
	{
		PhysicsBody *body = new PhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(g_PhysStaticMass);

		m_Statics.push_back(body);

		return body;
	}

	PhysicsBody *PhysicsWorld::CreateStatic(ICollisionHandler* response, int type, const PhysicalProperties &props)
	{
		PhysicsBody *body = new PhysicsBody(this, response);
		body->SetType(type);
		body->SetMass(g_PhysStaticMass);

		body->_setPosition(props.position);
		body->_setRotation(props.rotation);

		//// BM
		//body->SetUsePixelCollisions(props.use_bitmask);
		//if (props.bitmask)
		//	body->SetColBitmask(props.bitmask);
		//// AABB
		//body->SetUseAABBCollisions(props.use_aabb);
		//body->SetColAABB(props.aabb_x, props.aabb_y);
		//// DIST
		//body->SetUseDistCollisions(props.use_dist);
		//body->SetColDist(props.dist);

		m_Statics.push_back(body);

		return body;
	}

	void PhysicsWorld::DestroyStatic(PhysicsBody *body)
	{
		BodyList::iterator it = m_Statics.begin();
		for (; it != m_Statics.end(); ++it)
		{
			if ((*it) == body)
			{
				m_Statics.erase(it);
				break;
			}
		}

		delete body;
	}

	void PhysicsWorld::AddShape(Shape* shape)
	{
		// Check the body type, and process accordingly
		if (shape->GetBody() && shape->GetBody()->IsStatic())
			AddStaticShape(shape);
		else
			cpSpaceAddShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::AddStaticShape(Shape* shape)
	{
		cpSpaceAddStaticShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::RemoveShape(Shape* shape)
	{
		if (shape->GetBody() && shape->GetBody()->IsStatic())
			RemoveStaticShape(shape);
		else
			cpSpaceRemoveShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::RemoveShape(ShapePtr shape)
	{
		if (shape->GetBody() && shape->GetBody()->IsStatic())
			RemoveStaticShape(shape);
		else
			cpSpaceRemoveShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::RemoveStaticShape(Shape* shape)
	{
		cpSpaceRemoveStaticShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::RemoveStaticShape(ShapePtr shape)
	{
		cpSpaceRemoveStaticShape(m_ChipSpace, shape->GetShape());
	}

	void PhysicsWorld::Clear()
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
			BodyList::iterator it = m_Statics.begin();
			for (; it != m_Statics.end(); ++it)
			{
				delete (*it);
			}

			m_Statics.clear();
		}

		//cpSpaceFreeChildren(m_ChipSpace);
		cpSpaceDestroy(m_ChipSpace);

		// Clear the collision grid
		//m_CollisionGrid->Clear();
	}

	void PhysicsWorld::constrainBorders(void* ptr, void* data)
	{
		cpBody* body = (cpBody*)ptr;
		PhysicsWorld* world = (PhysicsWorld*)data;

		if (body->p.x < 0.0f || body->p.x > world->m_Width)
			body->v.x = 0.0f;
		if (body->p.y < 0.0f || body->p.y > world->m_Height)
			body->v.y = 0.0f;

		body->p.x = fe_clamped(body->p.x, 0.0f, world->m_Width);
		body->p.y = fe_clamped(body->p.y, 0.0f, world->m_Height);
	}

	void PhysicsWorld::wrapAround(void* ptr, void* data)
	{
		cpBody* body = (cpBody*)ptr;
		PhysicsWorld* world = (PhysicsWorld*)data;

		if (body->p.x > world->m_Width)
			body->p.x = 0.1f;
		else if (body->p.x < 0.0f)
			body->p.x = world->m_Width;

		if (body->p.y > world->m_Height)
			body->p.y = 0.1f;
		else if (body->p.y < 0.0f)
			body->p.y = world->m_Height;
	}

	void PhysicsWorld::RunSimulation(unsigned int split)
	{
		m_RunningSimulation = true; // bodies can't be deleted

		cpFloat dt = 1.0/60.0/1.0;
		cpSpaceStep(m_ChipSpace, dt);

		m_RunningSimulation = false; // bodies can be deleted

		// Delete bodies
		for (BodyList::iterator it = m_DeleteQueue.begin(), end = m_DeleteQueue.end(); it != end; ++it)
		{
			DestroyBody((*it));
		}
		m_DeleteQueue.clear();

		if (m_Wrap)
			cpArrayEach(m_ChipSpace->bodies, &wrapAround, this);
		else
			cpArrayEach(m_ChipSpace->bodies, &constrainBorders, this);
		//for (BodyList::iterator it = m_Bodies.begin(), end = m_Bodies.end(); it != end; ++it)
		//{
		//}
	}

	void PhysicsWorld::DebugDraw(bool fast)
	{
		if (fast)
		{
			CL_Display::draw_pixel(0, 0, CL_Color::white);
			clBegin(GL_POLYGON);
			cpSpaceHashEach(m_ChipSpace->staticShapes, &drawBodyPoint, NULL);
			cpSpaceHashEach(m_ChipSpace->activeShapes, &drawBodyPoint, NULL);
			clEnd();
		}
		else
		{
			cpSpaceHashEach(m_ChipSpace->staticShapes, &drawObject, NULL);
			cpSpaceHashEach(m_ChipSpace->activeShapes, &drawObject, NULL);
		}
	}


	void PhysicsWorld::Initialise(int level_x, int level_y)
	{
		Clear();

		cpResetShapeIdCounter();
		cpSpaceInit(m_ChipSpace);

		//m_ChipSpace->iterations = 5;
		cpSpaceResizeStaticHash(m_ChipSpace, 10.0, 4999);
		cpSpaceResizeActiveHash(m_ChipSpace, 32.0, 999);

		cpSpaceAddCollisionPairFunc(m_ChipSpace, g_PhysBodyCpCollisionType, 0, &bodyCollFunc, this);
		cpSpaceAddCollisionPairFunc(m_ChipSpace, g_PhysBodyCpCollisionType, g_PhysBodyCpCollisionType, &bodyCollFunc, this);

		//m_CollisionGrid->SetCellSize(g_PhysGridCellW, g_PhysGridCellH, level_x, level_y);

		m_Width = level_x;
		m_Height = level_y;
	}

	void PhysicsWorld::ActivateWrapAround()
	{
		SendToConsole("World: Wrap around activated");
		m_Wrap = true;
	}

	void PhysicsWorld::DeactivateWrapAround()
	{
		SendToConsole("World: Wrap around deactivated");
		m_Wrap = false;
	}

	bool PhysicsWorld::UseWrapAround() const
	{
		return m_Wrap;
	}

	void PhysicsWorld::SetBodyDeactivationPeriod(unsigned int millis)
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

	unsigned int PhysicsWorld::GetBodyDeactivationPeriod() const
	{
		return m_DeactivationPeriod;
	}

	void PhysicsWorld::SetDeactivationVelocity(float minvel)
	{
		m_DeactivationVelocity = minvel;
		m_DeactivationVelocitySquared = minvel * minvel;
	}

	float PhysicsWorld::GetDeactivationVelocity() const
	{
		return m_DeactivationVelocity;
	}


	void PhysicsWorld::SetMaxVelocity(float maxvel)
	{
		m_MaxVelocity = maxvel;
		m_MaxVelocitySquared = maxvel * maxvel;
	}

	float PhysicsWorld::GetMaxVelocity() const
	{
		return m_MaxVelocity;
	}

	void PhysicsWorld::SetDamping(float damping)
	{
		if (damping < 1.1f)
			m_ChipSpace->damping = damping;
	}

	float PhysicsWorld::GetDamping() const
	{
		return m_ChipSpace->damping;
	}

	void PhysicsWorld::SetGravity(const Vector2& grav_vector)
	{
		m_ChipSpace->gravity = cpv(grav_vector.x, grav_vector.y);
	}

	Vector2 PhysicsWorld::GetGravity() const
	{
		return Vector2(m_ChipSpace->gravity.x, m_ChipSpace->gravity.y);
	}
 

	void PhysicsWorld::SetBitmaskRes(int ppb)
	{
		m_BitmaskRes = ppb;
	}

	int PhysicsWorld::GetBitmaskRes() const
	{
		return m_BitmaskRes;
	}

	//const CollisionGrid* PhysicsWorld::GetCollisionGrid() const
	//{
	//	return m_CollisionGrid;
	//}

}

				// Apply relative force
				//if (engine_force != 0.0f)
				//{
				//	//Vector2 v = b1->GetVelocity();
				//	//Vector2 n = v.normal();

				//	// force initial
				//	Vector2 fi = b1->GetRelativeForce();
				//	Vector2 v = b1->GetVelocity();

				//	// force final
				//	//Vector2 ff = fi

				//	//float angle = ff.angleFrom(fi);

				//	// Calculate the damping on the force
				//	Vector2 dampForce = v * linDamping;

				//	// Valculate the acceleration
				//	Vector2 a = (fi - dampForce) * b1->GetInverseMass();

				//	// Then the initial velocity is:
				//	Vector2 vi = a*delta;


				//	// Now for Vf
				//	float speed = vi.length();

				//	float direction =
				//		fe_degtorad(
				//			b1->GetRotation() + b1->GetRotationalVelocity() * delta
				//			);

				//	Vector2 vf(
				//		sinf( direction ) * speed,
				//		-cosf( direction ) * speed
				//		);

				//	float angle = vf.angleFrom(vi);

				//	float dist = speed * delta + a.length() *0.5f*delta*delta;

				//	// Calculate the radius (r = Segment/theta)
				//	float r = dist / angle;

				//	// Displacement
				//	Vector2 p(
				//		r*sinf(angle),
				//		r*-cosf(angle)
				//		);

				//	//velocity += p;
				//}


				//Vector2 fi = b1->GetRelativeForce();
				//if (fi.length() > 0.0f)
				//{
				//	//Vector2 v = b1->GetVelocity();
				//	//Vector2 n = v.normal();

				//	// force initial
				//	//Vector2 fi = b1->GetRelativeForce();

				//	// force final
				//	//Vector2 ff = fi

				//	//float angle = ff.angleFrom(fi);

				//	// Calculate the damping on the force
				//	//Vector2 dampForce = velocity * b1->GetCoefficientOfFriction();

				//	// Valculate the acceleration
				//	Vector2 a = fi * b1->GetInverseMass();

				//	// Then the initial velocity is:
				//	Vector2 vi = a*delta;


				//	// Now for Vf
				//	float speed = vi.length();

				//	float direction =
				//		fe_degtorad(
				//			b1->GetRotation() + b1->GetRotationalVelocity() * delta
				//			);

				//	Vector2 vf(
				//		sinf( direction ) * speed,
				//		-cosf( direction ) * speed
				//		);

				//	float angle = vi.angleFrom(vf);

				//	if (angle > 0.1f)
				//	{
				//		float dist = speed * delta;// + a.length() *0.5f*delta*delta;

				//		// Calculate the radius (r = Segment/theta)
				//		float r = dist / angle;

				//		// Displacement
				//		Vector2 p(
				//			r*sinf(angle),
				//			r*-cosf(angle)
				//			);

				//		b1->m_Position += p*delta;
				//		//velocity += p;
				//	}
				//	else
				//		b1->m_Position += vi*delta;
				//}
				//b1->_setRelativeForce(Vector2::ZERO);



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
