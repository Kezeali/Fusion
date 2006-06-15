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
	 * Holds data about a physical object.
	 * 
	 * \see
	 * FusionPhysicsBody
	 */
	class FusionPhysicsResponse
	{
	public:
		FusionPhysicsResponse();
		virtual ~FusionPhysicsResponse();

	public:
		//! Each type of body can have a derived class for different collison responses :D.
		virtual void CollisionResponse() {};

	};

}

#endif