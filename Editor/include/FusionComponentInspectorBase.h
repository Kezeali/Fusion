/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionComponentInspectorBase
#define H_FusionComponentInspectorBase

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentInspector.h"
#include "FusionPropertySignalingSystem.h"

#include "FusionArchetypalEntityManager.h"
#include "FusionResourceEditorFactory.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>

#include <EASTL/vector.h>

// TEMP: for hacky circle editor setup
#include "FusionEntity.h"

namespace FusionEngine { namespace Inspectors
{

	class ComponentInspectorBase : public ComponentInspector
	{
	public:
		GWEN_CONTROL_INLINE(ComponentInspectorBase, ComponentInspector)
		{
			m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope = false;
			InitUI();
		}

		//! Derived class should call AddXInput methods here
		virtual void InitUI() = 0;

	protected:
		eastl::vector<ComponentPtr> m_Components;

		typedef Gwen::Controls::PropertyRow* PropertyRowPtr;
		struct PropertySubscriptionData
		{
			size_t propertyIndex;
			std::vector<SyncSig::HandlerConnection_t> connections;
			Gwen::Controls::PropertyRow* propertyRow; // the property-row to be refreshed when this listener is triggered
			std::function<void (ComponentPtr, size_t, PropertyRowPtr)> update;
		};
		std::map<std::string, PropertySubscriptionData> m_Properties;

		Gwen::Controls::Properties* m_PropertiesControl;

		bool m_ResettingUI_DontApplyChangesToArchetypeInstance_YouDope;

		void SetComponents(const std::vector<ComponentPtr>& components);

		void ResetUIValues();

		void RefreshUIForProp(const std::string& name);

		void AddTextProperty(const std::string& label,
			std::function<std::string (ComponentPtr)> get,
			std::function<void (ComponentPtr, std::string)> set);

		void AddIntProperty(const std::string& label,
			std::function<int (ComponentPtr)> get,
			std::function<void (ComponentPtr, int)> set);
		
		void AddFloatProperty(const std::string& label,
			std::function<float (ComponentPtr)> get,
			std::function<void (ComponentPtr, float)> set);

		void AddBoolProperty(const std::string& label,
			std::function<bool (ComponentPtr)> get,
			std::function<void (ComponentPtr, bool)> set);

	};

}}

#endif