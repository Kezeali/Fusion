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

// Class
#include "FusionOntologicalSystem.h"

// Fusion
#include "FusionEntityManager.h"
#include "FusionEntityFactory.h"
#include "FusionGameMapLoader.h"
#include "FusionPhysicsWorld.h"
#include "FusionInputHandler.h"
#include "FusionScriptingEngine.h"
#include "FusionClientOptions.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIODeviceProvider.h"
#include "FusionXml.h"
#include "FusionPlayerRegistry.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	const std::string s_OntologicalSystemName = "Entities";

	OntologicalSystem::OntologicalSystem(Renderer *renderer, InputManager *input_manager, Network *network)
		: m_Renderer(renderer),
		m_InputManager(input_manager),
		m_Network(network),
		m_EntityManager(NULL),
		m_MapLoader(NULL),
		m_PhysicsWorld(NULL)
	{
	}

	OntologicalSystem::~OntologicalSystem()
	{
		CleanUp();
	}

	const std::string &OntologicalSystem::GetName() const
	{
		return s_OntologicalSystemName;
	}

	bool OntologicalSystem::Initialise()
	{
		if (m_EntityManager == NULL)
		{
			ViewportPtr viewport = m_Renderer->CreateViewport(Renderer::ViewFull);
			AddViewport(viewport);

			PlayerRegistry::AddPlayer(1, 0, NetHandle());
			PlayerRegistry::SetArbitrator(1);

			m_EntitySyncroniser = new EntitySynchroniser(m_InputManager, m_Network);
			m_EntityManager = new EntityManager(m_Renderer, m_InputManager, m_EntitySyncroniser);
			m_MapLoader = new GameMapLoader(m_EntityManager);

			ScriptingEngine *manager = ScriptingEngine::getSingletonPtr();

			manager->RegisterGlobalObject("EntityManager entity_manager", m_EntityManager);

			m_EntityManager->GetFactory()->SetScriptingManager(manager, "main");
			m_EntityManager->GetFactory()->SetScriptedEntityPath("Entities/");

			ClientOptions options(L"gameconfig.xml", "gameconfig");

			std::string worldSizeString;
			options.GetOption("world_size", &worldSizeString);
			StringVector components = fe_splitstring(worldSizeString, ",");

			float worldX = 1000.f, worldY = 1000.f;
			if (components.size() >= 2)
			{
				worldX = boost::lexical_cast<float>(fe_trim(components[0]));
				worldY = boost::lexical_cast<float>(fe_trim(components[1]));
			}
			m_PhysicsWorld = new PhysicsWorld(worldX, worldY);

			options.GetOption("startup_entity", &m_StartupEntity);
		}

		m_EntityManager->GetFactory()->LoadScriptedType(m_StartupEntity);

		// Load map entitites
		std::string maps, line;
		OpenString_PhysFS(maps, L"maps.txt"); // maps.txt contains a list of map files used by the game, seperated by newlines
		std::string::size_type lineBegin = 0, lineEnd;
		while (true)
		{
			if (lineBegin >= maps.length())
				break;

			lineEnd = maps.find("\n", lineBegin);
			if (lineEnd == std::string::npos)
				break;

			if (lineEnd != lineBegin+1) // ignore empty lines
			{
				line = fe_trim( maps.substr(lineBegin, lineEnd-lineBegin) ); // Get the line, trimming out the crap
				// Open the map file
				try
				{
					CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
					CL_IODevice mapFile = dir.open_file(fe_widen(line), CL_File::open_existing, CL_File::access_read);
					// Load entity types into the factory (the map loader has an EntityManager pointer
					//  from which it can get an EntityFactory pointer, on which it calls LoadScriptedType()
					//  for each entity found in the mapFile)
					m_MapLoader->LoadEntityTypes(mapFile);
				}
				catch (CL_Exception &)
				{
					SendToConsole("MapLoader - Reading maps.txt: Couldn't load " + line + ", file doesn't exist or can't be read.");
				}
			}

			lineBegin = lineEnd+1;
		}

		return true;
	}

	//PHYSFS_File *file = PHYSFS_openRead();
		//std::string buffer(256);
		//PHYSFS_sint64 count;
		//std::string currentLine
		//do
		//{
		//	count = PHYSFS_read(file, (void*)&buffer[0], 1, 256);
		//	std::string::size_type pos = 0;
		//	while (pos != std::string::npos)
		//	{
		//		pos = buffer.find("\n", pos);
		//	}
		//} while (count >= 256)

	void OntologicalSystem::CleanUp()
	{
		if (m_EntityManager != NULL)
		{
			delete m_MapLoader;
			delete m_EntityManager;
			delete m_EntitySyncroniser;

			delete m_PhysicsWorld;
		}
	}

	void OntologicalSystem::Update(float split)
	{
		m_PhysicsWorld->RunSimulation(split);
		m_EntityManager->Update(split);
		m_Renderer->Update(split);

		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			ViewportPtr &viewport = *it;
			viewport->GetCamera()->Update(split);
		}
	}

	void OntologicalSystem::Draw()
	{
		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			m_Renderer->Draw(*it);
		}
		m_EntityManager->Draw();
	}

	void OntologicalSystem::AddViewport(ViewportPtr viewport)
	{
		m_Viewports.push_back(viewport);
	}

	void OntologicalSystem::RemoveViewport(const ViewportPtr &viewport)
	{
		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			if (*it == viewport)
			{
				m_Viewports.erase(it);
				break;
			}
		}
	}

	void OntologicalSystem::RemoveAllViewports()
	{
		m_Viewports.clear();
	}

	OntologicalSystem::ViewportArray &OntologicalSystem::GetViewports()
	{
		return m_Viewports;
	}

	void OntologicalSystem::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&OntologicalSystem::OnModuleRebuild, this, _1) );
	}

	void OntologicalSystem::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			ev.manager->RegisterGlobalObject("World world", m_PhysicsWorld);
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			EntityPtr entity = m_EntityManager->InstanceEntity(m_StartupEntity, "startup", 1);

			if (entity)
			{
				//entity->Spawn();
				// Force stream-in
				entity->StreamIn();

				CameraPtr camera(new Camera(entity));
				m_Viewports.front()->SetCamera(camera);
			}
			else
			{
				m_Viewports.front()->SetCamera( CameraPtr(new Camera()) );
			}
		}
	}

}
