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
#include "FusionWorldLoader.h"
#include "FusionPhysicsWorld.h"
#include "FusionInputHandler.h"
#include "FusionScriptingEngine.h"
#include "FusionClientOptions.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	const std::string s_OntologicalSystemName = "Entities";

	OntologicalSystem::OntologicalSystem(InputManager *input_manager)
		: m_InputManager(input_manager),
		m_EntityManager(NULL),
		m_GameLoader(NULL),
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
			m_EntityManager = new EntityManager();
			m_GameLoader = new GameAreaLoader(m_EntityManager);

			m_EntityManager->GetFactory()->SetScriptedEntityPath("Entities");

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

			std::string startupEntity;
			options.GetOption("startup_entity", &startupEntity);

			m_EntityManager->InstanceEntity(startupEntity, "startup");
		}

		return true;
	}

	void OntologicalSystem::CleanUp()
	{
		if (m_EntityManager != NULL)
		{
			delete m_GameLoader;
			delete m_EntityManager;

			delete m_PhysicsWorld;
		}
	}

	bool OntologicalSystem::Update(unsigned int split)
	{
		m_PhysicsWorld->RunSimulation(split);
		m_EntityManager->Update(split * 0.001f);
		return true;
	}

	void OntologicalSystem::Draw()
	{
		m_EntityManager->Draw();
	}

	void OntologicalSystem::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&OntologicalSystem::OnModuleRebuild, this, _1) );
	}

	void OntologicalSystem::OnModuleRebuild(BuildModuleEvent& ev)
	{
		ev.manager->RegisterGlobalObject("World@ world", m_PhysicsWorld);
	}

}
