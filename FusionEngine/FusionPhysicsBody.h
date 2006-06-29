#ifndef Header_FusionEngine_FusionPhysicsBody
#define Header_FusionEngine_FusionPhysicsBody
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Abstract class, the basis for movable/colliding objects.
	 *
	 * \remarks
	 * With regurad to the state stored in this class:
	 * For FusionShips this should be initialisd by the ShipFactory when it creates
	 * a new ship. It should remain indipendant of the ClientEnvironment after that
	 * point - all modification to it can be done manually, rather than requiring it
	 * to know of ShipResource.
	 * 
	 * \see
	 * FusionPhysicsWorld | FusionFhysicsElipse.
	 */
	class FusionPhysicsBody
	{
	public:
		/*!
		 * \brief
		 * Constructor.
		 *
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsBody(FusionPhysicsWorld *world);
		//! Virtual destructor.
		virtual ~FusionPhysicsBody();

	public:
		//! Does what you think it does.
		void SetMass(float mass);

		//! Preferably this is used to move the body.
		virtual void ApplyForce(const CL_Vector2 &force);
		//! We don't care about yo torque.
		virtual SetRotationVelocity(const float velocity);
		
		//@{
		/*! Collision type properties.
		 * I think these are self explanatory.
		 */
		void SetUsePixelCollisions(bool usePixel);
		void SetUseBBCollisions(bool useBB);
		void SetUseDistCollisions(bool useDist);
		//@}
		
		//@{
		/*! Collision type property retrieval.
		 * I think these are self explanatory.
		 */
		void GetUsePixelCollisions(bool usePixel);
		void GetUseBBCollisions(bool useBB);
		void GetUseDistCollisions(bool useDist);
		//@}

		//@{
		//! State retreival.
		virtual CL_Vector2 &GetPosition() const;
		virtual CL_Vector2 &GetAcceleration() const;
		virtual CL_Vector2 &GetVelocity() const;

		virtual float GetRotationVelocity() const;
		virtual float GetRotation() const;
		//@}

		//@{
		//! For syncronising client-side only, shouldn't be called otherwise.
		virtual void _setAcceleration(const CL_Vector2 &acceleration);
		virtual void _setVelocity(const CL_Vector2 &velocity);
		virtual void _setPosition(const CL_Vector2 &position);

		virtual void _setRotation(const float rotation);
		//@}

	protected:
		FusionPhysicsWorld *m_World;
		FusionPhysicsProperties *m_Properties;

		bool m_IsColliding;
		
		//! Bitmask collisions
		bool m_UsesPixel;
		//! Bounding Box collisions
		bool m_UsesBB;
		//! Distance (circle) based collisions
		bool m_UsesDist;

		//@{
		//! "State" stuff.
		float m_Mass;

		CL_Vector2 m_AppliedForce;
		Cl_Vector2 m_Acceleration;
		Cl_Vector2 m_Velocity;
		Cl_Vector2 m_Position;

		float m_Rotation;
		float m_RotationVelocity;
		//@}

	};

}

#endif