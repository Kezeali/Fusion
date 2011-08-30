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
	class Box2DFixture;

	class Box2DWorld;
	class Box2DTask;
	class Box2DInterpolateTask;

	class Box2DSystem : public IComponentSystem
	{
	public:
		Box2DSystem();
		virtual ~Box2DSystem()
		{}

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "Box2DSystem"; }
		
		std::shared_ptr<ISystemWorld> CreateWorld();
	};

	class Box2DWorld : public ISystemWorld
	{
		friend class Box2DTask;
		friend class Box2DInterpolateTask;
	public:
		Box2DWorld(IComponentSystem* system);
		~Box2DWorld();

		b2World* Getb2World() const { return m_World; }

	private:
		std::vector<std::string> GetTypes() const;

		ComponentPtr InstantiateComponent(const std::string& type);
		ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle);

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta);

		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		std::vector<ISystemTask*> GetTasks();

		std::vector<boost::intrusive_ptr<Box2DBody>> m_BodiesToCreate;
		std::vector<boost::intrusive_ptr<Box2DBody>> m_ActiveBodies;

		b2World* m_World;
		Box2DTask* m_B2DTask;
		Box2DInterpolateTask* m_B2DInterpTask;
	};

	class Box2DTask : public ISystemTask
	{
	public:
		Box2DTask(Box2DWorld* sysworld, b2World* const world);
		~Box2DTask();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Simulation; }

		PerformanceHint GetPerformanceHint() const { return LongSerial; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		Box2DWorld *m_B2DSysWorld;
		b2World* const m_World;
	};

	class Box2DInterpolateTask : public ISystemTask
	{
	public:
		Box2DInterpolateTask(Box2DWorld* sysworld);
		~Box2DInterpolateTask();

		void Update(const float delta);

		SystemType GetTaskType() const { return SystemType::Rendering; }

		PerformanceHint GetPerformanceHint() const { return Short; }

		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		Box2DWorld *m_B2DSysWorld;
	};

}

#endif
