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

			position(Vector2::ZERO),
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
	class PhysicsWorld// : public RefCounted<PhysicsWorld, "PhysicsWorld">
	{
	public:
		//! Constructor.
		PhysicsWorld();
		//! Virtual destructor.
		virtual ~PhysicsWorld();

	public:
		//! List of Collsions
		//typedef std::list<Collision *> CollisionList;
		//typedef std::vector<PhysicsBody *> BodyList;

	public:
		//! [depreciated] Adds a body to the world so it will have physics applied to it.
		/*!
		 * This is retainded not only for backwards compatibility, but also to allow you 
		 * to bypass the factory methods and add your crazy custom-class bodies.
		 *
		 * \param body Pointer to the body to add.
		 */
		void AddBody(PhysicsBody *body);
		//! [depreciated] Removes the given body.
		/*!
		 * \param body Pointer to the body to remove.
		 */
		void RemoveBody(PhysicsBody *body);

		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateBody(int type);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateBody(int type, const PhysicalProperties &props);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateBody(ICollisionHandler *response, int type);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateBody(ICollisionHandler *response, int type, const PhysicalProperties &props);
		//! Destroys the given body.
		/*!
		 * \param[in] body Pointer to the body to remove.
		 */
		void DestroyBody(PhysicsBody *body);

		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateStatic(int type);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateStatic(int type, const PhysicalProperties &props);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateStatic(ICollisionHandler* response, int type);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		PhysicsBody *CreateStatic(ICollisionHandler* response, int type, const PhysicalProperties &props);
		//! Destroys the given static body.
		/*!
		 * \param[in] body Pointer to the body to remove.
		 */
		void DestroyStatic(PhysicsBody *body);

		void AddShape(Shape* shape);

		void AddStaticShape(Shape* shape);

		void RemoveShape(Shape* shape);

		void RemoveStaticShape(Shape* shape);

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
		 */
		void RunSimulation(unsigned int split);

		//! Draws all shapes in the simulation
		void DebugDraw(bool fast = true);

		//! Resets the CollisonGrid and sets the borders up, etc.
		/*!
		 * Call this every time a new level is loaded.
		 *
		 * \remarks
		 * Remember to add a terrain body for the new level! (use AddBody())
		 */
		void Initialise(int level_x, int level_y);

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

		cpSpace* GetChipSpace() const
		{
			return m_ChipSpace;
		}

	private:
		// Chipmunk space
		cpSpace* m_ChipSpace;

		//! All physical objects controled by this world.
		BodyList m_Bodies;
		// Bodies to be deleted after the simulation is complete
		BodyList m_DeleteQueue;

		bool m_RunningSimulation;

		//! Body hash map / query interface
		//CollisionGrid *m_CollisionGrid;

		//! Static objects are listed here.
		/*!
		 * These are allways collision checked against every other object - this
		 * shouldn't create a preformance issue, as generally the only static
		 * object is the terrain.
		 */
		BodyList m_Statics;

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
			static void constrainBorders(void* ptr, void* data);
			static void wrapAround(void* ptr, void* data);
	};

	static void drawPolyShape(cpShape *shape)
	{
		cpBody *body = shape->body;
		cpPolyShape *poly = (cpPolyShape *)shape;

		int num = poly->numVerts;
		cpVect *verts = poly->verts;

		glBegin(GL_LINE_LOOP);
		for(int i=0; i<num; i++){
			cpVect v = cpvadd(body->p, cpvrotate(verts[i], body->rot));
			glVertex2f(v.x, v.y);
		} glEnd();
	}

	static void drawCircle(cpFloat x, cpFloat y, cpFloat r, cpFloat a)
	{
		const int segs = 8;
		const cpFloat coef = 2.0*M_PI/(cpFloat)segs;

		glBegin(GL_LINE_STRIP); {
			for(int n = 0; n <= segs; n++){
				cpFloat rads = n*coef;
				glVertex2f(r*cos(rads + a) + x, r*sin(rads + a) + y);
			}
			glVertex2f(x,y);
		} glEnd();
	}

	static void drawCircleShape(cpShape *shape)
	{
		cpBody *body = shape->body;
		cpCircleShape *circle = (cpCircleShape *)shape;
		cpVect c = cpvadd(body->p, cpvrotate(circle->c, body->rot));

		drawCircle(c.x, c.y, circle->r, body->a);
	}

	static void drawObject(void *ptr, void *unused)
	{
		cpShape *shape = (cpShape *)ptr;
		CL_Display::draw_pixel(shape->body->p.x, shape->body->p.y, CL_Color::aliceblue);

		clColor3f(shape->e, shape->u, shape->id*0.0001);
		switch (shape->type)
		{
		case CP_POLY_SHAPE:
			drawPolyShape(shape);
		case CP_CIRCLE_SHAPE:
			drawCircleShape(shape);
		}
	}


	static void drawBodyPoint(void *ptr, void *unused)
	{
		cpShape *shape = (cpShape *)ptr;
		cpCircleShape* circle = (cpCircleShape*)shape;
		cpVect v = cpvadd(circle->c, shape->body->p);

		clColor3f(shape->e, shape->u, shape->id*0.0001);
		clVertex2f(v.x, v.y);
	}

}

#endif
