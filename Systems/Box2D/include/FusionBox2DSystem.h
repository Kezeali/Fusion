/*
*  Copyright (c) 2011-2012 Fusion Project Team
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
	class Box2DPolygonFixture;

	class Box2DWorld;
	class Box2DTask;
	class Box2DInterpolateTask;

	class Box2DContactListenerDelegator;
	class Box2DContactListener;

	//! Creates Box2DWorld
	class Box2DSystem : public IComponentSystem
	{
	public:
		//! CTOR
		Box2DSystem();
		virtual ~Box2DSystem()
		{}

	private:
		SystemType GetType() const { return SystemType::Simulation; }

		std::string GetName() const { return "Box2DSystem"; }
		
		std::shared_ptr<ISystemWorld> CreateWorld();
	};

	class AuthorityContactManager;
	class TransformPinner;

	//! Manages a b2World
	class Box2DWorld : public ISystemWorld, public std::enable_shared_from_this<Box2DWorld>
	{
		friend class Box2DTask;
		friend class Box2DInterpolateTask;
	public:
		//! CTOR
		Box2DWorld(IComponentSystem* system);
		~Box2DWorld();

		void AddContactListener(const std::shared_ptr<Box2DContactListener>& listener);
		void RemoveContactListener(const std::shared_ptr<Box2DContactListener>& listener);

		// NOTE: due to how Box2DBody destruction is handled, Box2DWorld should
		//  NEVER have a Clear method (that clears the b2World), nor should you
		//  clear b2World in any other way - just destroy the current Box2DWorld
		//  and create a new one from the system.
		b2World* Getb2World() const { return m_World; }

		//! Initialise components now (usually they get initialised when the task runs)
		void InitialiseActiveComponents();

	private:
		std::vector<std::string> GetTypes() const;

		ComponentPtr InstantiateComponent(const std::string& type);
		ComponentPtr InstantiateComponent(const std::string& type, const Vector2& pos, float angle);

		void OnActivation(const ComponentPtr& component);
		void OnDeactivation(const ComponentPtr& component);

		std::vector<ISystemTask*> GetTasks();

		std::vector<boost::intrusive_ptr<Box2DBody>> m_BodiesToCreate;
		std::vector<boost::intrusive_ptr<Box2DBody>> m_ActiveBodies;

		std::vector<boost::intrusive_ptr<Box2DPolygonFixture>> m_PolygonFixtures;

		AuthorityContactManager* m_AuthorityContactManager;
		std::shared_ptr<TransformPinner> m_TransformPinner;

		b2World* m_World;
		Box2DTask* m_B2DTask;
		Box2DInterpolateTask* m_B2DInterpTask;

		Box2DContactListenerDelegator* m_ContactListenerDelegator;
	};

	//! Updates Box2D-based physics components
	class Box2DTask : public ISystemTask
	{
	public:
		//! CTOR
		Box2DTask(Box2DWorld* sysworld, b2World* const world);
		~Box2DTask();

		void Update(const float delta);

		//! Simulation
		SystemType GetTaskType() const { return SystemType::Simulation; }

		//! LongSerial
		PerformanceHint GetPerformanceHint() const { return LongSerial; }

		//! false
		bool IsPrimaryThreadOnly() const
		{
			return false;
		}

	protected:
		Box2DWorld *m_B2DSysWorld;
		b2World* const m_World;
	};

	//! Does interpolation on objects for which it is enabled
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
