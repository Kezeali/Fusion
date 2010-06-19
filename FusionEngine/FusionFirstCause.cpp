/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionFirstCause.h"

#include "FusionClientOptions.h"
#include "FusionEditor.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionFlagRegistry.h"
#include "FusionGameMapLoader.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionOntologicalSystem.h"
#include "FusionPhysicalEntityManager.h"
#include "FusionStateManager.h"

namespace FusionEngine
{

	FirstCause::FirstCause(ClientOptions *options, Renderer *renderer, InputManager *input_manager)
		: m_Options(options),
		m_Renderer(renderer),
		m_InputManager(input_manager),
		m_EntityFactory(nullptr),
		m_EntitySync(nullptr),
		m_Streaming(nullptr),
		m_EntityManager(nullptr),
		m_InstancingSync(nullptr),
		m_MapLoader(nullptr),
		m_PhysWorld(nullptr),
		m_InEditor(false)
	{
		m_EditorEnabled = m_Options->GetOption_str("editor") == "enabled";

		if (m_EditorEnabled)
		{
			Console::CommandHelp help;
			help.helpText = "Switches to the given engine mode.\nEnter switchto <name of mode>\nMode names: 'game', 'editor'\nSee Also: togglemode";
			help.argumentNames.push_back("mode");
			Console::CommandFunctions fncs; fncs.callback = boost::bind(&FirstCause::SwitchTo, this, _1);
			Console::getSingleton().BindCommand("switchto", fncs, help);
			help.helpText = "This is an alias of switchto.\n" + help.helpText;
			Console::getSingleton().BindCommand("start", fncs, help);

			help.helpText = "Toggles between Editor and game mode.";
			help.argumentNames.clear();
			fncs.callback = boost::bind(&FirstCause::ToggleMode, this, _1);
			Console::getSingleton().BindCommand("togglemode", fncs, help);
		}

		Console::getSingleton().BindCommand("streamingrange", boost::bind(&FirstCause::SetStreamingRange, this, _1));
	}

	template <class T>
	inline void checkedDelete(T*& obj)
	{
		if (obj != nullptr)
			delete obj;
		obj = nullptr;
	}

	FirstCause::~FirstCause()
	{
		checkedDelete(m_EntityFactory);
		checkedDelete(m_InstancingSync);
		checkedDelete(m_Streaming);
		checkedDelete(m_EntityManager);
		checkedDelete(m_MapLoader);
		checkedDelete(m_PhysWorld);
	}

	void FirstCause::Initialise(ModulePtr module)
	{
		// The static flag registry is used by renderables for quick filtering
		new StaticFlagRegistry();

		m_EntityFactory = new EntityFactory();
		m_EntitySync = new EntitySynchroniser(m_InputManager);
		m_Streaming = new StreamingManager();
		m_EntityManager = new EntityManager(m_InputManager, m_EntitySync, m_Streaming);
		m_InstancingSync = new InstancingSynchroniser(m_EntityFactory, m_EntityManager);
		m_MapLoader = new GameMapLoader(m_Options, m_EntityFactory, m_EntityManager);

		m_PhysWorld = new PhysicalWorld();
		m_PhysWorld->SetGraphicContext(m_Renderer->GetGraphicContext());

		m_Streaming->SetRange(2000);

		ScriptManager *manager = ScriptManager::getSingletonPtr();

		manager->RegisterGlobalObject("StreamingManager streamer", m_Streaming);
		manager->RegisterGlobalObject("EntityManager entity_manager", m_EntityManager);
		
		m_EntityFactory->SetScriptingManager(manager);
		m_EntityFactory->SetModule(module);
		m_EntityFactory->SetScriptedEntityPath("Entities/");

		m_Ontology.reset(new OntologicalSystem(m_Renderer, m_InstancingSync, m_PhysWorld, m_Streaming, m_MapLoader, m_EntityManager));
		if (m_EditorEnabled)
		{
			m_Editor.reset(new Editor(m_InputManager, m_EntityFactory, m_Renderer, m_InstancingSync, m_PhysWorld, m_Streaming, m_MapLoader, m_EntityManager));
			manager->RegisterGlobalObject("Editor editor", m_Editor.get());

			m_Editor->SetEntityModule(module);
			// Load all entity types so they can be used in the editor
			m_EntityFactory->LoadAllScriptedTypes();
		}

		m_Ontology->SetModule(module);
	}

	void FirstCause::BeginExistence(SystemsManager *system_manager)
	{

		system_manager->AddSystem(m_Ontology);
		if (m_EditorEnabled)
		{
			system_manager->AddSystem(m_Editor);
			m_Ontology->Stop();
			m_Editor->Start();
		}
		else
			m_Ontology->Start();
	}

	void FirstCause::SwitchToEditor()
	{
		m_InEditor = true;
		m_Ontology->Clear();
		m_Ontology->Stop();
		m_Editor->Start();
	}

	void FirstCause::SwitchToGame()
	{
		m_InEditor = false;
		m_Editor->Stop();
		m_Ontology->Start();
	}

	//void FirstCause::Test()
	//{
	//	if (m_InEditor)
	//	{
	//		//m_Editor->SpawnEntities(m_EntityManager);
	//		//m_Editor->Deactivate();
	//		//m_Ontology->Activate();
	//	}
	//}

	std::string FirstCause::SwitchTo(const StringVector &args)
	{
		if (args.size() < 2)
			return "This command requires a parameter ('editor' or 'game').";

		if (args[1] == "editor")
			SwitchToEditor();
		else if (args[1] == "game")
			SwitchToGame();
		else
			return "This command requires 'editor' or 'game' as the first parameter.";

		return "Switched to " + args[1] + ".";
	}

	std::string FirstCause::ToggleMode(const StringVector &args)
	{
		if (m_InEditor)
		{
			SwitchToGame();
			return "Switched to game";
		}
		else
		{
			SwitchToEditor();
			return "Switched to editor";
		}
	}
	
	std::string FirstCause::SetStreamingRange(const StringVector &args)
	{
		if (args.size() < 2)
			return boost::lexical_cast<std::string>( m_Streaming->GetRange() );

		m_Streaming->SetRange( boost::lexical_cast<float>(args[1]) );
		return "";
	}

}
