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

#ifndef H_FusionArchetypalEntityManager
#define H_FusionArchetypalEntityManager

#include "FusionPrerequisites.h"

#include "FusionTypes.h"
#include "FusionPropertySignalingSystem.h"
#include "FusionSynchronisedSignalingSystem.h"

#include <BitStream.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <map>
#include <unordered_map>

#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

namespace FusionEngine
{
	
	namespace Archetypes
	{
		typedef std::uint32_t PropertyID_t;
		typedef std::uint16_t ComponentID_t;

		class Profile;
	}

	class IInstanceAgent
	{
	public:
		virtual ~IInstanceAgent() {}

		virtual void SetManagedEntity(const EntityPtr& entity) = 0;

		virtual void AutoOverride(const std::string& name, bool enable) = 0;

		// TODO: make this take a property name (at least in an overload)
		virtual void OverrideProperty(Archetypes::PropertyID_t id, RakNet::BitStream& data) = 0;

		virtual void RemoveOverride(const std::string& property_name) = 0;

		virtual void Serialise(RakNet::BitStream& stream) = 0;
		virtual void Deserialise(RakNet::BitStream& stream) = 0;
	};

	class IDefinitionAgent
	{
	public:
		virtual ~IDefinitionAgent() {}

		virtual void Serialise(RakNet::BitStream& stream) = 0;
		virtual void Deserialise(RakNet::BitStream& stream) = 0;

		virtual void ComponentAdded(const ComponentPtr& component) = 0;
		virtual void ComponentRemoved(const ComponentPtr& component) = 0;

		virtual void PushState() = 0;
	};

	//! Transfers archetype changes to an entity
	class ArchetypalEntityManager : public IInstanceAgent
	{
	public:
		ArchetypalEntityManager(const std::shared_ptr<Archetypes::Profile>& definition);
		virtual ~ArchetypalEntityManager();

		void SetManagedEntity(const EntityPtr& entity);

		//! When true, an override will be created automatically next time the given instance property changes
		void AutoOverride(const std::string& name, bool enable);

		//! Used by instances to override definition properties
		void OverrideProperty(Archetypes::PropertyID_t id, RakNet::BitStream& data);

		void RemoveOverride(const std::string& property_name);

		//! Called on instances when the definition changes
		void OnComponentAdded(Archetypes::ComponentID_t arch_id, const std::string& type, const std::string& identifier);
		//! Called on instances when the definition changes
		void OnComponentRemoved(Archetypes::ComponentID_t arch_id);

		//! Called on instances when the definition changes
		void OnSerialisedDataChanged(RakNet::BitStream& data);

		//! Save the local overrides
		void Serialise(RakNet::BitStream& stream);
		//! Load the local overrides
		void Deserialise(RakNet::BitStream& stream);

	private:
		typedef std::map<Archetypes::PropertyID_t, std::unique_ptr<RakNet::BitStream>> ModifiedProperties_t;
		ModifiedProperties_t m_ModifiedProperties;
		std::map<Archetypes::ComponentID_t, IComponent*> m_Components;

		std::shared_ptr<Archetypes::Profile> m_Profile;

		std::weak_ptr<Entity> m_ManagedEntity;

		friend class ArchetypeFactory;
		boost::signals2::connection m_ChangeConnection;

		ComponentFactory* m_ComponentFactory;

		std::unordered_map<PropertyID, SyncSig::HandlerConnection_t> m_PropertyListenerConnections;

		std::set<Archetypes::ComponentID_t> m_AutoOverride;

		// Deserialises overriden properties
		void PerformPropertyOverrides();

		void AddPropertyListeners(const ComponentPtr& component);

		void OnInstancePropertyChanged(Archetypes::PropertyID_t id);

		// NOCOPEEE
		ArchetypalEntityManager(const ArchetypalEntityManager&) {}
		ArchetypalEntityManager& operator= (const ArchetypalEntityManager&) {}
	};

	class ArchetypeDefinitionAgent : public IDefinitionAgent
	{
	public:
		ArchetypeDefinitionAgent(const EntityPtr& entity, const std::shared_ptr<Archetypes::Profile>& profile, std::map<ComponentPtr, Archetypes::ComponentID_t> ids);
		virtual ~ArchetypeDefinitionAgent() {}

		//! Save the local overrides
		void Serialise(RakNet::BitStream& stream);
		//! Load the local overrides
		void Deserialise(RakNet::BitStream& stream);

		void ComponentAdded(const ComponentPtr& component);
		void ComponentRemoved(const ComponentPtr& component);

		void PushState();

		boost::signals2::signal<void (Archetypes::ComponentID_t, const std::string&, const std::string&)> SignalAddComponent;

		boost::signals2::signal<void (Archetypes::ComponentID_t)> SignalRemoveComponent;

		boost::signals2::signal<void (RakNet::BitStream&)> SignalChange;

	private:
		std::weak_ptr<Entity> m_DefinitionEntity;

		std::shared_ptr<Archetypes::Profile> m_Profile;

		// This seems dumb, and should probably be refactored (perhaps store the ID in the component itself?)
		std::map<ComponentPtr, Archetypes::ComponentID_t> m_ComponentIdMap;

		std::unordered_map<PropertyID, SyncSig::HandlerConnection_t> m_PropertyListenerConnections;

		void AddPropertyListeners(const ComponentPtr& component);
	};

}

#endif
