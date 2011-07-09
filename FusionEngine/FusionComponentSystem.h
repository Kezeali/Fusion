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

namespace FusionEngine
{

	class ISystemWorld;

	class ISystemTask
	{
	public:
		ISystemTask(ISystemWorld* world)
			: m_SystemWorld(world)
		{}
		virtual ~ISystemTask() {}

		ISystemWorld* GetSystemWorld() const { return m_SystemWorld; }

		virtual void Update(const float delta) = 0;

		enum PerformanceHint
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

	class ISystemWorld
	{
	public:
		virtual ~ISystemWorld() {}

		virtual std::vector<std::string> GetTypes() const = 0;
		virtual std::shared_ptr<IComponent> InstantiateComponent(const std::string& type) = 0;
		//! Instanciate method for physics / transform components
		virtual std::shared_ptr<IComponent> InstantiateComponent(const std::string& type, const Vector2& pos, float angle, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data)
		{
			return InstantiateComponent(type);
		}

		virtual void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data) = 0;

		virtual void OnActivation(const std::shared_ptr<IComponent>& component) = 0;
		//! component.use_count() should be decremented by at least 1 when this function returns. This is checked with an assertion in the world manager.
		virtual void OnDeactivation(const std::shared_ptr<IComponent>& component) = 0;

		virtual ISystemTask* GetTask() = 0;
	};

	typedef std::shared_ptr<ISystemWorld> SystemWorldPtr;

	typedef std::map<std::string, std::string> ComponentStaticProps;
	enum SystemType { Other, Physics, Rendering };

	class IComponentSystem
	{
	public:
		virtual ~IComponentSystem() {}

		virtual SystemType GetType() const = 0;

		virtual std::string GetName() const = 0;

		virtual ISystemWorld* CreateWorld() = 0;
	};

}

#endif