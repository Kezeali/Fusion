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

		CL_Rect GetBounds() const;

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
		GameMapLoader(ClientOptions *options);
		~GameMapLoader();

		void HandlePacket(RakNet::Packet *packet);

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
		std::shared_ptr<GameMap> LoadMap(const std::string &filename, const CL_VirtualDirectory &directory, InstancingSynchroniser* synchroniser = nullptr);

		std::shared_ptr<GameMap> LoadMap(const std::string &filename, InstancingSynchroniser* synchroniser = nullptr);

		void onEntityInstanced(EntityPtr &entity);

	private:
		EntityFactory *m_Factory;
		EntityManager *m_Manager;

		// The currently loaded map (must be written to the save game file so it can be re-loaded)
		std::string m_MapFilename;
		uint32_t m_MapChecksum;

		typedef boost::bimaps::bimap< boost::bimaps::unordered_set_of<std::string>, boost::bimaps::unordered_set_of<unsigned int> > TypeIndex;
		TypeIndex m_TypeIndex;
		unsigned int m_NextTypeIndex;

		boost::signals2::connection m_FactoryConnection;

		ClientOptions *m_ClientOptions;
	};

}

#endif
