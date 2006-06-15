#ifndef Header_FusionEngine_FusionPhysicsElipse
#define Header_FusionEngine_FusionPhysicsElipse

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionBody.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * An implimentation of PhysicsBody. Handles distance based collisions.
	 * 
	 * \sa
	 * FusionBody | FusionPhysicsWorld.
	 */
	class FusionPhysicsElipse : public FusionPhysicsBody
	{
	public:
		/*!
		 * \brief
		 * Constructor.
		 *
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsElipse(FusionPhysicsWorld *world);
		//! Virtual destructor.
		virtual ~FusionPhysicsElipse();

	public:
		//! Sets the distance from which a collision will be recorded.
		void SetRadius(float radius);

		//! Returns the distance from which a collision will be recorded.
		float GetRadius() const;

	private:
		float m_Radius;

	};

}

#endif