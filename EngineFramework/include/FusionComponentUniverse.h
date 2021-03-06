/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionComponentUniverse
#define H_FusionComponentUniverse

#include "FusionPrerequisites.h"

#include "FusionComponentFactory.h"

#include "Messaging/FusionRouter.h"

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
//#include <boost/multi_index_container.hpp>
//#include <boost/multi_index/hashed_index.hpp>
#include <boost/thread/mutex.hpp>

namespace FusionEngine
{

	namespace System
	{
		class WorldBase;
	}
	class ComponentTypeInfoCache;

	//! Stores component worlds
	class ComponentUniverse : public ComponentFactory, public Messaging::Router
	{
	public:
		ComponentUniverse();
		virtual ~ComponentUniverse();

		void AddWorld(const std::shared_ptr<System::WorldBase>& world);
		void RemoveWorld(const std::shared_ptr<System::WorldBase>& world);
		std::shared_ptr<System::WorldBase> GetWorldByComponentType(const std::string& type);

		std::map<std::string, std::shared_ptr<System::WorldBase>> GetWorlds() const;

		ComponentPtr InstantiateComponent(const std::string& type);

		void OnActivated(const ComponentPtr& component);
		void OnDeactivated(const ComponentPtr& component);

	private:
		typedef std::map<std::string, std::shared_ptr<System::WorldBase>> WorldsMap_t;
		WorldsMap_t m_Worlds;

		struct tag {
			struct type {};
			struct world {};
		};
		typedef boost::bimaps::bimap<
			boost::bimaps::unordered_set_of< boost::bimaps::tagged<std::string, tag::type> >,
			boost::bimaps::unordered_multiset_of< boost::bimaps::tagged<std::shared_ptr<System::WorldBase>, tag::world> >
		> ComponentTypes_t;
		ComponentTypes_t m_ComponentTypes;

		std::shared_ptr<ComponentTypeInfoCache> m_ComponentTypeInfoCache;

		void ProcessMessage(Messaging::Message message) override;
	};

}

#endif
