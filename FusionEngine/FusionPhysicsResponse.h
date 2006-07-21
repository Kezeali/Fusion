#ifndef Header_FusionEngine_FusionPhysicsResponse
#define Header_FusionEngine_FusionPhysicsResponse
#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Each body can have a derived class for different collison responses :D.
	 * 
	 * \remarks
	 * MCS - This is probably one of my best ideas on the physics front.
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
		virtual void CollisionResponse() {};

	};

}

#endif