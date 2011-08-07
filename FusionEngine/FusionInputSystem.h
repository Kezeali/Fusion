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

#ifndef H_FusionInputSystem
#define H_FusionInputSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionEntityComponent.h"

#include "FusionInputHandler.h"

namespace FusionEngine
{

	namespace Components
	{
		class Input;
	}

	class InputSystem : public IComponentSystem
	{
	public:
		InputSystem();
		virtual ~InputSystem()
		{}

		ISystemWorld* CreateWorld();

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "InputSystem"; }

		InputManager *m_InputManager;
	};

	class InputWorld : public ISystemWorld
	{
		friend class InputTask;
	public:
		InputWorld(IComponentSystem* system);
		~InputWorld();

	private:
		std::vector<std::string> GetTypes() const;

		std::shared_ptr<IComponent> InstantiateComponent(const std::string& type);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

		//void OnPrepare(const std::shared_ptr<IComponent>& component);
		void OnActivation(const std::shared_ptr<IComponent>& component);
		void OnDeactivation(const std::shared_ptr<IComponent>& component);

		ISystemTask* GetTask();

	private:

		InputTask* m_InputTask;

		std::vector<std::shared_ptr<Components::Input>> m_ActiveComponents;
	};

	class InputTask : public ISystemTask
	{
	public:
		InputTask(InputWorld* sysworld);
		~InputTask();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		InputWorld* m_InputWorld;

		std::map<int, std::deque<InputEvent>> m_PlayerInputEvents;
	};

}

#endif
