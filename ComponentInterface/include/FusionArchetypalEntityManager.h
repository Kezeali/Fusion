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

#include <BitStream.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <map>

#include <boost/signals2/connection.hpp>

namespace FusionEngine
{
	
	namespace Archetypes
	{
		typedef std::uint32_t PropertyID_t;
		typedef std::uint16_t ComponentID_t;
	}

	class Archetype;

	//! Transfers archetype changes to an entity
	class ArchetypalEntityManager
	{
	public:
		ArchetypalEntityManager(const std::shared_ptr<Archetype>& definition);
		~ArchetypalEntityManager();

		void SetManagedEntity(const EntityPtr& entity);

		void OverrideProperty(Archetypes::PropertyID_t id, RakNet::BitStream& data);

		void OnComponentAdded(Archetypes::ComponentID_t arch_id, const std::string& type, const std::string& identifier);
		void OnComponentRemoved(Archetypes::ComponentID_t arch_id);

		void OnPropertyChanged(Archetypes::PropertyID_t id, RakNet::BitStream& data);
		void OnSerialisedDataChanged(RakNet::BitStream& data);

		void Serialise(RakNet::BitStream& stream);
		void Deserialise(RakNet::BitStream& stream);

	private:
		typedef std::map<Archetypes::PropertyID_t, std::unique_ptr<RakNet::BitStream>> ModifiedProperties_t;
		ModifiedProperties_t m_ModifiedProperties;
		std::map<Archetypes::ComponentID_t, IComponent*> m_Components;

		std::shared_ptr<Archetype> m_Definition;

		std::weak_ptr<Entity> m_ManagedEntity;

		friend class ArchetypeFactory;
		boost::signals2::connection m_ChangeConnection;

		ComponentFactory* m_ComponentFactory;

		// Deserialises overriden properties
		void PerformPropertyOverrides();

		// NOCOPEEE
		ArchetypalEntityManager(const ArchetypalEntityManager&) {}
		ArchetypalEntityManager& operator= (const ArchetypalEntityManager&) {}
	};

}

#endif
