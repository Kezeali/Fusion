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

#include "FusionInspectorTypeUtils.h"

namespace FusionEngine { namespace Inspectors
{

	void RigidBodyInspector::InitUI()
	{
		//AddProperty("Interpolate", AddToggleInput("Interpolate",
		//	BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->Interpolate.Set(value); }),
		//	BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->Interpolate.Get(); })
		//	));

		//AddProperty("LinearDamping", AddTextInput("LinearDamping",
		//	FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->LinearDamping.Set(value); }),
		//	FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->LinearDamping.Get(); })
		//	));
		//AddProperty("AngularDamping", AddTextInput("AngularDamping",
		//	FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->AngularDamping.Set(value); }),
		//	FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->AngularDamping.Get(); })
		//	));

		//AddProperty("GravityScale", AddTextInput("GravityScale",
		//	FloatSetter_t([](float value, ComponentIPtr<IRigidBody> component) { component->GravityScale.Set(value); }),
		//	FloatGetter_t([](ComponentIPtr<IRigidBody> component)->float { return component->GravityScale.Get(); })
		//	));
		//
		//AddProperty("SleepingAllowed", AddToggleInput("SleepingAllowed",
		//	BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->SleepingAllowed.Set(value); }),
		//	BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->SleepingAllowed.Get(); })
		//	));
		//
		//AddProperty("Bullet", AddToggleInput("Bullet",
		//	BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->Bullet.Set(value); }),
		//	BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->Bullet.Get(); })
		//	));
		//
		//AddProperty("FixedRotation", AddToggleInput("FixedRotation",
		//	BoolSetter_t([](bool value, ComponentIPtr<IRigidBody> component) { component->FixedRotation.Set(value); }),
		//	BoolGetter_t([](ComponentIPtr<IRigidBody> component)->bool { return component->FixedRotation.Get(); })
		//	));
	}

} }
