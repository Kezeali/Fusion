#ifndef Header_FusionEngine_FusionPhysicsWorld
#define Header_FusionEngine_FusionPhysicsWorld
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{
	//! Used for initialising bodies.
	struct PhysicalProperties
	{
		PhysicalProperties()
			: mass(0.0f),
			radius(0.0f),

			position(CL_Vector2::ZERO),

			rotation(0.0f)
			{
			}

		float mass;
		float radius;

		CL_Vector2 position;

		float rotation;
	};

	/*!
	 * \brief
	 * The controller for moving objects - this takes data (eg. velocity, force)
	 * from Bodies and moves them (the bodies do no movement themselves.)
	 *
	 * \see
	 * FusionPhysicsWorld | FusionPhysicsElipse.
	 */
	class FusionPhysicsWorld
	{
	public:
		//! Constructor.
		FusionPhysicsWorld();
		//! Virtual destructor.
		virtual ~FusionPhysicsWorld();

	public:
		//! List of PhysicsBodies
		typedef std::vector<FusionPhysicsBody *> PhysicsBodyList;

		//! Returned by _checkVectorForCollisions()
		struct CollisionPoint
		{
			bool point_found;
			CL_Vector2 collision_point;
		};

	public:
		//! Adds a body to the world so it will have physics applied to it.
		/*!
		 * This allow you to bypass the factory methods and add your crazy
		 * custom-class bodies.
		 *
		 * \param body Pointer to the body to add.
		 */
		void AddBody(FusionPhysicsBody *body);
		//! [depreciated] Removes the given body.
		/*!
		 * \param body Pointer to the body to remove.
		 */
		void RemoveBody(FusionPhysicsBody *body);

		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateBody(int type);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateBody(int type, const PhysicalProperties &props);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateBody(FusionPhysicsResponse *response, int type);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateBody(FusionPhysicsResponse *response, int type, const PhysicalProperties &props);
		//! Destroys the given body.
		/*!
		 * \param[in] body Pointer to the body to remove.
		 */
		void DestroyBody(FusionPhysicsBody *body);
		//! Removes all bodies.
		void Clear();

		//! Does movement and collision detection.
		/*!
		 * \param split Step magnitude (millis since last step.)
		 */
		void RunSimulation(unsigned int split);

		//! Resets the CollisonGrid and sets the borders up, etc.
		/*!
		 * Call this every time a new level is loaded.
		 *
		 * \remarks
		 * Remember to add a terrain body for the new level! (use AddBody())
		 */
		void LevelChange(int level_x, int level_y);

		//! Allows the maximum velocity to be set
		void SetMaxVelocity(float maxvel);
		//! Returns the current maximum velocity
		float GetMaxVelocity() const;

		//! Allows the bitmask resolution to be set (pixels per bit)
		void SetBitmaskRes(int ppb);
		//! Returns the res bodies should use for their bitmasks
		int GetBitmaskRes() const;

	public:
		//! Used internally by RunSimulation for its namesake.
		/*!
		 * \param one The object to check for collisions against.
		 * \param two The object which may be colliding against param one.
		 */
		bool _checkCollision(const FusionPhysicsBody *one, const FusionPhysicsBody *two);
		//! Used internally by RunSimulation to check for collisions on moving objects.
		/*!
		 * \param vector_one The movement vector for object one.
		 * \param vector_two The movement vector for object two.
		 * \param one The object to check for collisions against.
		 * \param two The object which may be colliding against param one.
		 */
		CollisionPoint _checkVectorForCollisions(const CL_Vector2 &vector_one, const CL_Vector2 &vector_two,
			 const FusionPhysicsBody *one, const FusionPhysicsBody *two) const;
		//! Gets the intersection of two vectors.
		/*!
		 * This returns a point of intersection assuming the vectors have infinate
		 * length (starting at pos_one and pos_two, and with the gradients given by
		 * vector_one and vector_two), so the point returned may not be within the
		 * bounds of the actual vectors. For this reason you should use 
		 * _checkBoundaries() to verify the returned point.
		 *
		 * \param pos_one The starting point of the first vector
		 * \param pos_two The starting point of the second vector
		 * \param vector_one The first vector
		 * \param vector_two The second vector
		 * \returns CL_Vector The point on the vectors where an intersection is found
		 */
		CL_Vector2 _getVectorIntersection(const CL_Vector2 &pos_one, const CL_Vector2 &pos_two, const CL_Vector2 &vector_one, const CL_Vector2 &vector_two) const;
		//! Checks whether the given point is within the given vectors.
		/*!
		 * \param[in] pos_one The starting point (offset) of the first vector
		 * \param[in] pos_two The starting point (offset) of the second vector
		 * \param[in] vec_one The first vector
		 * \param[in] vec_two The second vector
		 * \param[in] intersec The of the point to check
		 * \returns[bool] True if the point intersec is on the given lines
		 */
		bool FusionPhysicsWorld::_checkBoundaries(const CL_Vector2 &pos_one, const CL_Vector2 &pos_two, const CL_Vector2 &vec_one, const CL_Vector2 &vec_two, const CL_Vector2 &intersec) const;

	private:
		//! All physical objects controled by this world.
		PhysicsBodyList m_Bodies;

		//! Collision list.
		FusionPhysicsCollisionGrid *m_CollisionGrid;

		//! World dimensions
		int m_Width, m_Height;

		//! The squared maximum velocity for a moving body.
		float m_MaxVelocitySquared;
		//! The maximum velocity of a moving body.
		float m_MaxVelocity;

		//! The resolution to use for bitmasks
		int m_BitmaskRes;
	};

}

#endif
