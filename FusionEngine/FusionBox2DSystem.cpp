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

#include "FusionBox2DSystem.h"

#include "FusionBox2DComponent.h"

namespace FusionEngine
{

	ISystemWorld* Box2DSystem::CreateWorld()
	{
		return new Box2DWorld();
	}

	Box2DWorld::Box2DWorld()
	{
		b2Vec2 gravity(0.0f, 0.0f);
		m_World = new b2World(gravity, true);

		m_B2DTask = new Box2DTask(this, m_World);
	}

	Box2DWorld::~Box2DWorld()
	{
		delete m_B2DTask;
		delete m_World;
	}

	std::vector<std::string> Box2DWorld::GetTypes() const
	{
		static const std::string types[] = { "B2Body", "B2Fixture" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	std::shared_ptr<IComponent> Box2DWorld::InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
	{
		if (type == "B2Body")
		{
			b2BodyDef def;
			def.position.Set(pos.x, pos.y);
			def.angle = angle;

			auto com = std::make_shared<Box2DBody>(m_World->CreateBody(&def));
			return com;
		}
	}

	void Box2DWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		auto b2Component = std::dynamic_pointer_cast<Box2DBody>(component);
		if (b2Component)
		{
			m_ActiveBodies.push_back(b2Component);
			b2Component->SetActive(true);
		}
	}

	void Box2DWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto b2Component = std::dynamic_pointer_cast<Box2DBody>(component);
		if (b2Component)
		{
			// Deactivate the body in the simulation
			b2Component->SetActive(false);
			// Find and remove the deactivated body (from the Active Bodies list)
			auto _where = std::find(m_ActiveBodies.begin(), m_ActiveBodies.end(), b2Component);
			if (_where != m_ActiveBodies.end())
			{
				_where->swap(m_ActiveBodies.back());
				m_ActiveBodies.pop_back();
			}
		}
	}

	ISystemTask* Box2DWorld::GetTask()
	{
		return m_B2DTask;
	}

	void Box2DWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		if (type == "B2Body")
		{
			Box2DBody::MergeDelta(result, current_data, new_data);
		}
		else if (type == "B2Fixture")
		{
			Box2DFixture::MergeDelta(result, current_data, new_data);
		}
	}

	Box2DTask::Box2DTask(Box2DWorld* sysworld, b2World* const world)
		: ISystemTask(sysworld),
		m_B2DSysWorld(sysworld),
		m_World(world)
	{
	}

	Box2DTask::~Box2DTask()
	{
	}

	void Box2DTask::Update(const float delta)
	{
		m_World->Step(delta, 10, 10);
		//auto activeBodies = m_B2DSysWorld->m_ActiveBodies;
		//for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		//{
		//	auto body = *it;
		//}
	}

}