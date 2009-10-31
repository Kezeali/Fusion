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

#include "Common.h"

#include "FusionEditor.h"

#include "FusionBoostSignals2.h"
#include "FusionRenderer.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionScriptingEngine.h"
#include "FusionGUI.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIODeviceProvider.h"
#include "FusionXml.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>


namespace FusionEngine
{

	Editor::Editor(InputManager *input, Renderer *renderer, StreamingManager *streaming_manager, EntityManager *ent_manager, GameMapLoader *map_util)
		: m_Input(input),
		m_Renderer(renderer),
		m_Streamer(streaming_manager),
		m_EntityManager(ent_manager),
		m_MapUtil(map_util)
	{
	}

	Editor::~Editor()
	{
	}

	const std::string s_EditorSystemName = "Editor";

	const std::string &Editor::GetName() const
	{
		return s_EditorSystemName;
	}

	bool Editor::Initialise()
	{
		Enable(false);

		m_Viewport.reset(new Viewport());
		m_Camera.reset( new Camera(ScriptingEngine::getSingleton().GetEnginePtr()) );
		m_Viewport->SetCamera(m_Camera);

		m_RawInputConnection = m_Input->SignalRawInput.connect( boost::bind(&Editor::OnRawInput, this, _1) );

		return true;
	}

	void Editor::CleanUp()
	{
	}

	void Editor::Update(float split)
	{
		if (m_Enabled)
		{
			const CL_Vec2f &currentPos = m_Camera->GetPosition();
			m_Camera->SetPosition(currentPos.x + m_CamVelocity.x, currentPos.y + m_CamVelocity.y);
			m_Camera->Update(split);
		}
	}

	void Editor::Draw()
	{
		m_Renderer->Draw(m_EntityManager->GetDomain(GAME_DOMAIN), m_Viewport, 0);
	}

	void Editor::Enable(bool enable)
	{
		if (enable)
		{
			//this->PushMessage(new SystemMessage(SystemMessage::PAUSE, "Entities"));
			this->PushMessage(new SystemMessage(SystemMessage::HIDE, "Entities"));

			m_EntityManager->SetDomainState(GAME_DOMAIN, DS_STREAMING);

			m_Streamer->SetPlayerCamera(255, m_Camera);

			this->PushMessage(new SystemMessage(SystemMessage::RESUME));
			this->PushMessage(new SystemMessage(SystemMessage::SHOW));
		}
		else
		{
			//this->PushMessage(new SystemMessage(SystemMessage::RESUME, "Entities"));
			this->PushMessage(new SystemMessage(SystemMessage::SHOW, "Entities"));

			m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);

			m_Streamer->RemovePlayerCamera(255);

			this->PushMessage(new SystemMessage(SystemMessage::PAUSE));
			this->PushMessage(new SystemMessage(SystemMessage::HIDE));
		}
		m_Enabled = enable;
	}

	void Editor::OnRawInput(const RawInput &ev)
	{
		if (ev.InputType == RawInput::Pointer)
		{
		}
		else if (ev.InputType == RawInput::Button)
		{
			if (ev.ButtonPressed == true)
			{
				if (ev.Code == CL_KEY_LEFT)
				{
					m_CamVelocity.x = -10;
				}
				if (ev.Code == CL_KEY_RIGHT)
				{
					m_CamVelocity.x = 10;
				}
				if (ev.Code == CL_KEY_UP)
				{
					m_CamVelocity.y = -10;
				}
				if (ev.Code == CL_KEY_DOWN)
				{
					m_CamVelocity.y = 10;
				}
				if (ev.Code == CL_KEY_S)
				{
					Save("test.xml");
				}
			}
			else if (ev.ButtonPressed == false)
			{
				if (ev.Code == CL_KEY_LEFT || ev.Code == CL_KEY_RIGHT)
				{
					m_CamVelocity.x = 0;
				}
				if (ev.Code == CL_KEY_UP || ev.Code == CL_KEY_DOWN)
				{
					m_CamVelocity.y = 0;
				}

				if (ev.Code == CL_MOUSE_LEFT)
				{
					Vector2 position((float)ev.PointerPosition.x, (float)ev.PointerPosition.y);
					CL_Rectf area;
					m_Renderer->CalculateScreenArea(area, m_Viewport, true);
					CreateEntity(area.left + position.x, area.top + position.y);
				}
			}
		}
	}

	void Editor::DisplayError(const std::string &title, const std::string &message)
	{
		SendToConsole(title + " error:" + message);
	}

	void Editor::CreateEntity(float x, float y)
	{
		GameMapLoader::GameMapEntity gmEntity;
		gmEntity.hasName = false;
		gmEntity.entity = m_EntityManager->GetFactory()->InstanceEntity("Test", "default");
		gmEntity.entity->SetPosition(Vector2(x, y));
		m_EntityManager->AddPseudoEntity(gmEntity.entity);
		m_Entities.push_back(gmEntity);
	}

	void Editor::StartEditor()
	{
		Enable();
	}

	void Editor::StopEditor()
	{
		Enable(false);
	}

	void Editor::Save(const std::string &filename)
	{
		CL_String dataFileName = CL_PathHelp::get_basename(fe_widen(filename)) + ".entdata";
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice out = vdir.open_file(dataFileName, CL_File::create_always, CL_File::access_write);

		serialiseEntityData(out);

		TiXmlDocument *doc = new TiXmlDocument();
		buildMapXml(doc);
		SaveXml_PhysFS(doc, fe_widen(filename));
		delete doc;

		m_CurrentFilename = filename;
	}

	void Editor::Load(const std::string &filename)
	{
		CL_String dataFileName = CL_PathHelp::get_basename(fe_widen(filename)) + ".entdata";
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice in = vdir.open_file(dataFileName, CL_File::open_existing, CL_File::access_read);

		IDTranslator translator = m_EntityManager->MakeIDTranslator();

		m_EntityManager->ClearDomain(GAME_DOMAIN);
		m_Entities.clear();

		SerialisedDataArray archetypes, entities;
		loadEntityData(in, archetypes, entities);

		TiXmlDocument *doc = OpenXml_PhysFS(fe_widen(filename));
		try
		{
			parseMapXml(doc, archetypes, entities, translator);
		}
		catch (FileTypeException &ex)
		{
			DisplayError(ex.GetName(), ex.GetDescription());
		}
		delete doc;

		initialiseEntities(entities, translator);

		m_CurrentFilename = filename;
	}

	void Editor::Save()
	{
		if (!m_CurrentFilename.empty())
			Save(m_CurrentFilename);
	}

	void Editor::Load()
	{
		if (!m_CurrentFilename.empty())
			Load(m_CurrentFilename);
	}

	void Editor::Compile(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice out = vdir.open_file(filename.c_str(), CL_File::create_always, CL_File::access_write);

		m_EntityManager->CompressIDs();

		Save();
		m_EntityManager->ClearDomain(GAME_DOMAIN);
		m_MapUtil->CompileMap(out, m_UsedTypes, m_Archetypes, m_Entities);
		Load();
	}

	void Editor::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);
		r = engine->RegisterObjectMethod("Editor",
			"void enable()",
			asMETHOD(Editor, Enable), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void startEditor()",
			asMETHOD(Editor, StartEditor), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void stopEditor()",
			asMETHOD(Editor, StopEditor), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void save(const string &in)",
			asMETHODPR(Editor, Save, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void load(const string &in)",
			asMETHODPR(Editor, Load, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void save()",
			asMETHOD(Editor, Save), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor",
			"void load()",
			asMETHOD(Editor, Load), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Editor",
			"void compile(const string &in)",
			asMETHODPR(Editor, Compile, (const std::string &), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	void Editor::spawnEntities()
	{
		for (GameMapLoader::GameMapEntityArray::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			it->entity->Spawn();
		}
	}

	void Editor::serialiseEntityData(CL_IODevice file)
	{
		// TODO:? the code that serialises entity data to a CL_IODevice could conceiveably be a public GameMapLoader method,
		//  since it already exists there, and this method would simply iterate over each entity and call that. However,
		//  since serialising/writing entity data is so simple I haven't at this time implemented that - much in the same
		//  way that you don't implement a class method for writing integers to std::cout.

		file.write_uint32(m_Archetypes.size());
		unsigned int dataIndex = 0;
		for (GameMapLoader::ArchetypeMap::iterator it = m_Archetypes.begin(), end = m_Archetypes.end(); it != end; ++it)
		{
			GameMapLoader::Archetype &archetype = it->second;
			// Update the data index in the editor's archetype info (this is written to the XML file)
			archetype.dataIndex = dataIndex++;

			// Write the state information
			file.write_uint32(archetype.packet.mask);
			file.write_string_a(archetype.packet.data.c_str());
		}

		file.write_uint32(m_Entities.size());
		SerialisedData state;
		for (GameMapLoader::GameMapEntityArray::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			const GameMapLoader::GameMapEntity &gmEntity = *it;
			if (gmEntity.archetypeId.empty())
			{
				state.mask = gmEntity.stateMask;
				gmEntity.entity->SerialiseState(state, true);

				file.write_uint32(state.mask);
				file.write_string_a(state.data.c_str());
			}
		}
	}

	void Editor::loadEntityData(CL_IODevice file, Editor::SerialisedDataArray &archetypes, Editor::SerialisedDataArray &entities)
	{
		SerialisedData state;

		unsigned int numArchetypes = file.read_uint32();
		archetypes.reserve(numArchetypes);
		for (unsigned int i = 0; i < numArchetypes; ++i)
		{
			state.mask = file.read_int32();
			state.data = file.read_string_a().c_str();
			archetypes.push_back(state);
		}

		unsigned int numEntityStates = file.read_uint32();
		entities.reserve(numEntityStates);
		for (unsigned int i = 0; i < numEntityStates; ++i)
		{
			state.mask = file.read_int32();
			state.data = file.read_string_a().c_str();
			entities.push_back(state);
		}
	}

	void Editor::buildMapXml(TiXmlDocument *document)
	{
		TiXmlDeclaration *decl = new TiXmlDeclaration( XML_STANDARD, "", "" );
		document->LinkEndChild(decl);

		TiXmlElement *root = new TiXmlElement("editor_data");
		document->LinkEndChild(root);

		//TiXmlElement *typesElm = new TiXmlElement("used_types");
		//root->LinkEndChild(typesElm);
		//TiXmlElement *elem;
		//for (StringSet::iterator it = m_UsedTypes.begin(), end = m_UsedTypes.end(); it != end; ++it)
		//{
		//	elem = new TiXmlElement("type");
		//	elem->SetAttribute("name", it->c_str());

		//	typesElm->LinkEndChild(elem);
		//}

		TiXmlElement *archetypesElm = new TiXmlElement("archetypes");
		root->LinkEndChild(archetypesElm);
		for (GameMapLoader::ArchetypeMap::iterator it = m_Archetypes.begin(), end = m_Archetypes.end(); it != end; ++it)
		{
			const GameMapLoader::Archetype &archetype = it->second;

			TiXmlElement *archetypeElm = new TiXmlElement("archetype");
			archetypeElm->SetAttribute("name", it->first);
			archetypeElm->SetAttribute("entity_type", archetype.entityTypename);
			archetypeElm->SetAttribute("data_index", archetype.dataIndex);
			archetypesElm->LinkEndChild(archetypeElm);
		}

		TiXmlElement *entities = new TiXmlElement("entities");
		root->LinkEndChild(entities);
		for (unsigned int i = 0; i < m_Entities.size(); ++i)
		{
			const GameMapLoader::GameMapEntity &gmEntity = m_Entities[i];

			TiXmlElement *entityElm = new TiXmlElement("entity");
			if (!gmEntity.entity->IsPseudoEntity())
				entityElm->SetAttribute("id", gmEntity.entity->GetID());
			if (gmEntity.hasName)
				entityElm->SetAttribute("name", gmEntity.entity->GetName());
			entityElm->SetAttribute("type", gmEntity.entity->GetType());
			if (!gmEntity.archetypeId.empty())
				entityElm->SetAttribute("archetype", gmEntity.archetypeId);
			entityElm->SetAttribute("data_index", i);
			entities->LinkEndChild(entityElm);

			TiXmlElement *transform = new TiXmlElement("location");
			std::ostringstream positionStr; positionStr << gmEntity.entity->GetPosition().x << "," << gmEntity.entity->GetPosition().y;
			transform->SetAttribute("position", positionStr.str());
			transform->SetAttribute("angle", boost::lexical_cast<std::string>(gmEntity.entity->GetAngle()));
			entityElm->LinkEndChild(transform);
		}
	}

	void Editor::parseMapXml(TiXmlDocument *document, const Editor::SerialisedDataArray &archetypes, const Editor::SerialisedDataArray &entities, const IDTranslator &translator)
	{
		TiXmlElement *root = document->FirstChildElement();
		if (root->ValueStr() != "editor_data")
			FSN_EXCEPT(ExCode::FileType, "Editor::parseMapXml", "The given file is not a map file");

		TiXmlElement *child = root->FirstChildElement();
		bool parsedArchetypes = false;
		while (child != NULL)
		{
			if (child->ValueStr() == "archetypes")
			{
				parse_Archetypes(child, archetypes);
				parsedArchetypes = true;
			}
			else if (child->ValueStr() == "entities")
			{
				if (parsedArchetypes)
					parse_Entities(child, entities.size(), translator);
				else
					FSN_EXCEPT(ExCode::FileType, "Editor::parseMapXml", "Map XML files must have the <archetypes> element before the <entities> element");
			}

			child = child->NextSiblingElement();
		}
	}

	bool getAttribute(std::string &out, TiXmlElement *element, const std::string &name, bool throw_on_failure = true)
	{
		const char *attribute = element->Attribute(name.c_str());
		if (attribute != NULL)
		{
			out = attribute;
			return true;
		}
		else if (throw_on_failure)
		{
			char name0 = std::tolower(name[0], std::locale());
			std::string indefinateArticle = (name0 == 'a' || name0 == 'e' || name0 == 'i' || name0 == 'o' || name0 == 'u') ? "a" : "an";
			std::string message = "<" + element->ValueStr() + "> elements must have " + indefinateArticle + " '" + name + "' attribute";
			FSN_EXCEPT(ExCode::FileType, "FusionEngine::getAttribute", message);
		}
		else
			return false;
	}

	std::string getAttribute(TiXmlElement *element, const std::string &name, bool throw_on_failure = true)
	{
		std::string value;
		getAttribute(value, element, name, throw_on_failure);
		return value;
	}

	void Editor::parse_Archetypes(TiXmlElement *element, const Editor::SerialisedDataArray &archetype_data)
	{
		TiXmlElement *child = element->FirstChildElement();
		std::string name;
		while (child != NULL)
		{
			getAttribute(name, child, "name");
			GameMapLoader::Archetype &archetype = m_Archetypes[name];

			getAttribute(archetype.entityTypename, child, "entity_type");

			const char *attribute = child->Attribute("data_index");
			if (attribute != NULL)
				archetype.dataIndex = boost::lexical_cast<unsigned int>(attribute);
			else
				archetype.dataIndex = archetype_data.size();

			if (archetype.dataIndex < archetype_data.size())
				archetype.packet = archetype_data[archetype.dataIndex];
			else
				SendToConsole("Reading map XML: The archetype '" + name + "' has an invalid index attribute");

			child = child->NextSiblingElement();
		}
	}

	template <typename T>
	void parse_vector(const std::string &value, T *x, T *y)
	{
		std::string::size_type d = value.find(",");
		if (d != std::string::npos)
		{
			std::string xstr = value.substr(0, d), ystr = value.substr(d+1);
			boost::trim(xstr); boost::trim(ystr);
			*x = boost::lexical_cast<T>(xstr);
			*y = boost::lexical_cast<T>(ystr);
		}
		else
		{
			*x = *y = boost::lexical_cast<T>(boost::trim_copy(value));
		}
	}

	void Editor::parse_Entities(TiXmlElement *element, unsigned int entity_count, const IDTranslator &translator)
	{
		EntityFactory *factory = m_EntityManager->GetFactory();

		TiXmlElement *child = element->FirstChildElement();
		std::string name, type, idStr;
		// Resize the entities array to construct enough GameMapEntity objects
		m_Entities.resize(entity_count);
		while (child != NULL)
		{
			unsigned int dataIndex = entity_count;
			{
				const char *attribute = child->Attribute("data_index");
				if (attribute != NULL)
					dataIndex = boost::lexical_cast<unsigned int>(attribute);
			}

			getAttribute(type, child, "type");
			getAttribute(idStr, child, "id", false);
			getAttribute(name, child, "name", false);

			if (dataIndex >= entity_count)
			{
				SendToConsole("Reading map XML: The entity '" + type + ":" + (name.empty() ? std::string("default") : name) + "' has an invalid index attribute");
				child = child->NextSiblingElement();
				continue;
			}

			GameMapLoader::GameMapEntity &gmEntity = m_Entities[dataIndex];

			if (name.empty())
			{
				name = "default";
				gmEntity.hasName = false;
			}

			gmEntity.entity = factory->InstanceEntity(type, name);
			if (gmEntity.entity)
			{
				getAttribute(gmEntity.archetypeId, child, "archetype", false);

				EntityPtr &entity = gmEntity.entity;

				TiXmlElement *location = child->FirstChildElement();
				if (location->ValueStr() == "location")
				{
					std::string attrValue; getAttribute(attrValue, location, "position");
					Vector2 position;
					parse_vector(attrValue, &position.x, &position.y);
					entity->SetPosition(position);

					getAttribute(attrValue, location, "angle");
					float angle = boost::lexical_cast<float>(attrValue);
				}

				ObjectID id = 0;
				if (!idStr.empty())
					id = boost::lexical_cast<ObjectID>(idStr);

				if (id == 0)
					m_EntityManager->AddPseudoEntity(entity);
				else
				{
					entity->SetID(translator(id));
					m_EntityManager->AddEntity(entity);
				}
			}

			child = child->NextSiblingElement();
		}
	}

	void Editor::initialiseEntities(const Editor::SerialisedDataArray &entity_data, const IDTranslator &translator)
	{
		EntityDeserialiser entityDeserialiser(m_EntityManager, translator);

		for (unsigned int i = 0; i < m_Entities.size(); ++i)
		{
			const GameMapLoader::GameMapEntity &gmEntity = m_Entities[i];
			// Initialise with archetype data
			if (!gmEntity.archetypeId.empty())
			{
				GameMapLoader::ArchetypeMap::iterator _where = m_Archetypes.find(gmEntity.archetypeId);
				if (_where != m_Archetypes.end() && _where->second.entityTypename == gmEntity.entity->GetType())
					gmEntity.entity->DeserialiseState(_where->second.packet, true, entityDeserialiser);
			}
			// Init. with specific entity data
			gmEntity.entity->DeserialiseState(entity_data[i], true, entityDeserialiser);
		}
	}

}
