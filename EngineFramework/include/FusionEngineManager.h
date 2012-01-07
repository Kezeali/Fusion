/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionEngineManager
#define H_FusionEngineManager

#include "FusionPrerequisites.h"

#include "FusionTypes.h"
#include "FusionVectorTypes.h"

#include <ClanLib/core.h> // For CL_String (bleh)
#include <ClanLib/display.h>
#include <ClanLib/sound.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace FusionEngine
{

	class EngineExtension;
	class IComponentSystem;
	class RegionMapLoader;

	void BootUp();

	class EngineManager
	{
	public:
		EngineManager(const std::vector<CL_String>& args);

		void Initialise();

		const CL_GraphicContext& GetGC() const;
		const CL_SoundOutput& GetSoundOutput() const;
		const std::shared_ptr<ScriptManager>& GetScriptManager() const;
		CameraSynchroniser* GetCameraSynchroniser() const;

		void AddExtension(const std::shared_ptr<EngineExtension>& extension);

		void AddSystem(std::unique_ptr<IComponentSystem>&& system);

		void Run();

	private:
		std::vector<std::shared_ptr<EngineExtension>> m_Extensions;

		std::map<std::string, std::unique_ptr<IComponentSystem>> m_Systems;

		std::unique_ptr<Logger> m_Logger;

		LogPtr m_Log;

		bool m_EditMode;

		Vector2i m_DisplayDimensions;
		bool m_Fullscreen;

		std::shared_ptr<ScriptManager> m_ScriptManager;
		std::shared_ptr<RegionMapLoader> m_CellLoader;
		std::shared_ptr<StreamingManager> m_StreamingManager;
		std::shared_ptr<CameraSynchroniser> m_CameraSync;

		CL_DisplayWindow m_DisplayWindow;
		CL_SoundOutput m_SoundOutput;

		void ReadOptions(ClientOptions* options);

	};

}

#endif
