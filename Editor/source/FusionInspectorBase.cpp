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

#include "PrecompiledHeaders.h"

//#include "FusionInspectorBase.h"
//
//#include "FusionInspectorUtils.h"
//
//#include <Rocket/Core.h>
//#include <Rocket/Controls.h>
//
//#include <boost/lexical_cast.hpp>
//
//namespace FusionEngine { namespace Inspectors
//{
//
//	void InspectorBase::AddTextInput(const std::string& name)
//	{
//		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> input_element;
//
//		Rocket::Core::Factory::InstanceElementText(this, "X");
//
//		Rocket::Core::XMLAttributes attributes;
//		attributes.Set("type", "text");
//		attributes.Set("id", "x_input");
//		attributes.Set("name", "x");
//		attributes.Set("value", "0");
//		attributes.Set("size", "6");
//		Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(this,
//			"input",
//			"input",
//			attributes);
//
//		addControl(this, input_element, text_input);
//
//		if (input_element)
//			m_Inputs.push_back(input_element);
//	}
//
//} }