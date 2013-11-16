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

#include "FusionCircleShapeInspector.h"

#include <Gwen/Controls/Property/Checkbox.h>
#include <Gwen/Controls/Property/Text.h>

#include <boost/lexical_cast.hpp>

using namespace Gwen;

namespace FusionEngine { namespace Inspectors
{

	void CircleShapeInspector::InitUI()
	{
		//auto row = props->Add(L"X", new Controls::Property::Checkbox( props ));
		//row->onChange.Add(this, [](Controls::Base* control)
		//{
		//	Controls::PropertyRow* row = (Controls::PropertyRow*)control;
		//	Controls::Property::Checkbox* checkbox = dynamic_cast<Controls::Property::Checkbox*>(row->GetProperty());
		//	checkbox->m_Checkbox->IsChecked()
		//});
		/*
		AddProperty("Position", AddTextInput("Y",
			FloatSetter_t([](float value, ComponentIPtr<ICircleShape> component) { component->Position.Set(Vector2(value, component->Position.Get().y)); }),
			FloatGetter_t([](ComponentIPtr<ICircleShape> component)->float { return component->Position.Get().y; })
			));
		AddProperty("Position", AddTextInput("X",
			FloatSetter_t([](float value, ComponentIPtr<ICircleShape> component) { component->Position.Set(Vector2(component->Position.Get().x, value)); }),
			FloatGetter_t([](ComponentIPtr<ICircleShape> component)->float { return component->Position.Get().x; })
			));

		AddProperty("Radius", AddTextInput("Radius",
			FloatSetter_t([](float value, ComponentIPtr<ICircleShape> component) { component->Radius.Set(value); }),
			FloatGetter_t([](ComponentIPtr<ICircleShape> component)->float { return component->Radius.Get(); })
			));
			*/
	}

} }
