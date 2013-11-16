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

#ifndef H_FusionASScriptInspector
#define H_FusionASScriptInspector

#include "FusionPrerequisites.h"

//#include "FusionComponentInspector.h"
#include "FusionComponentInspectorBase.h"

#include "FusionAngelScriptComponent.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

namespace FusionEngine { namespace Inspectors
{

	class ASScriptInspector : public ComponentInspectorBase
	{
	public:
		GWEN_CONTROL(ASScriptInspector, ComponentInspectorBase);

		void SetComponents(const std::vector<ComponentPtr>& components);
		void ReleaseComponents();

	private:
		//struct ScriptPropertyInput
		//{
		//	std::string name;
		//	unsigned int index;
		//	int type_id;
		//	size_t array_index;
		//	Gwen::Controls::Base* input;
		//};
		//boost::multi_index_container<ScriptPropertyInput, boost::multi_index::indexed_by<
		//	boost::multi_index::sequenced<>,
		//	boost::multi_index::ordered_unique<boost::multi_index::member<ScriptPropertyInput, boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput>, &ScriptPropertyInput::input>>,
		//	boost::multi_index::ordered_non_unique<boost::multi_index::member<ScriptPropertyInput, unsigned int, &ScriptPropertyInput::index>>
		//>> m_Inputs;

		std::vector<ComponentIPtr<ASScript>> m_Components;

		//boost::intrusive_ptr<Rocket::Core::Element> m_PropertiesSection;

		std::vector<SyncSig::HandlerConnection_t> m_Connections;

		asIScriptEngine* m_ScriptEngine;

		template <class T>
		void setValue(unsigned int index, T& value, int typeId)
		{
			for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
				(*it)->SetPropertyRaw(index, &value, typeId);
		}

		//void AddPropertyControl(Rocket::Core::Element* parent, unsigned int index, const std::string& name, unsigned int type_id);

		void InitUI();

		void RefreshPropertyValue(unsigned int index);

		//void RefreshPropertyValue(const ScriptPropertyInput& prop, const ComponentIPtr<ASScript>& component, bool first = true);

		void ResetUIValues();

		//void ProcessEvent(Rocket::Core::Event& ev);
	};

} }

#endif
