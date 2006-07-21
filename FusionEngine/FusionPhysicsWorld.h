#ifndef Header_FusionEngine_FusionPhysicsWorld
#define Header_FusionEngine_FusionPhysicsWorld
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{
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
		//! Constructor. Should only be called by FusionScene.
		/*!
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsWorld();
		//! Virtual destructor.
		virtual ~FusionPhysicsWorld();
		
	public:
		typedef std::vector<FusionPhysicsBody*> PhysicsBodyList;

		//!  Adds a body to the world so it will have physics applied to it.
		/*!
		 * \param body Pointer to the body to add.
		 */
		void AddBody(FusionPhysicsBody *body);
		//! Does movement and collision detection.
		/*!
		 * \param split Step magnitude (millis since last step.)
		 */
		void RunSimulation(unsigned int split);

	public:
		//! Used internally by RunSimulation for its namesake.
		/*!
		 * \param one The object to check for collisions against.
		 * \param two The object which may be colliding against param one.
		 */
		bool _CheckCollision(const FusionPhysicsBody &one, const FusionPhysicsBody &two);
		//! Used internally by RunSimulation to check for collisions on moving objects.
		/*!
		 * \param vector The vector along which to check.
		 * \param one The object to check for collisions against.
		 * \param two The object which may be colliding against param one.
		 */
		bool _CheckVectorForCollisions(const CL_Vector2 &vector,
			const FusionPhsicsBody &one, const FusionPhysicsBody &two);

	private:
		//! All physical objects controled by this world.
		PhysicsBodyList m_Bodies;
		
	};

}

#endif