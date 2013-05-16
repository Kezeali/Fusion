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

#ifndef H_FusionComponentSystem
#define H_FusionComponentSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"

#include "FusionVectorTypes.h"

#include "Messaging/FusionRouterTask.h"

#include <tbb/concurrent_queue.h>

namespace FusionEngine
{

	class SystemTaskBase;
	class SystemWorldBase;
	class IComponentSystem;

	typedef std::shared_ptr<SystemWorldBase> SystemWorldPtr;

	enum SystemType : uint8_t { Simulation = 0x01, Rendering = 0x02, Streaming = 0x04, Messaging = 0x08, Editor = 0x10 };

	//! Component System
	class IComponentSystem
	{
	public:
		virtual ~IComponentSystem() {}

		virtual SystemType GetType() const = 0;

		virtual std::string GetName() const = 0;

		virtual void RegisterScriptInterface(asIScriptEngine* engine) {}

		virtual std::shared_ptr<SystemWorldBase> CreateWorld() = 0;
	};

	//! World
	class SystemWorldBase
	{
	public:
		SystemWorldBase(IComponentSystem* system)
			: m_System(system)
		{}
		virtual ~SystemWorldBase() {}

		IComponentSystem* GetSystem() const
    {
        return m_System;
    }

		SystemType GetSystemType() const;

		/*void SendMessage(Message message)
		{
			m_OutgoingMessages.push(message);
		}

		void ReceiveMessage(Message message)
		{
			m_IncommingMessages.push(message);
		}

		void PostSelfMessage(const char type, boost::any data)
		{
			ReceiveMessage(Message(Message::TargetType::System, GetSystem()->GetName(), type, data));
		}

		void PostSystemMessage(std::string systemName, const char type, boost::any data)
		{
			SendMessage(Message(Message::TargetType::System, systemName, type, data));
		}

		enum class EngineMessage
		{
			RefreshComponentTypes
		};

		void PostEngineMessage(EngineMessage message)
		{
			SendMessage(Message(Message::TargetType::Engine, "", 0, message));
		}

		bool TryPopOutgoingMessage(Message& message)
		{
			return m_OutgoingMessages.try_pop(message);
		}

		bool TryPopIncommingMessage(Message& message)
		{
			return m_IncommingMessages.try_pop(message);
		}

		void ProcessReceivedMessages()
		{
			Message message;
			while (TryPopIncommingMessage(message))
			{
				ProcessMessage(message);
			}
		}*/

		virtual void ProcessMessage(Messaging::Message message) = 0;

		virtual void OnWorldAdded(const std::string& other_world_name) {}
		virtual void OnWorldRemoved(const std::string& other_world_name) {}

		virtual std::vector<std::string> GetTypes() const = 0;
		virtual ComponentPtr InstantiateComponent(const std::string& type) = 0;

		//! Allows a system to prevent an entity from activating until all required resources are loaded
		virtual void Prepare(const ComponentPtr& component) { component->MarkReady(); }
		//! Cancel preparation & drop any references to the given component
		virtual void CancelPreparation(const ComponentPtr& component)
		{
			if (component->IsPreparing())
				FSN_EXCEPT(NotImplementedException, "CancelPreparation() isn't implemented by " + GetSystem()->GetName());
		}
		//! Called when a component is activated
		virtual void OnActivation(const ComponentPtr& component) = 0;
		//! component.use_count() should be decremented by at least 1 when this function returns. This is checked with an assertion in the world manager.
		virtual void OnDeactivation(const ComponentPtr& component) = 0;

		virtual SystemTaskBase* GetTask() { return nullptr; }

		virtual std::vector<SystemTaskBase*> GetTasks()
		{
			FSN_ASSERT(GetTask() != nullptr);
			std::vector<SystemTaskBase*> tasks(1);
			tasks[0] = GetTask();
			return tasks;
		}

	private:
		IComponentSystem* m_System;

		std::unique_ptr<RouterTask> m_RouterTask;
	};

	//! Task
	class SystemTaskBase
	{
	public:
		SystemTaskBase(SystemWorldBase* world, const std::string& name)
			: m_SystemWorld(world),
			m_Name(name)
		{}
		virtual ~SystemTaskBase() {}

		SystemWorldBase* GetSystemWorld() const { return m_SystemWorld; }

		SystemType GetSystemType() const;

		std::string GetName() const { return m_Name; }

		virtual void Update() = 0;

		virtual SystemType GetTaskType() const = 0;

		virtual std::vector<std::string> GetDependencies() const { return std::vector<std::string>(); }

		enum PerformanceHint : uint16_t
		{
			LongSerial = 0,
			LongParallel,
			Short,
			NoPerformanceHint,
			NumPerformanceHints
		};
		virtual PerformanceHint GetPerformanceHint() const { return NoPerformanceHint; }

		virtual bool IsPrimaryThreadOnly() const = 0;

	protected:
		SystemWorldBase *m_SystemWorld;
		std::string m_Name;
	};


	inline SystemType SystemTaskBase::GetSystemType() const
	{
		return GetSystemWorld()->GetSystemType();
	}

	inline SystemType SystemWorldBase::GetSystemType() const
	{
		return GetSystem()->GetType();
	}

}

#endif