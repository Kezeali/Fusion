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
*/

#ifndef Header_FusionEngine_FusionPhysicsBody
#define Header_FusionEngine_FusionPhysicsBody

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionBitmask.h"
#include "FusionPhysicsCallback.h"

namespace FusionEngine
{
	//! Collision type flags
	/*!
	 * Flags used by FusionPhysicsWorld to decide what type of collision
	 * detection/response to use.
	 */
	enum CollisionFlags
	{
		//! (0000)
		C_NONE = 0,
		//! (0001)
		C_STATIC = 1,
		//! (0010)
		C_BOUNCE = 2
	};

	/*!
	 * \brief
	 * The basis for movable/colliding objects.
	 *
	 * \remarks
	 * With regurad to the state stored in this class:
	 * For FusionShips this should be initialisd by the ShipFactory when it creates
	 * a new ship. It should remain indipendant of the ClientEnvironment after that
	 * point - all modification to it can be done manually, rather than requiring it
	 * to know of ShipResource.
	 * <br>
	 * MCS - Just one other key thing to remember, FusionPhysicsBody is brainless!
	 * This class just stores data, and keeps that data valid (i.e. modify the AABB
	 * to fit the bitmask if it rotates.)
	 * <br>
	 * MCS - AABBs are not yet implimented
	 * <br>
	 * MCS - perhaps this should be abstract an class. 
	 *
	 * \todo AABB for FusionPhysicsBody
	 *
	 * \todo Perhaps bodies should have ApplyPosition and ApplyRotation methods, rather
	 * than giving FusionPhysicsWorld friend access...
	 *
	 * \see
	 * FusionPhysicsWorld | FusionFhysicsElipse.
	 */
	class FusionPhysicsBody
	{
		friend class FusionPhysicsWorld;
	public:
		//FusionPhysicsBody();
		//! Constructor.
		/*!
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsBody(FusionPhysicsWorld *world);
		//! Constructor with response param.
		/*!
		 * \param world
		 * The world in which this body resides.
		 * \param response
		 * The response function to call on upon a collision.
		 */
		FusionPhysicsBody(FusionPhysicsWorld *world, CollisionCallback response);

	public:
		//! Sets the type ID for this object.
		void SetType(int type);
		//! Does what you think it does.
		virtual void SetMass(float mass);
		//! Sets the radius used for torque equations.
		/*
		 * There are no torque equations ATM, but may be in the future...
		 */
		virtual void SetRadius(float radius);

		//! Gets the type of this object
		int GetType();
		//! Gets the mass of this object
		virtual float GetMass();
		//! Gets one over mass
		virtual float GetInverseMass();
		//! Gets the radius of this object
		virtual float GetRadius();

		//! Preferably this is used to move the body.
		virtual void ApplyForce(const CL_Vector2 &force);
		//! Preferably this is used to move the body.
		/*!
		 * Applies force based on the current orientation.
		 */
		virtual void ApplyForce(float force);
		//! Sets the constant used to apply damping to the body's movement.
		virtual void SetCoefficientOfFriction(float damping);
		//! Sets the constant used to apply bounce to the body's collisions.
		virtual void SetCoefficientOfRestitution(float bounce);
		//! We don't care about yo' torque.
		virtual void SetRotationalVelocity(float velocity);

		//@{
		//! Properties.
		virtual void SetColBitmask(FusionBitmask *bitmask);
		virtual void SetColAABB(float width, float height);
		//virtual SetColAABB(const CL_Rectf &bbox);
		virtual void SetColDist(float dist);
		//@}

		//@{
		//! Property retreival.

		virtual FusionBitmask *GetColBitmask() const;
		virtual CL_Rectf GetColAABB() const;
		virtual float GetColDist() const;

		//! Returns true if the given point is solid.
		/*!
		 * <p>
		 * Quick access to the bitmask function FusionBitmask#GetBit()
		 * </p>
		 * If 'auto_offset' is true, the function will automatically offset the given
		 * 'point' relative to this body's current position, and scale it to match
		 * the the bitmask's PPB setting. <br>
		 * This is oftern required, because GetBit isn't designed to act as a collision
		 * detection function.
		 *
		 * \param point
		 * The point on the bitmask to check, starting at (0,0).
		 * \param auto_offset
		 * Automatically offset the 'point' param.
		 */
		virtual bool GetColPoint(const CL_Point &point, bool auto_offset = true) const;
		//@}

		//@{
		//! Collision type properties.
		/*!
		 * I think these are self explanatory.
		 */

		virtual void SetUsePixelCollisions(bool usePixel);
		virtual void SetUseAABBCollisions(bool useAABB);
		virtual void SetUseDistCollisions(bool useDist);
		//@}

		//@{
		/*!
		 * Collision type property retrieval.
		 * I think these are self explanatory.
		 */
		virtual bool GetUsePixelCollisions() const;
		virtual bool GetUseAABBCollisions() const;
		virtual bool GetUseDistCollisions() const;
		//@}

		//! Sets the user data
		/*!
		 * Usually a pointer to the game object this physical entity is associated with.
		 */
		void SetUserData(void *userdata);
		//! Retrives user data
		const void *GetUserData() const;

		//! Sets the collision response callback
		void SetCollisionCallback(const CollisionCallback &method);

		//! Calls the collision response (if this body has one.)
		void CollisionResponse(FusionPhysicsBody *other);
		//! Calls the collision response (if this body has one) with a reference point.
		void CollisionResponse(FusionPhysicsBody *other, const CL_Vector2 &collision_point);

		//! Returns the current collision config.
		int GetCollisionFlags() const;

		//! Returns true if the given flag is set.
		bool CheckCollisionFlag(int flag);

		//! Allows the collision flags to be set manually
		/*!
		 * This isn't usually necessary, as collision flags get set by relavant
		 * methods (e.g. If SetMass(0) is called, the C_STATIC flag will be set.)
		 */
		void _setCollisionFlags(int flags);

		//@{
		//! State retreival.
		virtual const CL_Vector2 &GetPosition() const;
		//! Integer point used as that makes this eaisier to pass to FusionBitmask.
		virtual CL_Point GetPositionPoint() const;
		//! Gets the net force applied to the body.
		virtual const CL_Vector2 &GetForce() const;
		virtual const CL_Vector2 &GetAcceleration() const;
		virtual const CL_Vector2 &GetVelocity() const;

		//! Guess
		virtual float GetCoefficientOfFriction() const;
		//! Yep
		virtual float GetCoefficientOfRestitution() const;

		virtual float GetRotationalVelocity() const;
		virtual float GetRotation() const;
		//@}

		//! Returns true if this object is active.
		/*!
		 * Objects which haven't had calls to any property modifying methods in a while
		 * will become in-active, and will be ignored during FusionPhysicsWorld#RunSimulation()
		 */
		bool IsActive() const;

		//! Sets m_Active to true.
		/*!
		 * m_Active will be set to true, and m_DeactivationCounter will be set to
		 * m_DeactivationPeriod. 
		 * <br>
		 * Allows FusionPhysicsWorld to activate bodies.
		 */
		void _activate();

		//! Sets m_Active to false.
		/*!
		 * Allows FusionPhysicsWorld to deactivate bodies.
		 */
		void _deactivate();

		//! Sets m_Active to false when m_DeactivationCounter reaches zero.
		/*!
		 * Allows FusionPhysicsWorld to deactivate bodies after a period.
		 *
		 * \param[in] split
		 * The amount of time since the last step (i.e. the amount to decrease the
		 * deactivation counter by.)
		 */
		void _deactivateAfterCountdown(unsigned int split);

		//! Sets the value of m_DeactivationCounter.
		/*!
		 * Generally, _deactivateAfterCountdown() should be used rather than this.
		 */
		void _setDeactivationCount(unsigned int count);

		//! Returns the value of m_DeactivationCounter.
		/*!
		 * Returns the ammount of time left before this body deactivates.
		 */
		unsigned int GetDeactivationCount() const;

		//! Sets m_DeactivationPeriod.
		/*!
		 * Sets the deactivation period (the period used to set the deactivation time.)
		 *
		 * \param[in] period
		 * The value to set to.
		 */
		void SetDeactivationPeriod(unsigned int period);

		//! Gets m_DeactivationPeriod.
		/*!
		 * Returns the period used to set the deactivation counter.
		 */
		unsigned int GetDeactivationPeriod() const;

		//@{
		//! For syncronising client-side only, shouldn't be called otherwise.

		//! Sets the position.
		virtual void _setPosition(const CL_Vector2 &position);
		//! Sets the force.
		virtual void _setForce(const CL_Vector2 &force);
		//! Sets the acceleration.
		virtual void _setAcceleration(const CL_Vector2 &acceleration);
		//! Sets the velocity.
		virtual void _setVelocity(const CL_Vector2 &velocity);

		//! Sets the rotation.
		virtual void _setRotation(float rotation);
		//@}

		//! Adds the given body to the collision list
		void _notifyCollisionWith(FusionPhysicsBody *other);
		//! Checks for the given body on the collision list
		bool IsCollidingWith(FusionPhysicsBody *other) const;
		//! Clears the collision list
		void ClearCollisions();

		//@{
		//! Used internally by CollisionGrid

		//! Stores the Collision Grid Pos; the position on the grid.
		void _setCGPos(int ind);
		//! Retreives the Collision Grid Pos - required for sorting etc.
		int _getCGPos() const;
		//! [depreciated] Stores the Collision Cell Index; the position within the cell.
		void _setCCIndex(int ind);
		//! Retreives the Collision Cell Index
		int _getCCIndex() const;

		//! Sets m_GotCGUpdate to true.
		/*!
		 * Allows the collision grid to tell this body that it's update request has been noted.
		 */
		void _notifyCGwillUpdate();
		//! Returns m_GotCGUpdate.
		/*!
		 * Allows the collision grid to see if this body has already requested an update.
		 */
		bool _CGwillUpdate() const;
		//! Sets m_GotCGUpdate to false.
		/*!
		 * Allows the collision grid to tell this body that it's previously requested 
		 * update has been completed.
		 */
		void _notifyCGUpdated();
		//@}

	protected:
		//@{
		//! Used internally by CollisionGrid

		//! Collision Grid Index
		int m_CGPos;
		//! Collision Grid Index
		int m_CCIndex;

		//! True if FusionPhysicsCollisionGrid#_updateThis(thisobject) has been called.
		bool m_GotCGUpdate;
		//@}

		//! Containing world
		FusionPhysicsWorld *m_World;

		//! \see FusionPhysicsCallback.h
		CollisionCallback m_CollisionResponse;
		//! Data which may be useful for collision responses, etc.
		void *m_UserData;
		//! Collision flags, such as C_STATIC
		int m_CollisionFlags;

		//! The ID for the current object's type
		/*!
		 * This is used to identify objects in collision responses
		 */
		int m_Type;

		//! True when the body is active.
		bool m_Active;

		//! The time left before this body deactivates, if nothing happens to it.
		int m_DeactivationCounter;

		//! The period used to set m_DeactivationCounter.
		unsigned int m_DeactivationPeriod;

		//! bitmask
		FusionBitmask *m_Bitmask;
		//! aabb
		CL_Rectf m_AABB;
		//! dist
		float m_ColDist;

		//! Lists the bodies which have signaled that they are colliding with this one
		BodyList m_CollidingBodies;

		//! Bitmask collisions
		bool m_UsesPixel;
		//! Bounding Box collisions
		bool m_UsesAABB;
		//! Distance (circle) based collisions
		bool m_UsesDist;

		//@{
		//! "State" stuff.
		float m_Mass;
		float m_InverseMass;
		float m_Radius;
		//! Linear damping, a.k.a. coefficient of friction
		float m_LinearDamping;
		//! Bounce, a.k.a. coefficient of restitution
		float m_Bounce;

		CL_Vector2 m_AppliedForce;
		CL_Vector2 m_Acceleration;
		CL_Vector2 m_Velocity;
		CL_Vector2 m_Position;

		float m_Rotation;
		//! Current velocity of rotation.
		/*!
		 * \remarks
		 * (ShipResource has RotationVelocity [no 'al'], that being the <i>maximum</i>
		 * velocity of rotation.)
		 */
		float m_RotationalVelocity;
		//@}

	};

}

#endif
