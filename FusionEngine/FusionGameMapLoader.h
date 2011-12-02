/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionGameAreaLoader
#define H_FusionGameAreaLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/vector_of.hpp>
#include <boost/signals2.hpp>
#include <ClanLib/core.h>

#include "FusionEntityDeserialiser.h"
#include "FusionPacketHandler.h"
#include "FusionRefCounted.h"

namespace FusionEngine
{

	class Cell;
	class CellCache;

	namespace IO
	{
		class PhysFSDevice;
	}

	class GameMap
	{
	public:
		GameMap(CL_IODevice& file, const std::string& name);

		// TODO: remove this
		//void LoadCell(Cell* out, size_t index, bool include_synched, EntityFactory* factory, EntityManager* entityManager, InstancingSynchroniser* instantiator);

		//! Loads entities that aren't managed by the cell archiver
		void LoadNonStreamingEntities(bool include_synched, EntityManager* entityManager, EntityFactory* factory, InstancingSynchroniser* instantiator);
		// (To be) Used by CellArchiver to obtain static region data from a compiled map file 
		std::vector<char> GetRegionData(int32_t x, int32_t y, bool include_synched);

		static void CompileMap(std::ostream& device, float cell_size, CellCache* cache, const std::vector<EntityPtr>& nonStreamingEntities);

		const std::string& GetName() const { return m_Name; }

		float GetMapWidth() const;
		unsigned int GetNumCellsAcross() const;
		float GetCellSize() const;

		uint32_t GetNonStreamingEntitiesLocation() const { return m_NonStreamingEntitiesLocation; }

	private:
		std::string m_Name;
		CL_IODevice m_File;
		std::vector<std::pair<uint32_t, uint32_t>> m_CellLocations; // Locations within the file for each cell
		uint32_t m_NonStreamingEntitiesLocation;
		uint32_t m_NonStreamingEntitiesDataLength;
		unsigned int m_XCells;
		Vector2T<int32_t> m_MinCell;
		Vector2T<int32_t> m_MaxCell;
		Vector2T<uint32_t> m_NumCells;
		float m_CellSize;
		float m_MapWidth;
	};
	
	//! Loads maps and games.
	class GameMapLoader : public PacketHandler
	{
	public:
		struct Archetype
		{
			std::string entityTypename;
			StringVector::size_type entityIndex;

			unsigned int dataIndex; // used by the editor when writing data files (may be removed)

			//SerialisedData packet;
		};
		typedef std::vector<Archetype> ArchetypeArray;

		typedef std::tr1::unordered_map<std::string, Archetype> ArchetypeMap;

		class MapEntity : public RefCounted
		{
		public:
			EntityPtr entity;
			bool synced;
			bool hasName; // true if the entity has a unique name (other wise it will be called 'default')
			std::string archetypeId;

			unsigned int stateMask;

			unsigned int dataIndex; // used by the editor when reading data files (may be removed)

			MapEntity() : RefCounted(0), hasName(true), stateMask(0xFFFFFFFF) {}
			virtual ~MapEntity() {}
		};
		typedef boost::intrusive_ptr<MapEntity> MapEntityPtr;
		typedef std::vector<MapEntityPtr> MapEntityArray;

		enum TypeFlags
		{
			NoTypeFlags = 0,
			ArchetypeFlag = 1 << 0,
			PseudoEntityFlag = 1 << 1
		};
	public:
		GameMapLoader(ClientOptions *options);
		~GameMapLoader();

		void HandlePacket(RakNet::Packet *packet);

		//! Load entity types from the given map - call before compiling the script module
		void LoadEntityTypes(const std::string &filename, CL_VirtualDirectory &directory);
		void LoadEntityTypes(CL_IODevice &device);

		//! Loads the given map file
		/*!
		* \param[in] filename
		* The file to load
		* \param[in] directory
		* The filesystem to load the file from
		* \param[in] synchroniser
		* Load synced Entities (entities with an ID, that are synced over the network/saved.)
		* Set if starting a new game; if joining a game or loading a save, set to null
		* so synced entities can be loaded from the existing ontology.
		*/
		std::shared_ptr<GameMap> LoadMap(const std::string &filename, CL_VirtualDirectory &directory, InstancingSynchroniser* synchroniser = nullptr);

		std::shared_ptr<GameMap> LoadMap(const std::string &filename, InstancingSynchroniser* synchroniser = nullptr);

		void LoadSavedGame(const std::string &filename, CL_VirtualDirectory &directory, InstancingSynchroniser* synchroniser);
		void SaveGame(const std::string &filename, CL_VirtualDirectory &directory);

		//! Compiles a binary map file from map-editor data
		static void CompileMap(CL_IODevice &device, const StringSet &used_entity_types, const ArchetypeMap &archetypes, const MapEntityArray &pseudo_entities, const MapEntityArray &entities);
		
		//! Saves a single Entity to a data device
		/*!
		* \param[in] entity
		* The entity to save. After such a straightforward comment, here's where I'd put
		* a witty or subversive aside; that is, if I didn't I think too highly of you
		* to demeen the (albeit tenuous) relationship we have as comment-writer and
		* comment-reader. Err, oh dear.
		*
		* \param[in] device
		* The target device to write entity data to.
		*/
		static void SaveEntity(const EntityPtr &entity, CL_IODevice &device);
		//! Loads a single Entity from a data device
		/*!
		* The complementary function to SaveEntity.
		*
		* \param[in] factory
		* Factory to use to spawn the entity.
		* \param[in] manager
		* Manager with respect to which any Entity-pointer custom properties will be deserialised.
		* \param[in] device
		* The IO-device to load the entity data from.
		*
		* \return
		* The entity, if successful. nullptr otherwise
		*/
		static EntityPtr LoadEntity(CL_IODevice &device, EntityFactory *factory, const IEntityRepo *manager);

		void onEntityInstanced(EntityPtr &entity);

		static void Register(asIScriptEngine *engine);

	private:
		EntityFactory *m_Factory;
		EntityManager *m_Manager;

		std::shared_ptr<CL_VirtualFileSource> m_FileSource;

		// The currently loaded map (must be written to the save game file so it can be re-loaded)
		std::string m_MapFilename;
		uint32_t m_MapChecksum;

		typedef boost::bimaps::bimap< boost::bimaps::unordered_set_of<std::string>, boost::bimaps::unordered_set_of<unsigned int> > TypeIndex;
		TypeIndex m_TypeIndex;
		unsigned int m_NextTypeIndex;

		boost::signals2::connection m_FactoryConnection;

		ClientOptions *m_ClientOptions;

		void deserialiseBasicProperties(EntityPtr &entity, CL_IODevice &device);

		void loadPseudoEntities(CL_IODevice &device, const ArchetypeArray &archetypes, const IDTranslator &translator);
		void loadEntities(CL_IODevice &device, const ArchetypeArray &archetypes, const IDTranslator &translator, InstancingSynchroniser* synchroniser);
	};

}

#endif
