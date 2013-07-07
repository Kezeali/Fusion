#include "PrecompiledHeaders.h"

#ifdef _DEBUG
#include <vld.h>
#endif

#include "FusionEngineManager.h"
#include "FusionEditor.h"

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>

#include <string>
#include <memory>
#include <thread>

#include "FusionPhysFS.h"

// Support
#include "FusionLogger.h"

// Components
#include "FusionAngelScriptSystem.h"
#include "FusionBox2DSystem.h"
#include "FusionCLRenderSystem.h"

class EntryPoint
{
public:
	static int main(const std::vector<std::string> &args)
	{
		using namespace FusionEngine;

		// Init ClanLib
		clan::SetupCore setupCore;
		clan::SetupDisplay setupDisplay;
		clan::SetupGL setupGL;
		clan::SetupSound setupSound;

		SetupPhysFS setupPhysfs(clan::System::get_exe_path().c_str());
		if (!SetupPhysFS::is_init())
			return 1;

		//clan::ConsoleWindow conWindow("Console", 80, 10);

		try
		{
			EngineManager manager(args);

			auto editor = std::make_shared<Editor>(args);
			manager.AddExtension(editor);

			manager.AddSystem(std::unique_ptr<AngelScriptSystem>(new AngelScriptSystem(manager.GetScriptManager())));
			manager.AddSystem(std::unique_ptr<Box2DSystem>(new Box2DSystem));
			manager.AddSystem(std::unique_ptr<CLRenderSystem>(new CLRenderSystem(manager.GetCanvas(), manager.GetCameraSynchroniser())));

			auto consoleConnection = Console::getSingleton().OnNewData.connect([](const std::string& data) {
					std::cout << data;
					std::cout << std::flush;
			});

			manager.Run();

			SendToConsole("EngineManager quit");
		}
		catch (std::exception& ex)
		{
			std::cout << "Unhandled exception: " << ex.what();
			throw;
		}

		return 0;
	}
};

clan::Application app(&EntryPoint::main);
