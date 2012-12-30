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

	class ArchetypeFactory;

	class Cell;
	class CellDataSource;

	class CellFileManager;

	class VirtualFilesystem
	{
	public:
		virtual ~VirtualFilesystem() {}
		virtual std::shared_ptr<std::istream> OpenFileForReading(const std::string& path) const = 0;
		virtual std::shared_ptr<std::ostream> OpenFileForWriting(const std::string& path) const = 0;
		virtual void CreateFolder(const std::string& path) const = 0;
		virtual void Delete(const std::string& path) const = 0;
	};

	class GameMap
	{
	public:
		GameMap(const std::string& path);

		void InitInstantiator(EntityInstantiator* instantiator);

		//! Loads entities that aren't managed by the cell archiver
		void LoadNonStreamingEntities(bool include_synched, EntityManager* entityManager, ComponentFactory* factory, ArchetypeFactory* archetype_factory, EntityInstantiator* instantiator);

		static void CompileMap(const VirtualFilesystem& vfs, const std::string& map_name, float cell_size, CellFileManager* cache, const std::vector<EntityPtr>& nonStreamingEntities, EntityInstantiator* instantiator);

		std::string GetName() const { return m_Name; }

		std::string GetPath() const { return m_Path; }

		std::string GetMetadataPath() const { return GetPath() + "/" + GetName() + metadataExtension; }

		std::string GetInstantiatorStatePath() const { return GetPath() + "/" + instantiatorStateFilename; }

		std::string GetTentDataPath() const { return GetPath() + "/" + tentDataFilename; }

		std::string GetEntityDatabasePath() const { return GetPath() + "/" + entityDatabaseFilename; }

		float GetCellSize() const;

	private:
		std::string m_Path;
		std::string m_Name;
		float m_CellSize;

		static const std::string metadataExtension;
		static const std::string instantiatorStateFilename;
		static const std::string tentDataFilename;
		static const std::string entityDatabaseFilename;

		void ReadMetadata();
	};
	
	//! Loads maps and games.
	class GameMapLoader : public PacketHandler
	{
	public:
		GameMapLoader(/*ClientOptions *options*/);
		~GameMapLoader();

		void HandlePacket(RakNet::Packet *packet);

		//! Loads the given map file
		/*!
		* \param[in] filename
		* The file to load
		*/
		std::shared_ptr<GameMap> LoadMap(const std::string &filename, bool synchronise = false);

		void onEntityInstanced(EntityPtr &entity);

	private:
		ComponentFactory *m_Factory;
		EntityManager *m_Manager;

		// The currently loaded map (must be written to the save game file so it can be re-loaded)
		std::string m_MapFilename;
		uint32_t m_MapChecksum;

		typedef boost::bimaps::bimap< boost::bimaps::unordered_set_of<std::string>, boost::bimaps::unordered_set_of<unsigned int> > TypeIndex;
		TypeIndex m_TypeIndex;
		unsigned int m_NextTypeIndex;

		boost::signals2::connection m_FactoryConnection;

		//ClientOptions *m_ClientOptions;
	};

}

#endif
