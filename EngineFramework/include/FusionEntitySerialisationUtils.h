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

#ifndef H_FusionEntitySerialisationUtils
#define H_FusionEntitySerialisationUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

//#include <BitStream.h>
#include <ClanLib/core.h>

#include "FusionEntityComponent.h"
#include "FusionCellStreamTypes.h"

namespace RakNet
{
	class BitStream;
}

namespace FusionEngine
{

	class ArchetypeFactory;
	
	// One confusing thing here is that there are separate methods for serialising to a network stream vs to a file, even though the underlying methods
	//  for serialising individual components are the same. This should be rectified at some point, but luckily the methods are quite simple so this
	//  is still (generally) maintainable as it is.

	namespace EntitySerialisationUtils
	{
		enum SerialiseMode { Changes, All };

		// HumanReadable isn't implemented
		enum SerialisedDataStyle { FastBinary, EditableBinary, HumanReadable };

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

		void WriteComponent(OCellStream& outstr, EntityComponent* component, SerialisedDataStyle data_style);
		void ReadComponent(ICellStream& instr, EntityComponent* component, SerialisedDataStyle data_style);

		//! Merge inactive entity data
		std::streamsize MergeEntityData(ICellStream& in, OCellStream& out, RakNet::BitStream& incomming, RakNet::BitStream& incomming_occasional);

		//! Copy inactive entity data
		void CopyEntityData(ICellStream& in, OCellStream& out);

		//! In the future, there will be an ENTITY! IT IS THE ENTITY FUTURE.
		class EntityFuture
		{
		public:
			virtual ~EntityFuture() {}

			//! Waits for the value to be available, then returns it
			virtual EntityPtr get_entity() = 0;

			virtual std::shared_ptr<ICellStream> get_file() = 0;

			virtual std::pair<EntityPtr, std::shared_ptr<ICellStream>> get() = 0;

			//! Returns true if the return value is available
			virtual bool is_ready() const = 0;
		};

		//! Save an entity
		void SaveEntity(OCellStream& out, EntityPtr entity, bool id_included, SerialisedDataStyle data_style);
		//! Load an entity... eventually (checks for archetypes)
		std::shared_ptr<EntityFuture> LoadEntity(std::shared_ptr<ICellStream>&& in, bool id_included, ObjectID override_id, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser);
		//! Load an entity RIGHT NOW
		std::pair<EntityPtr, std::shared_ptr<ICellStream>> LoadEntityImmeadiate(std::shared_ptr<ICellStream>&& in, bool id_included, ObjectID override_id, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser);
		
		//! Load a non-archetypal entity
		EntityPtr LoadUniqueEntity(ICellStream& in, ObjectID id, PlayerID owner, const std::string& name, bool terrain, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser);
		//! Load an archetypal entity
		EntityPtr LoadArchetypalEntity(ICellStream& instr, ArchetypeFactory* archetype_factory, ObjectID id, PlayerID owner, const std::string& name, bool terrain, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator);
	}

}

#endif
