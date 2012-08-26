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
#include <ClanLib/vorbis.h>

#include <string>
#include <memory>

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
	static int main(const std::vector<CL_String> &args)
	{
		using namespace FusionEngine;

		// Init ClanLib
		CL_SetupCore setupCore;
		CL_SetupDisplay setupDisplay;
		CL_SetupGL setupGL;
		CL_SetupSound setupSound;
		CL_SetupVorbis setupVorbis;

		SetupPhysFS setupPhysfs(CL_System::get_exe_path().c_str());
		if (!SetupPhysFS::is_init())
			return 1;

		CL_ConsoleWindow conWindow("Console", 80, 10);

		try
		{
			EngineManager manager(args);

			manager.Initialise();
			
			manager.AddExtension(std::make_shared<Editor>(args));

			manager.AddSystem(std::unique_ptr<AngelScriptSystem>(new AngelScriptSystem(manager.GetScriptManager())));
			manager.AddSystem(std::unique_ptr<Box2DSystem>(new Box2DSystem));
			manager.AddSystem(std::unique_ptr<CLRenderSystem>(new CLRenderSystem(manager.GetDisplayWindow(), manager.GetCameraSynchroniser())));

			manager.Run();
		}
		catch (...)
		{
			std::cerr << "Unhandled exception";
			throw;
		}
		return 0;
	}
};

CL_ClanApplication app(&EntryPoint::main);
