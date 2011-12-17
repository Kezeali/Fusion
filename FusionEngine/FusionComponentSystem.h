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

#include "FusionCommon.h" // just for Vector2 typedef (should fix this, since it is often the case)
#include "FusionPositionSerialisation.h"

namespace FusionEngine
{

	class ISystemTask;
	class ISystemWorld;
	class IComponentSystem;

	typedef std::shared_ptr<ISystemWorld> SystemWorldPtr;

	enum SystemType : uint8_t { Simulation = 0x01, Rendering = 0x02 };

	//! Component System
	class IComponentSystem
	{
	public:
		virtual ~IComponentSystem() {}

		virtual SystemType GetType() const = 0;

		virtual std::string GetName() const = 0;

		virtual std::shared_ptr<ISystemWorld> CreateWorld() = 0;
	};

	//! World
	class ISystemWorld
	{
	public:
		ISystemWorld(IComponentSystem* system)
			: m_System(system)
		{}
		virtual ~ISystemWorld() {}

		IComponentSystem* GetSystem() const
    {
        return m_System;
    }

		SystemType GetSystemType() const;

		struct ComponentType
		{
			std::string name;
			PositionSerialiser transformSerialiser;

			ComponentType(std::string&& n)
				: name(std::move(n))
			{}
			ComponentType(const std::string& n)
				: name(n)
			{}
			ComponentType(const std::string& n, const PositionSerialiser& serialiser)
				: name(n),
				transformSerialiser(serialiser)
			{}
			ComponentType(std::string&& n, PositionSerialiser&& serialiser)
				: name(std::move(n)),
				transformSerialiser(std::move(serialiser))
			{}
			ComponentType(const ComponentType& other)
				: name(other.name),
				transformSerialiser(other.transformSerialiser)
			{}
			ComponentType(ComponentType&& other)
				: name(std::move(other.name)),
				transformSerialiser(std::move(other.transformSerialiser))
			{}
		};
		virtual std::vector<std::pair<std::string, PositionSerialisationFunctor>> GetPositionSerialisers() const { return std::vector<std::pair<std::string, PositionSerialisationFunctor>>(); }
		virtual std::vector<std::string> GetTypes() const = 0;
		virtual ComponentPtr InstantiateComponent(const std::string& type) = 0;
		//! Instanciate method for physics / transform components
		virtual ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
		{
			return InstantiateComponent(type);
		}

		virtual void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data) = 0;

		//! Allows a system to prevent an entity from activating until all required resources are loaded
		virtual void Prepare(const ComponentPtr& component) { component->MarkReady(); }
		//! Cancel preperation & drop any references to the given component
		virtual void CancelPreparation(const ComponentPtr& component)
		{
			if (component->IsPreparing())
				FSN_EXCEPT(NotImplementedException, "CancelPreparation() isn't implemented by " + GetSystem()->GetName());
		}
		//! Called when a component is activated
		virtual void OnActivation(const ComponentPtr& component) = 0;
		//! component.use_count() should be decremented by at least 1 when this function returns. This is checked with an assertion in the world manager.
		virtual void OnDeactivation(const ComponentPtr& component) = 0;

		virtual ISystemTask* GetTask() { return nullptr; }

		virtual std::vector<ISystemTask*> GetTasks()
		{
			FSN_ASSERT(GetTask() != nullptr);
			std::vector<ISystemTask*> tasks(1u);
			tasks[0] = GetTask();
			return tasks;
		}

	private:
		IComponentSystem* m_System;
	};

	//! Task
	class ISystemTask
	{
	public:
		ISystemTask(ISystemWorld* world) : m_SystemWorld(world)
		{}
		virtual ~ISystemTask() {}

		ISystemWorld* GetSystemWorld() const { return m_SystemWorld; }

		SystemType GetSystemType() const;

		virtual std::string GetName() const { return GetSystemWorld()->GetSystem()->GetName(); }

		virtual void Update(const float delta) = 0;

		virtual SystemType GetTaskType() const = 0;

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
		ISystemWorld *m_SystemWorld;
	};


	inline SystemType ISystemTask::GetSystemType() const
	{
		return GetSystemWorld()->GetSystemType();
	}

	inline SystemType ISystemWorld::GetSystemType() const
	{
		return GetSystem()->GetType();
	}

}

#endif