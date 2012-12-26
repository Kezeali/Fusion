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

#include "FusionFixtureInspector.h"

namespace FusionEngine { namespace Inspectors
{

	void FixtureInspector::InitUI()
	{
		AddProperty("Sensor", AddToggleInput("Sensor",
			BoolSetter_t([](bool value, ComponentIPtr<IFixture> component) { component->Sensor.Set(value); }),
			BoolGetter_t([](ComponentIPtr<IFixture> component)->bool { return component->Sensor.Get(); })
			));

		AddProperty("Density", AddTextInput("Density",
			FloatSetter_t([](float value, ComponentIPtr<IFixture> component) { component->Density.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IFixture> component)->float { return component->Density.Get(); })
			));

		AddProperty("Friction", AddTextInput("Friction",
			FloatSetter_t([](float value, ComponentIPtr<IFixture> component) { component->Friction.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IFixture> component)->float { return component->Friction.Get(); })
			));

		AddProperty("Restitution", AddTextInput("Restitution",
			FloatSetter_t([](float value, ComponentIPtr<IFixture> component) { component->Restitution.Set(value); }),
			FloatGetter_t([](ComponentIPtr<IFixture> component)->float { return component->Restitution.Get(); })
			));
	}

} }
