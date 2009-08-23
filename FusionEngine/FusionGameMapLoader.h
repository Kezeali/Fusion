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

#include "FusionCommon.h"

// Inherited
#include "FusionSingleton.h"

// Fusion
#include "FusionSerialisedData.h"


namespace FusionEngine
{

	class GameMapLoader
	{
	public:
		struct Archetype
		{
			std::string entityTypename;
			StringVector::size_type entityIndex;

			SerialisedData packet;
		};
		typedef std::vector<Archetype> ArchetypeArray;

		typedef std::tr1::unordered_map<std::string, Archetype> ArchetypeMap;

		struct GameMapEntity
		{
			EntityPtr entity;
			std::string archetypeId;

			unsigned int stateMask;
		};
		typedef std::vector<GameMapEntity> GameMapEntityArray;

		enum TypeFlags
		{
			NoTypeFlags = 0,
			ArchetypeFlag = 1 << 0,
			PseudoEntityFlag = 1 << 1
		};
	public:
		GameMapLoader(ClientOptions *options, EntityManager *manager);

		void LoadEntityTypes(CL_IODevice device);

		void LoadMap(CL_IODevice device);

		void LoadSavedGame(CL_IODevice device);
		void SaveGame(CL_IODevice device);

		//! Compiles a binary map file from map-editor data
		void CompileMap(CL_IODevice device, const StringSet &used_entity_types, const ArchetypeMap &archetypes, const GameMapEntityArray &entities);

	private:
		EntityManager *m_Manager;

		// The currently loaded map (must be written to the save game file so it can be re-loaded)
		std::string m_MapFilename;

		ClientOptions *m_ClientOptions;
	};

}

#endif
