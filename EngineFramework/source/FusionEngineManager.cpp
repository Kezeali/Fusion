/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionEngineManager.h"

#include "FusionCameraSynchroniser.h"
#include "FusionClientOptions.h"
#include "FusionComponentSystem.h"
#include "FusionEngineExtension.h"
#include "FusionLog.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionStreamingManager.h"

#include "FusionScriptManager.h"
#include "FusionRegionMapLoader.h"
#include "FusionStreamingManager.h"

#include <ClanLib/display.h>

namespace FusionEngine
{

	EngineManager::EngineManager(const std::vector<CL_String>& args)
		: m_EditMode(false),
		m_DisplayDimensions(800, 600),
		m_Fullscreen(false)
	{
		// Configure PhysFS
		SetupPhysFS::configure("lastflare", "Fusion", "7z");
		if (!SetupPhysFS::mount(s_PackagesPath, "", "7z", false))
			SendToConsole("Default resource path could not be located");
		SetupPhysFS::mount_archives(s_PackagesPath, "", "zip", false);

		// Clear cache
		SetupPhysFS::clear_temp();

		// Init Logger
		m_Logger.reset(new Logger);
	}

	void EngineManager::Initialise()
	{
		try
		{
			ClientOptions* options = new ClientOptions("settings.xml", "settings");

			ReadOptions(options);
			m_EditMode = options->GetOption_bool("edit");
			options->GetOption("screen_width", &m_DisplayDimensions.x);
			options->GetOption("screen_height", &m_DisplayDimensions.y);
			m_Fullscreen = options->GetOption_bool("fullscreen");

			// Get the general log file
			m_Log = Logger::getSingleton().OpenLog(g_LogGeneral);

			// Init the display window
			m_DisplayWindow = CL_DisplayWindow("Fusion", m_DisplayDimensions.x, m_DisplayDimensions.y, m_Fullscreen, !m_Fullscreen);

			// Init sound
			m_SoundOutput = CL_SoundOutput(44100);

			// Init ScriptManager
			m_ScriptManager = std::make_shared<ScriptManager>();

			// Init Streaming
			m_CellLoader.reset(new RegionMapLoader(m_EditMode));
			m_StreamingManager = std::make_shared<StreamingManager>(m_CellLoader.get());

			// Init Camera
			m_CameraSync = std::make_shared<CameraSynchroniser>(m_StreamingManager.get());
		}
		catch (std::exception& ex)
		{
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), (std::string("Failed to initialise engine: ") + ex.what()).c_str(), "Error", MB_OK);
#endif
		}
	}

	void EngineManager::ReadOptions(ClientOptions* options)
	{
		auto loggingLevel = options->GetOption_str("logging_level");
		if (loggingLevel.empty() || loggingLevel == "normal")
			m_Logger->SetDefaultThreshold(LOG_NORMAL);
		else if (loggingLevel == "info")
			m_Logger->SetDefaultThreshold(LOG_INFO);
		else if (loggingLevel == "trivial")
			m_Logger->SetDefaultThreshold(LOG_TRIVIAL);
		else if (loggingLevel == "critical")
			m_Logger->SetDefaultThreshold(LOG_CRITICAL);

		if (options->GetOption_bool("console_logging"))
			m_Logger->ActivateConsoleLogging();
	}

	const CL_GraphicContext& EngineManager::GetGC() const
	{
		return m_DisplayWindow.get_gc();
	}

	const CL_SoundOutput& EngineManager::GetSoundOutput() const
	{
		return m_SoundOutput;
	}

	const std::shared_ptr<ScriptManager>& EngineManager::GetScriptManager() const
	{
		return m_ScriptManager;
	}

	CameraSynchroniser* EngineManager::GetCameraSynchroniser() const
	{
		return m_CameraSync.get();
	}

	void EngineManager::AddExtension(const std::shared_ptr<EngineExtension>& extension)
	{
		m_Extensions.push_back(extension);
	}

	void EngineManager::AddSystem(std::unique_ptr<IComponentSystem>&& system)
	{
		const std::string name = system->GetName();
		m_Systems.insert(std::make_pair(name, std::move(system)));
	}

	void EngineManager::Run()
	{
		try
		{
			float time;
			float dt;
			while (true)
			{
				for (auto it = m_Extensions.begin(), end = m_Extensions.end(); it != end; ++it)
				{
					(*it)->Update(time, dt);
				}
			}

		}
		catch (Exception& ex)
		{
			m_Log->AddEntry("Unhandled exception: " + ex.ToString(), LOG_CRITICAL);
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), ("Unhandled exception: " + ex.ToString()).c_str(), "Error", MB_OK);
#endif
		}
		catch (std::exception& ex)
		{
			m_Log->AddEntry(std::string("Unhandled exception: ") + ex.what(), LOG_CRITICAL);
#ifdef _WIN32
			if (!m_DisplayWindow.is_null())
				MessageBoxA(m_DisplayWindow.get_hwnd(), (std::string("Unhandled exception: ") + ex.what()).c_str(), "Error", MB_OK);
#endif
		}
	}

}
