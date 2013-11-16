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

#ifndef H_FusionEntityInspector
#define H_FusionEntityInspector

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionTypes.h"

#include <deque>
#include <functional>
#include <string>

#include <boost/intrusive_ptr.hpp>

#include <Gwen/Controls/Base.h>

#include "FusionConsole.h"

namespace FusionEngine { namespace Inspectors
{

	class ElementEntityInspector : public Gwen::Controls::Base
	{
	public:
		GWEN_CONTROL(ElementEntityInspector, Gwen::Controls::Base);
		~ElementEntityInspector()
		{
			SendToConsole("Closed inspector");
			if (m_CloseCallback)
				m_CloseCallback();
		}

		void SetEntity(const EntityPtr& entity);

		void SetCloseCallback(const std::function<void (void)>& fn) { m_CloseCallback = fn; }

		typedef std::function<void (bool, const EntityPtr&)> BoolSetter_t;
		typedef std::function<bool (const EntityPtr&)> BoolGetter_t;
		typedef std::function<void (std::string, const EntityPtr&)> StringSetter_t;
		typedef std::function<std::string (const EntityPtr&)> StringGetter_t;

	private:
		EntityPtr m_Entity;

		std::function<void (void)> m_CloseCallback;

		//boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> apply_button;

		//struct Input
		//{
		//	boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> ui_element;

		//	std::function<void (const EntityPtr& entity)> publishToEntity;
		//	std::function<void (const EntityPtr& entity)> receiveFromEntity;
		//};

		//std::map<boost::intrusive_ptr<Rocket::Core::Element>, Input> m_Inputs;

		//std::deque<boost::intrusive_ptr<Rocket::Core::Element>> m_InlineSections;
		//class InlineSection
		//{
		//public:
		//	InlineSection(ElementEntityInspector* parent_)
		//		: parent(parent_)
		//	{
		//		auto line = Rocket::Core::Factory::InstanceElement(parent, "p", "p", Rocket::Core::XMLAttributes());
		//		parent->AppendChild(line);
		//		line->RemoveReference();
		//		parent->m_InlineSections.push_back(line);
		//	}
		//	~InlineSection()
		//	{
		//		parent->m_InlineSections.pop_back();
		//	}

		//	ElementEntityInspector* parent;
		//	boost::intrusive_ptr<Rocket::Core::Element> line;
		//};

		void InitUI();

		void ResetUIValues();

		void AddTextInput(const std::string& name, StringSetter_t setter, StringGetter_t getter, int size = 0);
		void AddTextInput(const std::string& name, StringGetter_t getter, int size = 0);
		void AddToggleInput(const std::string& name, BoolSetter_t setter, BoolGetter_t getter);

		//void ProcessEvent(Rocket::Core::Event& ev);
	};

} }

#endif
