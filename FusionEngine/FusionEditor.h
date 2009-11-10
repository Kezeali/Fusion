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

#ifndef Header_FusionEngine_Editor
#define Header_FusionEngine_Editor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

#include "FusionViewport.h"
#include "FusionInputHandler.h"
#include "FusionGameMapLoader.h"

#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/ElementDocument.h>
#include <EMP/Core/DataSource.h>

#include <containers/structured_set.hpp>


namespace FusionEngine
{

	class EditorDataSource : public EMP::Core::DataSource
	{
	public:
		EditorDataSource();
		virtual ~EditorDataSource();

		void UpdateSuggestions(const StringVector &suggestions);
		const std::string & GetSuggestion(size_t index);

		void GetRow(EMP::Core::StringList& row, const EMP::Core::String& table, int row_index, const EMP::Core::StringList& columns);
		int GetNumRows(const EMP::Core::String& table);

	protected:
		StringVector m_Suggestions;
	};

	//! Editor system (runs the map editor interface)
	class Editor : public System, public Rocket::Core::EventListener
	{
	public:
		Editor(InputManager *input, Renderer *renderer, StreamingManager *streaming_manager, EntityManager *ent_manager, GameMapLoader *map_util);
		virtual ~Editor();

	public:
		const std::string &GetName() const;

		bool Initialise();

		void CleanUp();

		void Update(float split);

		void Draw();

		void Enable(bool enable = true);

		void OnRawInput(const RawInput &ev);

		void ProcessEvent(Rocket::Core::Event& ev);

		void DisplayError(const std::string &title, const std::string &message);

		void CreateEntity(const std::string &type, const std::string &name, bool pseudo, float x, float y);

		//! Returns entities beginning with...
		void LookUpEntityType(StringVector &results, const std::string &search_term);
		//! Updates the EditorDataSource with the results of the look up on the given name
		void LookUpEntityType(const std::string &search_term);

		const std::string &GetSuggestion(size_t index);

		void SetEntityType(const std::string &type);
		void SetEntityMode(bool pseudo);

		void StartEditor();
		void StopEditor();

		void Save(const std::string &filename);
		void Load(const std::string &filename);

		void Save();
		void Load();

		void Close();

		void Compile(const std::string &filename);

		static void Register(asIScriptEngine *engine);

	protected:
		Renderer *m_Renderer;
		StreamingManager *m_Streamer;
		InputManager *m_Input;
		EntityManager *m_EntityManager;
		GameMapLoader *m_MapUtil;

		boost::signals2::connection m_RawInputConnection;

		ViewportPtr m_Viewport;
		CameraPtr m_Camera;

		Vector2 m_CamVelocity;

		Rocket::Core::ElementDocument *m_Document;

		bool m_Enabled;

		std::string m_CurrentFilename;

		std::string m_CurrentEntityType;
		bool m_PseudoEntityMode;

		typedef containers::structured_set<std::string> EntityTypeLookupSet;
		EntityTypeLookupSet m_EntityTypes;

		EditorDataSource *m_EditorDataSource;

		// Map data (entities, etc.) - passed to map creator ("GameMapLoader")
		StringSet m_UsedTypes;
		GameMapLoader::ArchetypeMap m_Archetypes;
		GameMapLoader::GameMapEntityArray m_PseudoEntities;
		GameMapLoader::GameMapEntityArray m_Entities;

		//! Spawns all map entities (called when entering play mode)
		void spawnEntities();

		//! Serialises and saves the state data of all map entities to the given file
		void serialiseEntityData(CL_IODevice file);

		typedef std::vector<SerialisedData> SerialisedDataArray;
		//! Reads serialised entity data from the given file
		void loadEntityData(CL_IODevice file, SerialisedDataArray &archetypes, SerialisedDataArray &entities);

		//! Creates an XML document for the current map
		void buildMapXml(TiXmlDocument *document);
		//! Reads the given Map XML doc.
		void parseMapXml(TiXmlDocument *document, const SerialisedDataArray &archetypes, const SerialisedDataArray &entities, const IDTranslator &translator);

		//! Reads the given <archetypes> element from an XML map doc
		void parse_Archetypes(TiXmlElement *element, const SerialisedDataArray &archetype_data);
		//! Reads the given <entities> element from an XML map doc.
		void parse_Entities(TiXmlElement *element, unsigned int entity_count, const IDTranslator &translator);

		// Adds all entities to the EntityManager and deserialises their state data
		void initialiseEntities(const SerialisedDataArray &entity_data, const IDTranslator &translator);

	};

}

#endif
