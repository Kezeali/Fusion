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

#include "FusionTransformInspector.h"

#include "FusionInspectorUtils.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/lexical_cast.hpp>

namespace FusionEngine { namespace Inspectors
{

	void TransformInspector::InitUI()
	{
		AddProperty("Position");
		AddTextInput("X",
			FloatSetter_t([](float value, ComponentIPtr<ITransform> component) { component->Position.Set(Vector2(value, component->Position.Get().y)); }),
			FloatGetter_t([](ComponentIPtr<ITransform> component)->float { return component->Position.Get().x; }),
			6
			);
		AddTextInput("Y",
			FloatSetter_t([](float value, ComponentIPtr<ITransform> component) { component->Position.Set(Vector2(component->Position.Get().x, value)); }),
			FloatGetter_t([](ComponentIPtr<ITransform> component)->float { return component->Position.Get().y; }),
			6
			);

		AddProperty("Angle");
		AddTextInput("Angle",
			FloatSetter_t([](float value, ComponentIPtr<ITransform> component) { component->Angle.Set(value); }),
			FloatGetter_t([](ComponentIPtr<ITransform> component)->float { return component->Angle.Get(); }),
			5
			);

		AddProperty("Depth");
		AddTextInput("Depth",
			IntSetter_t([](int value, ComponentIPtr<ITransform> component) { component->Depth.Set(value); }),
			IntGetter_t([](ComponentIPtr<ITransform> component)->int { return component->Depth.Get(); }),
			3
			);
	}

} }
