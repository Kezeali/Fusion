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

	TransformInspector::TransformInspector(const Rocket::Core::String& tag)
		: ComponentInspector(tag)
	{
		// X
		{
			Rocket::Core::Factory::InstanceElementText(this, "X");

			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", "x_input");
			attributes.Set("name", "x");
			attributes.Set("value", "0");
			attributes.Set("size", "6");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(this,
				"input",
				"input",
				attributes);

			addControl(this, m_XInput, text_input);
		}
		// Y
		{
			Rocket::Core::Factory::InstanceElementText(this, "Y");

			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", "y_input");
			attributes.Set("name", "y");
			attributes.Set("value", "0");
			attributes.Set("size", "6");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(this,
				"input",
				"input",
				attributes);

			addControl(this, m_YInput, text_input);
		}
		// Angle
		{
			Rocket::Core::Factory::InstanceElementText(this, "Angle");

			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", "angle_input");
			attributes.Set("name", "angle");
			attributes.Set("value", "0");
			attributes.Set("size", "5");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(this,
				"input",
				"input",
				attributes);

			addControl(this, m_AngleInput, text_input);
		}
		// Depth
		{
			Rocket::Core::Factory::InstanceElementText(this, "Depth");

			Rocket::Core::XMLAttributes attributes;
			attributes.Set("type", "text");
			attributes.Set("id", "depth_input");
			attributes.Set("name", "depth");
			attributes.Set("value", "0");
			attributes.Set("size", "3");
			Rocket::Core::Element* text_input = Rocket::Core::Factory::InstanceElement(this,
				"input",
				"input",
				attributes);

			addControl(this, m_DepthInput, text_input);
		}
	}

	void TransformInspector::SetComponents(const std::vector<ComponentPtr>& components)
	{
		for (auto it = components.begin(), end = components.end(); it != end; ++it)
		{
			if (auto typed = ComponentIPtr<ITransform>(*it))
				m_Components.push_back(typed);
		}

		if (!m_Components.empty())
			InitUI();
	}

	void TransformInspector::ReleaseComponents()
	{
		m_Components.clear();
	}

	void TransformInspector::InitUI()
	{
		m_XInput->SetValue("");
		m_YInput->SetValue("");
		m_AngleInput->SetValue("");
		m_DepthInput->SetValue("");
		bool first = true;
		for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
		{
			const auto pos = (*it)->Position.Get();
			if (first)
			{
				initUIValue(m_XInput, pos.x);
				initUIValue(m_YInput, pos.y);
				initUIValue(m_AngleInput, (*it)->Angle.Get());
				initUIValue(m_DepthInput, (*it)->Depth.Get());
			}
			else
			{
				clearIfNotEqual(m_XInput, pos.x);
				clearIfNotEqual(m_YInput, pos.y);
				clearIfNotEqual(m_AngleInput, (*it)->Angle.Get());
				clearIfNotEqual(m_DepthInput, (*it)->Depth.Get());
			}

			first = false;
		}
	}

	void TransformInspector::ProcessEvent(Rocket::Core::Event& ev)
	{
		try
		{
			if (ev == "enter")
			{
				if (ev.GetTargetElement() == m_XInput.get() || ev.GetTargetElement() == m_YInput.get())
				{
					Vector2 value(getUIValue<float>(m_XInput), getUIValue<float>(m_YInput));
					for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
						(*it)->Position.Set(value);
				}
				else if (ev.GetTargetElement() == m_AngleInput.get())
				{
					auto value = getUIValue<float>(m_AngleInput);
					for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
						(*it)->Angle.Set(value);
				}
				else if (ev.GetTargetElement() == m_DepthInput.get())
				{
					auto value = getUIValue<int>(m_DepthInput);
					for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
						(*it)->Depth.Set(value);
				}
			}
		}
		catch (boost::bad_lexical_cast&)
		{
			InitUI();
		}
	}

} }
