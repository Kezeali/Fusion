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

#ifndef H_FusionArchetypeFactory
#define H_FusionArchetypeFactory

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"
#include "FusionTypes.h"

#include "FusionComponentFactory.h"
#include "FusionResource.h"

#include <string>
#include <map>
#include <memory>

//#include <boost/signals2/signal.hpp>
#include <boost/thread/mutex.hpp>

namespace FusionEngine
{

	namespace Archetypes
	{
		class Profile;
	}
	class ArchetypeDefinitionAgent;

	class SaveDataArchive;

	//! General purpose resource holder (for keeping resources alive when they aren't in use)
	class ResourceSustainer
	{
	public:
		virtual ~ResourceSustainer() {}
		//! Prevent the cache from clearing
		virtual void Sustain() = 0;
		//! Allow the cache to clear
		virtual void EndSustain() = 0;

		//! Adds a resource to the cache
		virtual void StoreResource(const ResourceDataPtr& resource) = 0;
	};

	//! Firm but fair
	class ArchetypeFactoryManager : public Singleton<ArchetypeFactoryManager>
	{
	public:
		ArchetypeFactoryManager(ComponentFactory* factory, EntityManager* manager);
		ArchetypeFactoryManager(ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator);

		static void Sustain();
		static void EndSustain();
		static void StoreFactory(const ResourceDataPtr& resource);

		//! Returns true if instances should be linked to definitions (so that changes are tracked)
		static bool IsInstanceLinkingEnabled() { return getSingleton().m_Instantiator != nullptr; }

		static EntityInstantiator* GetInstantiator() { return getSingleton().m_Instantiator; }
		static ComponentFactory* GetComponentFactory() { return getSingleton().m_ComponentFactory; }
		static EntityManager* GetEntityManager() { return getSingleton().m_EntityManager; }
	private:
		std::unique_ptr<ResourceSustainer> m_ResourceSustainer;
		ComponentFactory* m_ComponentFactory;
		EntityManager* m_EntityManager;
		EntityInstantiator* m_Instantiator;
	};

	//! Data that defines the type of entity that a given factory can instantiate
	struct ArchetypeData
	{
	public:
		EntityPtr Archetype;
		std::shared_ptr<Archetypes::Profile> Profile;
		std::shared_ptr<ArchetypeDefinitionAgent> Agent;

		ArchetypeData() {}
		explicit ArchetypeData(EntityPtr entity) : Archetype(entity) {}
		ArchetypeData(ArchetypeData&& other) : Archetype(std::move(other.Archetype)) {}
	private:
		ArchetypeData(const ArchetypeData&) {}
		ArchetypeData& operator=(const ArchetypeData&) {}
	};

	//! Used to define archetypes
	class ArchetypeFactory
	{
	public:
		//! Default CTOR
		ArchetypeFactory();
		//! CTOR
		/*!
		* \param instantiator
		* The component instantiator which should be used to add components to live archetype instances. Only needed
		* in edit mode.
		*
		* \param resource
		* This factory's resource container, so it can be kept alive by archetype instances
		*/
		ArchetypeFactory(EntityInstantiator* instantiator, ResourceContainer* resource);
		~ArchetypeFactory();

		void SetEditable(bool value) { m_Editable = value; }

		void Save(std::ostream& stream);
		void Load(std::unique_ptr<std::istream> stream, ComponentFactory* factory, EntityManager* manager);

		const EntityPtr& GetArchetype() const { return m_ArchetypeData.Archetype; }

		//! Makes an instance of the given archetype
		EntityPtr MakeInstance(ComponentFactory* factory, const Vector2& pos, float angle) const;
		//! Makes an unlinked instance of the given archetype
		/*!
		* An unlinked instance is simply a clone of the archetype definition. Linked instances
		* change as the archetype entity changes.
		*
		* \remarks
		* Linked instances with become unlinked if the factory is unloaded.
		*/
		EntityPtr MakeUnlinkedInstance(ComponentFactory* factory, const Vector2& pos, float angle) const;

		//! Defines the given archetype using the given entity
		void DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type_id, const EntityPtr& entity);

		boost::signals2::connection AddChangeListener(const std::function<void (void)>& listener);

	private:
		std::string m_TypeName;
		ArchetypeData m_ArchetypeData;

		bool m_Editable;

		mutable boost::mutex m_Mutex;

		EntityInstantiator* m_ComponentInstantiator;

		ResourceContainer* m_ResourceContainer;

		EntityPtr makeInstance(ComponentFactory* factory, const Vector2& pos, float angle) const;
		void linkInstance(const EntityPtr& instance) const;
	};

	//! Archetype resource loader callback
	void LoadArchetypeResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);
	//! Archetype resource unloader callback
	void UnloadArchetypeResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data);

	class ArchetypeInstantiator
	{
	};

}

#endif
