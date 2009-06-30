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
*/

#ifndef Header_FusionEngine_PhysicsWorld
#define Header_FusionEngine_PhysicsWorld

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsBody.h"
#include "FusionPhysicsCallback.h"
#include "FusionPhysicsDebugDraw.h"

#include "FusionRefCounted.h"


namespace FusionEngine
{

	static const int g_PhysGridCellW = 128;
	static const int g_PhysGridCellH = 128;
	static const float g_PhysCollisionJump = 0.25f;

	//! Used for initialising bodies.
	/*!
	 * \todo Add 'bounce'
	 */
	struct PhysicalProperties
	{
		//! Constructor
		PhysicalProperties()
			: mass(0.0f),
			radius(0.0f),
			//bounce(0.0f),

			position(Vector2::zero()),
			rotation(0.0f),

			//use_bitmask(false),
			//use_aabb(false),
			//bitmask(0),
			use_dist(false),
			dist(0)
			{}

		//! Mass.
		float mass;
		//! Radius. Not used
		float radius;
		////! Coefficiant of restitution
		//float bounce;

		//! Initial Position.
		Vector2 position;
		//! Initial Rotation.
		float rotation;

		////! Use Pixel based collision detection.
		//bool use_bitmask;
		////! Use the given bitmask.
		//Bitmask *bitmask;
		////! Use AABB based collision detection.
		//bool use_aabb;
		////! Make an AABB with the given (upright) dimensions
		//float aabb_x, aabb_y;
		////! Use BB based collision detection.
		//bool use_bb;
		////! Make a BB with the given dimensions
		//float bb_x, bb_y;
		//! Use Distance based collision detection.
		bool use_dist;
		//! Use the given distance (for distance based collisions.)
		float dist;

	};

	class ContactListener : public b2ContactListener
	{
	public:
		ContactListener(PhysicsWorld *world);

		virtual void BeginContact(b2Contact* contact);
		virtual void EndContact(b2Contact* contact);
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		virtual void PostSolve(const b2Contact* contact, const b2ContactImpulse* impulse);

	protected:

		PhysicsWorld *m_World;
	};

	class ContactFilter : public b2ContactFilter
	{
	public:
		ContactFilter(PhysicsWorld *world);

		virtual bool ShouldCollide(b2Fixture *shape1, b2Fixture *shape2);
		virtual bool RayCollide(void *userData, b2Fixture *shape);

	protected:
		PhysicsWorld *m_World;
	};

	/*!
	 * \brief
	 * The controller for moving objects
	 *
	 * It is recomended that you use the factory methods (CreateBody(...), 
	 * DestroyBody(...)) rather than constructing bodies manually.
	 *
	 * \see
	 * PhysicsBody.
	 */
	class PhysicsWorld
	{
		friend class ContactListener;
	public:
		//! Constructor.
		PhysicsWorld();
		//! Constructor +width/height
		PhysicsWorld(float width, float height);
		//! Virtual destructor.
		virtual ~PhysicsWorld();

	public:
		//! List of Collsions
		//typedef std::list<Collision *> CollisionList;
		//typedef std::vector<PhysicsBody *> BodyList;

		typedef std::tr1::unordered_map<b2Body*, PhysicsBodyPtr> BodyMap;
		//! Type for a list of bodies
		typedef std::list<PhysicsBodyPtr> BodyList;

		//typedef std::list<b2ContactPoint> ContactList;

	public:
		//! Adds the given body to the world and initializes it
		/*!
		 * \param body Pointer to the body to add.
		 */
		void AddBody(PhysicsBodyPtr body);
		//! Removes the given body from the world.
		/*!
		 * \param body Pointer to the body to remove.
		 */
		void RemoveBody(PhysicsBodyPtr body);

		//! Internal method called by Body objects upon their destruction
		void _destroyBody(PhysicsBody *body);

		//! Returns a list of bodies currently in the world
		const BodyMap& GetBodies() const;

		//cpShape* createSimpleStatic(PhysicsBody* body = NULL);
		//void removeSimpleStatic(cpShape* shape);

		//! \todo Shape factory methods?
		//Shape* CreateShape();
		//Shape* CreateStaticShape();
		//void DestroyShape(Shape* shape);
		//void DestroyStaticShape(Shape* shape);

		//! Removes all bodies and statics.
		void Clear();

		//! Does movement and collision detection.
		/*!
		 * \param split Step magnitude (millis since last step.)
		 *
		 */
		void RunSimulation(float delta_milis);

		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnBeginContact(b2Contact *contact);
		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnEndContact(b2Contact *contact);
		//! Called via contact listener
		void OnPreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnPostSolve(const b2Contact *contact, const b2ContactImpulse *impulse);

		void ClearContacts();

		void SetGCForDebugDraw(CL_GraphicContext gc);

		//! Draws all shapes in the simulation
		//void DebugDraw(bool fast = true);

		//! Resets the CollisonGrid and sets the borders up, etc.
		/*!
		 * Call this every time a new level is loaded.
		 *
		 * \remarks
		 * Remember to add a terrain body for the new level! (use AddBody())
		 */
		//void Initialise(int level_x, int level_y);

		//! Activates wrap around
		void ActivateWrapAround();

		//! Deactivates wrap around
		void DeactivateWrapAround();

		//! Returns true if wrap around is active
		bool UseWrapAround() const;

		//! Set the time in milis for bodies to de-activate after.
		void SetBodyDeactivationPeriod(unsigned int millis);
		//! Returns the current deactivation period for bodies.
		unsigned int GetBodyDeactivationPeriod() const;

		//! Sets the minimum velocity a body can be moving before it will be deactivated.
		void SetDeactivationVelocity(float mixvel);
		//! Returns the minimum velocity a body can be moving before it will be deactivated.
		float GetDeactivationVelocity() const;

		//! Allows the maximum velocity to be set
		void SetMaxVelocity(float maxvel);
		//! Returns the current maximum velocity
		float GetMaxVelocity() const;

		void SetDamping(float damping);
		float GetDamping() const;

		void SetGravity(const Vector2& grav_vector);
		Vector2 GetGravity() const;

		//! Allows the bitmask scale to be set (pixels per bit)
		void SetBitmaskRes(int ppb);
		//! Returns the scale bodies should use for their bitmasks
		int GetBitmaskRes() const;

		//const CollisionGrid* GetCollisionGrid() const;

		b2World* GetInternalSpace() const
		{
			return m_BxWorld;
		}

	private:
		// Chipmunk space
		//cpSpace* m_ChipSpace;

		ContactListener *m_ContactListener;
		DebugDraw *m_DebugDraw;
		// Box2d world
		b2World *m_BxWorld;

		//b2AABB m_WorldAABB;
		//b2Vec2 m_Gravity;
		//bool m_CanSleep;

		//! All physical objects controled by this world.
		BodyMap m_Bodies;
		// Bodies to be deleted after the simulation is complete
		BodyList m_DeleteQueue;

		bool m_RunningSimulation;

		int m_VelocityIterations;
		int m_PositionIterations;

		//ContactList m_NewContacts;
		//ContactList m_ActiveContacts;
		//ContactList m_EndedContacts;

		//! World dimensions
		float m_Width, m_Height;

		//! True if objects should wrap around.
		bool m_Wrap;

		//! Speed scale
		float m_StepMagnitude;

		//! All newly created bodies will have their deactivaion period to this.
		unsigned int m_DeactivationPeriod;

		//! The squared minimum velocity for a moving body.
		float m_DeactivationVelocitySquared;
		//! The minimum velocity for a moving body.
		float m_DeactivationVelocity;

		//! The squared maximum velocity for a moving body.
		float m_MaxVelocitySquared;
		//! The maximum velocity of a moving body.
		float m_MaxVelocity;

		//! The resolution to use for bitmasks
		int m_BitmaskRes;

		private:
			void constrainBodies();
			//static void constrainBorders(void* ptr, void* data);
			//static void wrapAround(void* ptr, void* data);

			//void wrapBodiesAround();
	};

	//static void drawPolyShape(b2Shape *shape)
	//{
	//	b2Body *body = shape->GetBody();
	//	b2PolygonShape *polygon = (b2PolygonShape*)shape;

	//	int num = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();


	//	b2Vec2 vec = body->GetPosition() + (polygon->GetVertices())[0];
	//	int start_x = fe_lround(vec.x);
	//	int start_y = fe_lround(vec.y);
	//	int x, y;

	//	clBegin(CL_LINE_LOOP);
	//	for (unsigned int i = 1; i < unsigned int(num); i++) {

	//		vec = body->GetPosition() + (polygon->GetVertices())[i];
	//		x = fe_lround(vec.x);
	//		y = fe_lround(vec.y);

	//		clVertex2f(x, y);
	//	}
	//	clVertex2f(start_x, start_y);
	//	clEnd();

	//}

	//static void drawCircle(float32 x, float32 y, float32 r, float32 a)
	//{
	//	const int segs = 8;
	//	const float32 coef = 2.0*s_pi / (float32)segs;

	//	clBegin(CL_LINE_STRIP); {
	//		for(int n = 0; n <= segs; n++){
	//			float32 rads = n*coef;
	//			clVertex2f(r*cos(rads + a) + x, r*sin(rads + a) + y);
	//		}
	//		clVertex2f(x,y);
	//	} clEnd();
	//}

	//static void drawCircleShape(b2Shape *shape)
	//{
	//	b2Body *body = shape->GetBody();
	//	b2CircleShape *circle = (b2CircleShape *)shape;
	//	b2Vec2 c = body->GetWorldCenter() + circle->GetLocalPosition();

	//	drawCircle(c.x, c.y, circle->GetRadius(), body->GetAngle());
	//}

	//void drawObject(b2Shape *shape)
	//{
	//	b2Vec2 pos = shape->GetBody()->GetPosition();
	//	//CL_Display::draw_pixel(pos.x, pos.y, CL_Color::aliceblue);

	//	clColor3f(shape->GetRestitution(), shape->GetFriction(), 0.8f);
	//	switch (shape->GetType())
	//	{
	//	case b2ShapeType::e_polygonShape:
	//		drawPolyShape(shape);
	//	case b2ShapeType::e_circleShape:
	//		drawCircleShape(shape);
	//	}
	//}


	//void drawBodyPoint(b2Body *body)
	//{
	//	b2Vec2 v = body->GetWorldCenter();

	//	clColor3f(0.8f, 0.8f, 0.8f);
	//	clVertex2f(v.x, v.y);
	//}

}

#endif
