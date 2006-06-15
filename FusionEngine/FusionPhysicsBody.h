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

		virtual void SetPosition(const CL_Vector2 &position) = 0;

	protected:
		FusionPhysicsWorld *m_World;
		FusionPhysicsProperties m_Properties;

		bool m_IsColliding;

	};

}

#endif