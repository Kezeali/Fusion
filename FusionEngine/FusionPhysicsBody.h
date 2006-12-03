/*
  Copyright (c) 2006 FusionTeam

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

#include "FusionBitmask.h"
#include "FusionPhysicsCallback.h"

namespace FusionEngine
{
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
		FusionPhysicsBody(FusionPhysicsWorld *world, PhysicsFunctor *response);

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
		//! Gets the radius of this object
		virtual float GetRadius();

		//! Preferably this is used to move the body.
		virtual void ApplyForce(const CL_Vector2 &force);
		//! Sets the constant used to apply friction to the body.
		virtual void SetFrictionConstant(float constant);
		//! We don't care about yo' torque.
		virtual void SetRotationalVelocity(const float velocity);

		//@{
		//! Properties.
		virtual void SetColBitmask(const FusionBitmask &bitmask);
		virtual void SetColAABB(float width, float height);
		//virtual SetColAABB(const CL_Rectf &bbox);
		virtual void SetColDist(float dist);
		//@}

		//@{
		//! Property retreival.
		virtual FusionBitmask GetColBitmask() const;
		//! Returns true if the given point is solid
		/*!
		 * Quick access to the bitmask function FusionBitmask#GetBit()
		 */
		virtual bool GetColPoint(CL_Point point) const;
		virtual CL_Rectf GetColAABB() const;
		virtual float GetColDist() const;
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
		/**
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
		//! Calls the collision response (if this body has one.)
		void CollisionResponse(FusionPhysicsBody *other);
		//! Calls the collision response (if this body has one) with a reference point.
		void CollisionResponse(FusionPhysicsBody *other, const CL_Vector2 &collision_point);

		//@{
		//! State retreival.
		virtual const CL_Vector2 &GetPosition() const;
		virtual const CL_Point &GetPositionPoint() const;
		virtual const CL_Vector2 &GetAcceleration() const;
		virtual const CL_Vector2 &GetVelocity() const;

		virtual float GetRotationalVelocity() const;
		virtual float GetRotation() const;
		//@}

		//@{
		//! For syncronising client-side only, shouldn't be called otherwise.
		virtual void _setPosition(const CL_Vector2 &position);
		virtual void _setForce(const CL_Vector2 &force);
		virtual void _setAcceleration(const CL_Vector2 &acceleration);
		virtual void _setVelocity(const CL_Vector2 &velocity);

		virtual void _setRotation(float rotation);
		//@}

		//@{
		//! Used internally by CollisionGrid

		//! Stores the Collision Grid Pos; the position on the grid.
		void _setCGPos(int ind);
		//! Retreives the Collision Grid Pos - required for sorting etc.
		int _getCGPos() const;
		//! Stores the Collision Cell Index; the position within the cell.
		void _setCCIndex(int ind);
		//! Retreives the Collision Cell Index
		int _getCCIndex() const;
		//@}

	protected:
		//@{
		//! Used internally by CollisionGrid

		//! Collision Grid Index
		int m_CGPos;
		//! Collision Grid Index
		int m_CCIndex;
		//@}

		//! Containing world
		FusionPhysicsWorld *m_World;

		//! \see PhysicsCallback
		PhysicsFunctor *m_CollisionResponse;
		//! Data which may be useful for collision responses, etc.
		void *m_UserData;

		//! The ID for the current object's type
		/*!
		 * This is used to identify objects in collision responses
		 */
		int m_Type;

		//! bitmask
		FusionBitmask m_Bitmask;
		//! aabb
		CL_Rectf m_AABB;
		//! dist
		float m_ColDist;

		//! I don't think this is used...
		bool m_IsColliding;

		//! Bitmask collisions
		bool m_UsesPixel;
		//! Bounding Box collisions
		bool m_UsesAABB;
		//! Distance (circle) based collisions
		bool m_UsesDist;

		//@{
		//! "State" stuff.
		float m_Mass;
		float m_Radius;

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
