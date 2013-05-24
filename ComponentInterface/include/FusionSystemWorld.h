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

#ifndef H_FusionSystemWorld
#define H_FusionSystemWorld

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "Messaging/FusionMessage.h"
#include "FusionSystemType.h"

#include <EASTL/string.h>

namespace FusionEngine
{

	class IComponentSystem;
	class SystemTaskBase;

	namespace Messaging
	{
		class Router;
	}
	class RouterTask;

	//! World
	class SystemWorldBase : std::enable_shared_from_this<SystemWorldBase>
	{
	public:
		SystemWorldBase(IComponentSystem* system);
		virtual ~SystemWorldBase();

		std::shared_ptr<SystemWorldBase> GetShared() { return shared_from_this(); }

		IComponentSystem* GetSystem() const
    {
        return m_System;
    }

		SystemType GetSystemType() const;

		enum EngineRequest
		{
			Placeholder
		};

		struct ComponentDispatchRequest
		{
			enum Type
			{
				RefreshComponentTypes
			};

			Type type;
			SystemWorldBase* world;
		};
		
		void SendEngineRequest(EngineRequest request);

		void SendComponentDispatchRequest(ComponentDispatchRequest request);

		virtual void ProcessMessage(Messaging::Message message) = 0;

		virtual void OnWorldAdded(const std::string& other_world_name) {}
		virtual void OnWorldRemoved(const std::string& other_world_name) {}

		virtual std::vector<std::string> GetTypes() const = 0;
		virtual ComponentPtr InstantiateComponent(const std::string& type) = 0;

		//! Allows a system to prevent an entity from activating until all required resources are loaded
		virtual void Prepare(const ComponentPtr& component) { component->MarkReady(); }
		//! Cancel preparation & drop any references to the given component
		virtual void CancelPreparation(const ComponentPtr& component);
		//! Called when a component is activated
		virtual void OnActivation(const ComponentPtr& component) = 0;
		//! component.use_count() should be decremented by at least 1 when this function returns. This is checked with an assertion in the world manager.
		virtual void OnDeactivation(const ComponentPtr& component) = 0;

		virtual SystemTaskBase* GetTask();

		Messaging::Router* GetRouter() const;

		virtual std::vector<SystemTaskBase*> GetTasks()
		{
			FSN_ASSERT(GetTask() != nullptr);
			std::vector<SystemTaskBase*> tasks(1);
			tasks[0] = GetTask();
			return tasks;
		}

	private:
		IComponentSystem* m_System;

		RouterTask* m_RouterTask;
	};

}

#endif
