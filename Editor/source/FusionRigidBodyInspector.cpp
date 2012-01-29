/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionRigidBodyInspector.h"

namespace FusionEngine { namespace Inspectors
{

	inline CL_Origin StringToOrigin(const std::string str)
	{
		if (str == "Top-Left")
			return origin_top_left;
		else if (str == "Top-Center")
			return origin_top_center;
		else if (str == "Top-Right")
			return origin_top_right;

		else if (str == "Center-Left")
			return origin_center_left;
		else if (str == "Center")
			return origin_center;
		else if (str == "Center-Right")
			return origin_center_right;

		else if (str == "Bottom-Left")
			return origin_bottom_left;
		else if (str == "Bottom-Center")
			return origin_bottom_center;
		else if (str == "Bottom-Right")
			return origin_bottom_right;

		else
			return origin_center;
	}

	inline std::string OriginToString(const CL_Origin origin)
	{
		switch (origin)
		{
		case origin_top_left: return "Top-Left";
		case origin_top_center: return "Top-Center";
		case origin_top_right: return "Top-Right";

		case origin_center_left: return "Center-Left";
		case origin_center: return "Center";
		case origin_center_right: return "Center-Right";

		case origin_bottom_left: return "Bottom-Left";
		case origin_bottom_center: return "Bottom-Center";
		case origin_bottom_right: return "Bottom-Right";
		default: return "Center";
		};
	}

	void RigidBodyInspector::InitUI()
	{
		AddToggleInput("Interpolate",
			BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->Interpolate.Set(value); }),
			BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->Interpolate.Get(); })
			);

		AddTextInput("LinearDamping",
			FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->LinearDamping.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->LinearDamping.Get(); })
			);
		AddTextInput("AngularDamping",
			FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->AngularDamping.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->AngularDamping.Get(); })
			);

		AddRangeInput("GravityScale",
			0.f, 1.f,
			FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->GravityScale.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->GravityScale.Get(); })
			);
		
		AddToggleInput("SleepingAllowed",
			BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->SleepingAllowed.Set(value); }),
			BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->SleepingAllowed.Get(); })
			);
		
		AddToggleInput("Bullet",
			BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->Bullet.Set(value); }),
			BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->Bullet.Get(); })
			);
		
		AddToggleInput("FixedRotation",
			BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->FixedRotation.Set(value); }),
			BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->FixedRotation.Get(); })
			);
	}

} }
