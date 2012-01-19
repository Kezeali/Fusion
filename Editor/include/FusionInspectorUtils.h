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

#ifndef H_FusionInspectorUtils
#define H_FusionInspectorUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Core.h>
#include <Rocket/Controls.h>

namespace FusionEngine { namespace Inspectors
{
	template <class T>
	inline void addControl(Rocket::Core::Element* parent, boost::intrusive_ptr<T>& prop, Rocket::Core::Element* element)
	{
		if (element)
		{
			parent->AppendChild(element);

			prop.reset(dynamic_cast<T*>(element));

			element->RemoveReference();
		}
	}

	template <class T>
	inline void initUIValue(const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& ui_element, const T& value)
	{
		try
		{
			auto strval = boost::lexical_cast<std::string>(value);
			ui_element->SetValue(Rocket::Core::String(strval.data(), strval.data() + strval.size()));
		}
		catch (boost::bad_lexical_cast&)
		{
			SendToConsole(("Failed to init property inspector input '" + ui_element->GetId()).CString());
		}
	}

	template <class T>
	inline void clearIfNotEqual(const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& ui_element, const T& value)
	{
		try
		{
			auto currentValue = ui_element->GetValue();
			if (ui_element->GetValue().Empty() || std::string(currentValue.CString(), currentValue.CString() + currentValue.Length()) != boost::lexical_cast<std::string>(value))
				ui_element->SetValue("");
		}
		catch (boost::bad_lexical_cast&)
		{
			SendToConsole(("Failed to init property inspector input '" + ui_element->GetId()).CString());
		}
	}

	//! if 'first' is false, calls clearIfNotEqual. Otherwise calls initUIValue
	template <class T>
	inline void initUIValue(bool first, const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& ui_element, const T& value)
	{
		if (!first)
			clearIfNotEqual(ui_element, value);
		else
			initUIValue(ui_element, value);
	}

	template <class T>
	inline T getUIValue(const boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>& input)
	{
		auto strval = input->GetValue();
		return boost::lexical_cast<T>(std::string(strval.CString(), strval.CString() + strval.Length()));
	}

	template <class PropT, class ElemT>
	inline bool changeProperty(PropT& dest, Rocket::Core::Element* changed_element, const boost::intrusive_ptr<ElemT>& expected_element)
	{
		if (changed_element == expected_element)
		{
			dest.Set(getUIValue(expected_element));
			return true;
		}
		else
			return false;
	}

} }

#endif
