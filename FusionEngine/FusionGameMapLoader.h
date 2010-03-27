/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_GameAreaLoader
#define Header_FusionEngine_GameAreaLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/signals2.hpp>
#include <ClanLib/Core/IOData/iodevice.h>
#include <ClanLib/Core/IOData/virtual_directory.h>

#include "FusionEntityDeserialiser.h"
#include "FusionPacketHandler.h"
#include "FusionRefCounted.h"
#include "FusionSerialisedData.h"

namespace FusionEngine
{

	class GameMapLoader : public PacketHandler
	{
	public:
		struct Archetype
		{
			std::string entityTypename;
			StringVector::size_type entityIndex;

			unsigned int dataIndex; // used by the editor when writing data files (may be removed)

			SerialisedData packet;
		};
		typedef std::vector<Archetype> ArchetypeArray;

		typedef std::tr1::unordered_map<std::string, Archetype> ArchetypeMap;

		class GameMapEntity : public RefCounted
		{
		public:
			EntityPtr entity;
			bool synced;
			bool hasName; // true if the entity has a unique name (other wise it will be called 'default')
			std::string archetypeId;

			unsigned int stateMask;

			unsigned int dataIndex; // used by the editor when reading data files (may be removed)

			GameMapEntity() : RefCounted(0), hasName(true), stateMask(0xFFFFFFFF) {}
			virtual ~GameMapEntity() {}
		};
		typedef boost::intrusive_ptr<GameMapEntity> GameMapEntityPtr;
		typedef std::vector<GameMapEntityPtr> GameMapEntityArray;

		enum TypeFlags
		{
			NoTypeFlags = 0,
			ArchetypeFlag = 1 << 0,
			PseudoEntityFlag = 1 << 1
		};
	public:
		GameMapLoader(ClientOptions *options, EntityFactory *factory, EntityManager *manager);
		~GameMapLoader();

		void HandlePacket(Packet *packet);

		//! Load entity types from the given map - call before compiling the script module
		void LoadEntityTypes(const std::string &filename, CL_VirtualDirectory &directory);
		void LoadEntityTypes(CL_IODevice &device);

		//! Loads the given map file
		/*!
		* \param[in] filename
		* The file to load
		* \param[in] directory
		* The filesystem to load the file from
		* \param[in] include_synced_ents
		* Load synced Entities (entities with an ID, that are synced over the network/saved.)
		* Set to true if starting a new game; if joining a game or loading a save, set to false
		* so synced entities can be loaded from the existing ontology.
		*/
		void LoadMap(const std::string &filename, CL_VirtualDirectory &directory, bool include_synced_ents = true);

		void LoadSavedGame(const std::string &filename, CL_VirtualDirectory &directory);
		void SaveGame(const std::string &filename, CL_VirtualDirectory &directory);

		//! Compiles a binary map file from map-editor data
		static void CompileMap(CL_IODevice &device, const StringSet &used_entity_types, const ArchetypeMap &archetypes, const GameMapEntityArray &pseudo_entities, const GameMapEntityArray &entities);
		
		void onEntityInstanced(EntityPtr &entity);

	private:
		EntityFactory *m_Factory;
		EntityManager *m_Manager;

		// The currently loaded map (must be written to the save game file so it can be re-loaded)
		std::string m_MapFilename;

		typedef boost::bimaps::bimap< boost::bimaps::unordered_set_of<std::string>, boost::bimaps::unordered_set_of<unsigned int> > TypeIndex;
		TypeIndex m_TypeIndex;
		unsigned int m_NextTypeIndex;

		boost::signals2::connection m_FactoryConnection;

		ClientOptions *m_ClientOptions;

		void loadPseudoEntities(CL_IODevice &device, const ArchetypeArray &archetypes, const IDTranslator &translator);
		void loadEntities(CL_IODevice &device, const ArchetypeArray &archetypes, const IDTranslator &translator);
	};

}

#endif
