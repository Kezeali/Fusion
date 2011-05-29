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

#ifndef H_FusionPhysicalComponent
#define H_FusionPhysicalComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"

#include "FusionVector2.h"
#include "FusionCommon.h"

namespace FusionEngine
{

	FSN_BEGIN_COIFACE(IPhysicalProperties)
	public:
		virtual const Vector2& GetPosition() const = 0;
		virtual void SetPosition(const Vector2& pos) = 0;

		virtual float GetAngle() const = 0;
		virtual void SetAngle(float angle) = 0;

		virtual const Vector2&  GetVelocity() const = 0;
		virtual void SetVelocity(const Vector2& vel) = 0;

		virtual float GetAngularVelocity() const = 0;
		virtual void SetAngularVelocity(float vel) = 0;

		static bool IsThreadSafe() { return true; }
	};

	FSN_BEGIN_COIFACE(IPhysicalMethods)
	public:
		virtual void ApplyForce(const Vector2& force, const Vector2& point) = 0;
		virtual void ApplyForce(const Vector2& force) = 0;
		virtual void ApplyTorque(float torque) = 0;

		static bool IsThreadSafe() { return false; }
	};

	FSN_BEGIN_COIFACE(IFixtureProperties)
	public:
	};

}

#endif
