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
	}

	std::vector<std::string> Box2DWorld::GetTypes() const
	{
		static const std::string types[] = { "B2Body", "B2Fixture" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	static bool att_is_true(const std::string& att)
	{
		return att == "t" || att == "1" || att == "true";
	}

	const std::shared_ptr<IComponent> &Box2DWorld::InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
	{
		if (type == "B2Body")
		{
			b2BodyDef def;
			def.position.Set(pos.x, pos.y);
			def.angle = angle;

			auto com = std::make_shared<Box2DBody>(m_World->CreateBody(&def));
		}
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

}