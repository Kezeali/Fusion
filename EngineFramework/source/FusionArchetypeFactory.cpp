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

#include "PrecompiledHeaders.h"

#include "FusionArchetypeFactory.h"

#include "FusionArchetypalEntityManager.h"
#include "FusionArchetype.h"
#include "FusionBinaryStream.h"
#include "FusionCLStream.h"
#include "FusionEntity.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionResourceLoaderUtils.h"
#include "FusionSaveDataArchive.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <tbb/atomic.h>

namespace bio = boost::iostreams;

namespace FusionEngine
{

	const size_t s_DefaultVolatileCapacity = 100;

	//! Implements ResourceSustainer
	class SimpleResourceSustainer : public ResourceSustainer
	{
	public:
		//! CTOR
		SimpleResourceSustainer();

		//! Prevent the cache from clearing
		virtual void Sustain();
		//! Allow the cache to clear
		virtual void EndSustain();

		//! Set the number of resources that are cached while NOT sustaining
		void SetVolatileCapacity(size_t capacity) { m_VolitileCapacity = capacity; }
		size_t GetVolatileCapacity() const { return m_VolitileCapacity; }

		//! Adds a resource to the cache
		void StoreResource(const ResourceDataPtr& resource);

	private:
		class StoredResource
		{
		public:
			ResourceDataPtr resource;
			boost::signals2::connection hotReloadConnection;

			bool HotReloadEvents(ResourceDataPtr eventResource, ResourceContainer::HotReloadEvent ev)
			{
				switch (ev)
				{
				case ResourceContainer::HotReloadEvent::Validate:
					resource.reset();
					break;
				case ResourceContainer::HotReloadEvent::PostReload:
					resource = eventResource;
				}
				return true;
			}

			StoredResource(const ResourceDataPtr& resource);

			StoredResource(StoredResource&& other)
				: resource(std::move(other.resource)),
				hotReloadConnection(std::move(other.hotReloadConnection))
			{
			}

			~StoredResource()
			{
				hotReloadConnection.disconnect();
			}

			StoredResource& operator =(StoredResource&& other)
			{
				resource = std::move(other.resource);
				hotReloadConnection = std::move(other.hotReloadConnection);
				return *this;
			}
		};
		std::list<StoredResource> m_Volatile;
		std::list<ResourceDataPtr> m_Sustained;

		tbb::atomic<bool> m_Sustaining;
		size_t m_VolitileCapacity;
	};

	ArchetypeFactoryManager::ArchetypeFactoryManager(ComponentFactory* factory, EntityManager* manager)
		: m_ResourceSustainer(new SimpleResourceSustainer()),
		m_ComponentFactory(factory),
		m_EntityManager(manager),
		m_Instantiator(nullptr)
	{
	}

	ArchetypeFactoryManager::ArchetypeFactoryManager(ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator)
		: m_ResourceSustainer(new SimpleResourceSustainer()),
		m_ComponentFactory(factory),
		m_EntityManager(manager),
		m_Instantiator(instantiator)
	{
	}

	void ArchetypeFactoryManager::Sustain()
	{
		getSingleton().m_ResourceSustainer->Sustain();
	}

	void ArchetypeFactoryManager::EndSustain()
	{
		getSingleton().m_ResourceSustainer->EndSustain();
	}

	void ArchetypeFactoryManager::StoreFactory(const ResourceDataPtr& resource)
	{
		getSingleton().m_ResourceSustainer->StoreResource(resource);
	}

	SimpleResourceSustainer::SimpleResourceSustainer()
		: m_VolitileCapacity(s_DefaultVolatileCapacity)
	{
		m_Sustaining = false;
	}

	void SimpleResourceSustainer::Sustain()
	{
		m_Sustaining = true;
	}

	void SimpleResourceSustainer::EndSustain()
	{
		m_Sustaining = false;
		m_Sustained.clear();
	}

	void SimpleResourceSustainer::StoreResource(const ResourceDataPtr& resource)
	{
		if (m_Sustaining)
		{
			m_Sustained.push_back(resource);
		}
		m_Volatile.push_back(StoredResource(resource));
		if (m_Volatile.size() > m_VolitileCapacity)
			m_Volatile.pop_front();
	}

	SimpleResourceSustainer::StoredResource::StoredResource(const ResourceDataPtr& resource)
		: resource(resource)
	{
		using namespace std::placeholders;
		hotReloadConnection = resource->SigHotReloadEvents.connect(std::bind(&SimpleResourceSustainer::StoredResource::HotReloadEvents, this, _1, _2));
	}

	ArchetypeFactory::ArchetypeFactory()
		: m_ComponentInstantiator(nullptr),
		m_ResourceContainer(nullptr)
	{
	}

	ArchetypeFactory::ArchetypeFactory(EntityInstantiator* instantiator, ResourceContainer* resource)
		: m_ComponentInstantiator(instantiator),
		m_ResourceContainer(resource)
	{
	}

	ArchetypeFactory::~ArchetypeFactory()
	{
	}

	void ArchetypeFactory::Save(std::ostream& stream)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		IO::Streams::CellStreamWriter writer(&stream);
		writer.Write(m_Editable);

		EntitySerialisationUtils::SaveEntity(stream, m_ArchetypeData.Archetype, false, m_Editable ? EntitySerialisationUtils::EditableBinary : EntitySerialisationUtils::FastBinary);

		m_ArchetypeData.Profile->Save(stream, m_ArchetypeData.Archetype);
	}

	void ArchetypeFactory::Load(std::shared_ptr<std::istream> stream, ComponentFactory* factory, EntityManager* manager)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		IO::Streams::CellStreamReader reader(stream.get());
		reader.Read(m_Editable);

		// Load the archetype definition entity
		std::tie(m_ArchetypeData.Archetype, stream) = EntitySerialisationUtils::LoadEntityImmeadiate(std::move(stream), false, 0, m_Editable ? EntitySerialisationUtils::EditableBinary : EntitySerialisationUtils::FastBinary, factory, manager, m_ComponentInstantiator);
		//m_TypeName = m_ArchetypeData.Archetype->GetArchetype(); // get the type name
		
		// Load the profile
		m_ArchetypeData.Profile = std::make_shared<Archetypes::Profile>(m_TypeName);
		m_ArchetypeData.Profile->Load(*stream, m_ArchetypeData.Archetype);

		m_TypeName = m_ArchetypeData.Profile->GetName();
		m_ArchetypeData.Archetype->SetArchetype(m_TypeName);

		// Create a new definition agent
		m_ArchetypeData.Agent = std::make_shared<ArchetypeDefinitionAgent>(m_ArchetypeData.Archetype, m_ArchetypeData.Profile);
		m_ArchetypeData.Archetype->SetArchetypeDefinitionAgent(m_ArchetypeData.Agent);
	}

	EntityPtr ArchetypeFactory::MakeInstance(ComponentFactory* factory, const Vector2& pos, float angle) const
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		EntityPtr entity = makeInstance(factory, pos, angle);
		linkInstance(entity);

		return std::move(entity);
	}

	EntityPtr ArchetypeFactory::MakeUnlinkedInstance(ComponentFactory* factory, const Vector2& pos, float angle) const
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		return makeInstance(factory, pos, angle);
	}

	EntityPtr ArchetypeFactory::makeInstance(ComponentFactory* factory, const Vector2& pos, float angle) const
	{
		const auto& data = m_ArchetypeData;

		// Since archetypes aren't updated regularly, make sure the properties are up to date (so they can be cloned accurately)
		data.Archetype->SynchroniseParallelEdits();
		EntityPtr entity = data.Archetype->Clone(factory);

		entity->SetArchetype(m_TypeName);

		entity->SetPosition(pos);
		entity->SetAngle(angle);
		// Apply the position and angle values
		entity->SynchroniseParallelEdits();

		auto agent = std::make_shared<ArchetypalEntityManager>(entity, data.Profile, m_ComponentInstantiator, m_ResourceContainer != nullptr ? ResourceDataPtr(m_ResourceContainer) : ResourceDataPtr());
		entity->SetArchetypeAgent(agent);

		return std::move(entity);
	}

	void ArchetypeFactory::linkInstance(const EntityPtr& entity) const
	{
		const auto& data = m_ArchetypeData;

		// Set up signal handlers
		if (auto agent = std::dynamic_pointer_cast<ArchetypalEntityManager>(entity->GetArchetypeAgent()))
		{
			using namespace std::placeholders;
			agent->m_ChangeConnection = data.Agent->SignalChange.connect(std::bind(&ArchetypalEntityManager::OnSerialisedDataChanged, agent.get(), _1));
			agent->m_ComponentAddedConnection = data.Agent->SignalAddComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentAdded, agent.get(), _1, _2, _3));
			agent->m_ComponentRemovedConnection = data.Agent->SignalRemoveComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentRemoved, agent.get(), _1));
		}
	}

	void ArchetypeFactory::DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type, const EntityPtr& entity)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		m_TypeName = type;

		ArchetypeData& data = m_ArchetypeData;
		// Generate the archetype by cloning the given entity
		data.Archetype = entity->Clone(factory);
		data.Archetype->SetArchetype(m_TypeName);
		data.Archetype->SetName(m_TypeName);
		// Generate the type definition
		data.Profile = std::make_shared<Archetypes::Profile>(m_TypeName);
		data.Profile->Define(data.Archetype);
		// Create and apply the definition agent
		data.Agent = std::make_shared<ArchetypeDefinitionAgent>(data.Archetype, data.Profile);
		data.Archetype->SetArchetypeDefinitionAgent(data.Agent);

		// TODO: add archetypes to the entity manager or something to maintain them (stream in, sync props)
		data.Archetype->StreamIn();
	}

	void LoadArchetypeResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<ArchetypeFactory*>(resource->GetDataPtr());
		}

		ArchetypeFactory* factory;
		if (ArchetypeFactoryManager::GetInstantiator()) // provide stuff for modifying definitions in edit mode
			factory = new ArchetypeFactory(ArchetypeFactoryManager::GetInstantiator(), resource);
		else
			factory = new ArchetypeFactory();

		try
		{
			auto dev = fs.open_file(resource->GetPath(), clan::File::open_existing, clan::File::access_read);
			auto stream = std::make_shared<IO::CLStream>(dev);

			factory->Load(stream, ArchetypeFactoryManager::GetComponentFactory(), ArchetypeFactoryManager::GetEntityManager());

			// Add the metadata that will be used to detect changes to this resource
			resource->SetMetadata(CreateFileMetadata(resource->GetPath(), *stream));
		}
		catch (clan::Exception& ex)
		{
			delete factory;
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
		}
		catch (Exception&)
		{
			delete factory;
			throw;
		}
		resource->SetDataPtr(factory);

		ArchetypeFactoryManager::StoreFactory(resource);

		resource->setLoaded(true);
	}

	void UnloadArchetypeResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<ArchetypeFactory*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

}
