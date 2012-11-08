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
#include "FusionSaveDataArchive.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <tbb/atomic.h>

namespace bio = boost::iostreams;

namespace FusionEngine
{

	static const size_t s_DefaultVolatileCapacity = 100;

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

		void SetVolatileCapacity(size_t capacity) { m_VolitileCapacity = capacity; }
		size_t GetVolatileCapacity() const { return m_VolitileCapacity; }

		//! Adds a resource to the cache
		void StoreResource(const ResourceDataPtr& resource);

	private:
		std::list<ResourceDataPtr> m_Volatile;
		std::list<ResourceDataPtr> m_Sustained;

		tbb::atomic<bool> m_Sustaining;
		size_t m_VolitileCapacity;
	};

	ArchetypeFactoryManager::ArchetypeFactoryManager()
		: m_ResourceSustainer(new SimpleResourceSustainer())
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
		m_Volatile.push_back(resource);
		if (m_Volatile.size() > m_VolitileCapacity)
			m_Volatile.pop_front();
	}

	ArchetypeFactory::ArchetypeFactory(EntityInstantiator* instantiator)
		: m_ComponentInstantiator(instantiator)
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

		EntitySerialisationUtils::SaveEntity(stream, m_ArchetypeData.Archetype, false, m_Editable);

		m_ArchetypeData.Profile->Save(stream);
	}

	void ArchetypeFactory::Load(std::istream& stream)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		IO::Streams::CellStreamReader reader(&stream);
		reader.Read(m_Editable);

		// Load the archetype definition entity
		m_ArchetypeData.Archetype = EntitySerialisationUtils::LoadEntityImmeadiate(stream, false, 0, m_Editable, nullptr, nullptr, m_ComponentInstantiator);
		m_TypeName = m_ArchetypeData.Archetype->GetArchetype(); // get the type name
		// Load the profile
		m_ArchetypeData.Profile = std::make_shared<Archetypes::Profile>(m_TypeName);
		m_ArchetypeData.Profile->Load(stream);
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

		return std::move(entity);
	}

	void ArchetypeFactory::linkInstance(const EntityPtr& entity) const
	{
		const auto& data = m_ArchetypeData;

		auto agent = std::make_shared<ArchetypalEntityManager>(entity, data.Profile, m_ComponentInstantiator);
		// Set up signal handlers
		{
			using namespace std::placeholders;
			agent->m_ChangeConnection = data.Agent->SignalChange.connect(std::bind(&ArchetypalEntityManager::OnSerialisedDataChanged, agent.get(), _1));
			agent->m_ComponentAddedConnection = data.Agent->SignalAddComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentAdded, agent.get(), _1, _2, _3));
			agent->m_ComponentRemovedConnection = data.Agent->SignalRemoveComponent.connect(std::bind(&ArchetypalEntityManager::OnComponentRemoved, agent.get(), _1));
		}
		entity->SetArchetypeAgent(agent);
	}

	void ArchetypeFactory::DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type, const EntityPtr& entity)
	{
		boost::mutex::scoped_lock lock(m_Mutex);

		m_TypeName = type;

		ArchetypeData& data = m_ArchetypeData;
		// Generate the archetype by cloning the given entity
		data.Archetype = entity->Clone(factory);
		data.Archetype->SetArchetype(m_TypeName);
		// Generate the type definition
		data.Profile = std::make_shared<Archetypes::Profile>(m_TypeName);
		auto componentIds = data.Profile->Define(data.Archetype);
		// Create and apply the definition agent
		data.Agent = std::make_shared<ArchetypeDefinitionAgent>(data.Archetype, data.Profile, std::move(componentIds));
		data.Archetype->SetArchetypeDefinitionAgent(data.Agent);

		// TODO: add archetypes to the entity manager or something to maintain them
		data.Archetype->StreamIn();
	}

	void LoadArchetypeResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			delete static_cast<ArchetypeFactory*>(resource->GetDataPtr());
		}

		ArchetypeFactory* factory = new ArchetypeFactory(nullptr);
		try
		{
			auto dev = vdir.open_file(resource->GetPath(), CL_File::open_existing, CL_File::access_read);
			factory->Load(IO::CLStream(dev));
		}
		catch (CL_Exception& ex)
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

		ArchetypeFactoryManager::getSingleton().StoreFactory(resource);

		resource->setLoaded(true);
	}

	void UnloadArchetypeResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<ArchetypeFactory*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

}
