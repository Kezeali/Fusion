/*
*  Copyright (c) 2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionTransformComponent
#define H_FusionTransformComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionThreadSafeProperty.h"

#include "FusionVector2.h"
#include "FusionVectorTypes.h"

#include <tbb/spin_mutex.h>

namespace FusionEngine
{
	
	FSN_BEGIN_COIFACE(ITransform)
	public:
		void InitProperties()
		{
			Position.SetCallbacks(this, &ITransform::GetPosition, &ITransform::SetPosition);
			Angle.SetCallbacks(this, &ITransform::GetAngle, &ITransform::SetAngle);
			Depth.SetCallbacks(this, &ITransform::GetDepth, &ITransform::SetDepth);
		}

		ThreadSafeProperty<Vector2> Position;
		ThreadSafeProperty<float> Angle;
		ThreadSafeProperty<int> Depth;

		static void RegisterScriptInterface(asIScriptEngine* engine);

		static bool IsThreadSafe() { return true; }

		virtual bool HasContinuousPosition() const = 0;

		virtual Vector2 GetPosition() const = 0;
		virtual void SetPosition(const Vector2& pos) = 0;

	protected:

		virtual float GetAngle() const = 0;
		virtual void SetAngle(float angle) = 0;

		virtual int GetDepth() const = 0;
		virtual void SetDepth(int depth) = 0;
	};

}

#endif
