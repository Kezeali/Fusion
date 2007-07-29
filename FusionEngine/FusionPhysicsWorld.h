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

#ifndef Header_FusionEngine_FusionPhysicsWorld
#define Header_FusionEngine_FusionPhysicsWorld

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{

	static const int g_PhysGridCellW = 128;
	static const int g_PhysGridCellH = 128;
	static const float g_PhysCollisionJump = 0.25f;

	//! Represents a collision
	struct Collision
	{
		//! Basic Constructor.
		Collision() {};

		//! Basic Constructor.
		Collision(
			const Vector2 &normal,
			FusionPhysicsBody *first, FusionPhysicsBody *second,
			const Vector2 &first_pac, const Vector2 &second_pac)
			: Normal(normal),
			First(first),
			Second(second),
			First_Position(first_pac),
			Second_Position(second_pac)
		{}

		//! Collision normal (for First)
		Vector2 Normal;

		//! First object in the collision
		FusionPhysicsBody *First;
		//! Second object in the collision
		FusionPhysicsBody *Second;

		//! Point of collision
		Vector2 Contact;

		//! Position of the first object during the collision
		Vector2 First_Position;
		//! Position of the second object during the collision
		Vector2 Second_Position;

		//friend bool operator< (const Collision &lhs, const Collision &rhs);
		//friend bool operator== (const Collision &lhs, const Collision &rhs);

	};


	//bool operator< (const Collision &lhs, const Collision &rhs)
	//{
	//	if (&lhs.First < &rhs.Second && &lhs.Second < &rhs.First)
	//	{
	//		if (&lhs.First < &rhs.First && &lhs.Second < &rhs.Second)
	//		{
	//			return true;
	//		}
	//	}

	//	return false;
	//}

	//bool operator== (const Collision &lhs, const Collision &rhs)
	//{
	//	// Both collisions are exactly the same
	//	if (&lhs.First < &rhs.First && &lhs.Second < &rhs.Second)
	//	{
	//		return true;
	//	}

	//	// Both collisions are essentually the same, with the first and second inverted
	//	if (&lhs.First == &rhs.Second && &lhs.Second == &rhs.First)
	//	{
	//		return true;
	//	}

	//	return false;
	//}

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
			bounce(0.0f),

			position(Vector2::ZERO),
			rotation(0.0f),

			use_bitmask(false),
			use_aabb(false),
			use_dist(false),
			bitmask(0),
			dist(0)
			{}

		//! Mass.
		float mass;
		//! Radius. Not used
		float radius;
		//! Coefficiant of restitution
		float bounce;

		//! Initial Position.
		Vector2 position;
		//! Initial Rotation.
		float rotation;

		//! Use Pixel based collision detection.
		bool use_bitmask;
		//! Use the given bitmask.
		FusionBitmask *bitmask;
		//! Use AABB based collision detection.
		bool use_aabb;
		//! Make an AABB with the given (upright) dimensions
		float aabb_x, aabb_y;
		//! Use BB based collision detection.
		bool use_bb;
		//! Make a BB with the given dimensions
		float bb_x, bb_y;
		//! Use Distance based collision detection.
		bool use_dist;
		//! Use the given distance (for distance based collisions.)
		float dist;

	};

	/*!
	 * \brief
	 * The controller for moving objects - this takes data (eg. velocity, force)
	 * from Bodies and moves them (the bodies do no movement themselves.)
	 *
	 * It is recomended that you use the factory methods (CreateBody(...), 
	 * DestroyBody(...)) rather than the 
	 *
	 * \todo Activation / deactivation for objects which aren't moving.
	 *
	 * Objects should notify the world on property changes (_setPosition, etc)
	 * so the world can decide whether to activate them, and if a body doesn't
	 * notify over a long period of time, it gets de-activated (checks can be done
	 * at the end of RunSimulation.)
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
		//! List of Collsions
		typedef std::list<Collision *> CollisionList;
		//typedef std::vector<FusionPhysicsBody *> BodyList;

	public:
		//! [depreciated] Adds a body to the world so it will have physics applied to it.
		/*!
		 * This is retainded not only for backwards compatibility, but also to allow you 
		 * to bypass the factory methods and add your crazy custom-class bodies.
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
		FusionPhysicsBody *CreateBody(CollisionHandler *response, int type);
		//! Creates a body in the world so it will have physics applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateBody(CollisionHandler *response, int type, const PhysicalProperties &props);
		//! Destroys the given body.
		/*!
		 * \param[in] body Pointer to the body to remove.
		 */
		void DestroyBody(FusionPhysicsBody *body);

		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateStatic(int type);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateStatic(int type, const PhysicalProperties &props);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateStatic(CollisionHandler* response, int type);
		//! Creates a static body in the world so it will have collision detection applied to it.
		/*!
		 * \param[in] response A pointer to the response object.
		 * \param[in] type The type of body.
		 * \param[in] props Properties to initialise the body with.
		 * \returns A const pointer to the body created.
		 */
		FusionPhysicsBody *CreateStatic(CollisionHandler* response, int type, const PhysicalProperties &props);
		//! Destroys the given static body.
		/*!
		 * \param[in] body Pointer to the body to remove.
		 */
		void DestroyStatic(FusionPhysicsBody *body);

		//! Removes all bodies and statics.
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
		void Initialise(int level_x, int level_y);

		//! Activates wrap around
		void ActivateWrapAround();

		//! Deactivates wrap around
		void DeactivateWrapAround();

		//! Deactivates wrap around
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

		//! Allows the bitmask scale to be set (pixels per bit)
		void SetBitmaskRes(int ppb);
		//! Returns the scale bodies should use for their bitmasks
		int GetBitmaskRes() const;

		const FusionPhysicsCollisionGrid* GetCollisionGrid() const;

	private:
		Console m_Console;
		//! All physical objects controled by this world.
		BodyList m_Bodies;

		//! Collision list.
		FusionPhysicsCollisionGrid *m_CollisionGrid;

		//! Static objects are listed here.
		/*!
		 * These are allways collision checked against every other object - this
		 * shouldn't create a preformance issue, as generally the only static
		 * object is the terrain.
		 */
		BodyList m_Static;

		//! World dimensions
		int m_Width, m_Height;

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
	};

}

#endif
