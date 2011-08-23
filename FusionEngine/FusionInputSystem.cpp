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

#include "FusionStableHeaders.h"

#include "FusionInputSystem.h"

#include "FusionInputComponent.h"

namespace FusionEngine
{

	using namespace Components;

	InputSystem::InputSystem()
	{
	}

	std::shared_ptr<ISystemWorld> InputSystem::CreateWorld()
	{
		return std::make_shared<InputWorld>(this);
	}

	InputWorld::InputWorld(IComponentSystem* system)
		: ISystemWorld(system)
	{
		m_InputTask = new InputTask(this);
	}

	InputWorld::~InputWorld()
	{
		delete m_InputTask;
	}

	std::vector<std::string> InputWorld::GetTypes() const
	{
		static const std::string types[] = { "Input" };
		return std::vector<std::string>(types, types + 1);
	}

	std::shared_ptr<IComponent> InputWorld::InstantiateComponent(const std::string& type)
	{
		if (type == "Input")
		{
		}
		return std::shared_ptr<IComponent>();
	}

	void InputWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		auto com = std::dynamic_pointer_cast<Input>(component);
		if (com)
		{
			m_ActiveComponents.push_back(com);
		}
	}

	void InputWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto com = std::dynamic_pointer_cast<Input>(component);
		if (com)
		{
			auto _where = std::find(m_ActiveComponents.begin(), m_ActiveComponents.end(), com);
			if (_where != m_ActiveComponents.end())
			{
				_where->swap(m_ActiveComponents.back());
				m_ActiveComponents.pop_back();
			}
		}
	}

	ISystemTask* InputWorld::GetTask()
	{
		return m_InputTask;
	}

	void InputWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
	}

	InputTask::InputTask(InputWorld* world)
		: ISystemTask(world),
		m_InputWorld(world)
	{
	}

	InputTask::~InputTask()
	{
	}

	void InputTask::Update(const float delta)
	{
	}

}
