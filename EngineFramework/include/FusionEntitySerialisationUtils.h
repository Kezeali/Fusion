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

#ifndef H_FusionEntitySerialisationUtils
#define H_FusionEntitySerialisationUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

//#include <BitStream.h>
#include <ClanLib/core.h>

#include "FusionEntityComponent.h"

namespace RakNet
{
	class BitStream;
}

namespace FusionEngine
{

	typedef std::basic_istream<char> ICellStream;
	typedef std::basic_ostream<char> OCellStream;

	class ArchetypeFactory;
	
	namespace EntitySerialisationUtils
	{
		enum SerialiseMode { Changes, All };

		bool SerialiseEntity(RakNet::BitStream& out, const EntityPtr& entity, SerialiseMode mode);
		EntityPtr DeserialiseEntity(RakNet::BitStream& in, ComponentFactory* factory, EntityManager* manager);

		bool SerialiseContinuous(RakNet::BitStream& out, const EntityPtr& entity, SerialiseMode mode);
		void DeserialiseContinuous(RakNet::BitStream& in, const EntityPtr& entity, SerialiseMode mode);

		bool SerialiseOccasional(RakNet::BitStream& out, std::vector<uint32_t>& checksums, const EntityPtr& entity, SerialiseMode mode);
		void DeserialiseOccasional(RakNet::BitStream& in, std::vector<uint32_t>& checksums, const EntityPtr& entity, SerialiseMode mode);

		static void DeserialiseOccasional(RakNet::BitStream& in, const EntityPtr& entity, SerialiseMode mode)
		{
			std::vector<uint32_t> notUsed;
			DeserialiseOccasional(in, notUsed, entity, mode);
		}

		//! Deserialises and returns the position data from the given state, if it contains such data
		std::pair<bool, Vector2> DeserialisePosition(RakNet::BitStream& in, const Vector2& origin, const float radius);

		//! Writes the length of the stream, then the stream itself
		bool WriteStateWithLength(RakNet::BitStream& out, RakNet::BitStream& state);
		//! Reads length data written by WriteStateWithLength()
		RakNet::BitSize_t ReadStateLength(RakNet::BitStream& in);

		void WriteComponent(OCellStream& out, IComponent* component, bool editable);
		void ReadComponent(ICellStream& in, IComponent* component, bool editable);

		//! Merge inactive entity data
		std::streamsize MergeEntityData(ICellStream& in, OCellStream& out, RakNet::BitStream& incomming, RakNet::BitStream& incomming_occasional);

		//! Copy inactive entity data
		void CopyEntityData(ICellStream& in, OCellStream& out);

		class EntityFuture
		{
		public:
			virtual ~EntityFuture() {}

			virtual EntityPtr get() = 0;
		};

		//! Save an entity
		void SaveEntity(OCellStream& out, EntityPtr entity, bool id_included, bool editable);
		//! Load an entity (checks for archetypes)
		EntityFuture LoadEntity(ICellStream& in, bool id_included, ObjectID override_id, bool editable, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser);
		
		//! Load a non-archetypal entity
		EntityPtr LoadUniqueEntity(ICellStream& in, ObjectID id, PlayerID owner, const std::string& name, bool terrain, bool editable, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser);
		//! Load an archetypal entity
		EntityPtr LoadArchetypalEntity(ICellStream& instr, const std::string& archetype_id, ObjectID id, PlayerID owner, const std::string& name, bool terrain, bool editable, ComponentFactory* factory, ArchetypeFactory* archetype_factory, EntityManager* manager, EntityInstantiator* instantiator);
	}

}

#endif
