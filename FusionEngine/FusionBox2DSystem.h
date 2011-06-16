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

#ifndef H_FusionBox2DSystem
#define H_FusionBox2DSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentSystem.h"
#include "FusionEntityComponent.h"

class b2World;

namespace FusionEngine
{

	class Box2DBody;

	class Box2DWorld;

	class Box2DSystem : public IComponentSystem
	{
	public:
		Box2DSystem();
		virtual ~Box2DSystem()
		{}

	private:
		SystemType GetType() const { return SystemType::Physics; }

		std::string GetName() const { return "Box2DSystem"; }

		ISystemWorld* CreateWorld();

	};

	class Box2DWorld : public ISystemWorld
	{
	public:
		Box2DWorld();
		virtual ~Box2DWorld()
		{}

	private:
		std::vector<std::string> GetTypes() const;

		std::shared_ptr<IComponent> InstantiateComponent(const std::string& type, const Vector2& pos, float angle);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data);

		void OnActivation(const std::shared_ptr<IComponent>& component);
		void OnDeactivation(const std::shared_ptr<IComponent>& component);

		std::vector<std::shared_ptr<Box2DBody>> m_ActiveBodies;

		b2World* m_World;
	};

}

#endif
