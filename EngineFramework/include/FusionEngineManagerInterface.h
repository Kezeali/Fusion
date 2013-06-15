/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionEngineManagerInterface
#define H_FusionEngineManagerInterface

#include <Raknet/WindowsIncludes.h>

#include "FusionPrerequisites.h"

#include "DllConfig.h"

#include "FusionTypes.h"

//#include <ClanLib/display.h>
//#include <ClanLib/sound.h>

#include <memory>
#include <string>
#include <vector>

namespace FusionEngine
{

	namespace System
	{
		class ISystem;
	}
	class EngineExtension;

	//! EngineManager
	class FSN_DLL_API EngineManagerInterface
	{
	public:
		//! CTOR
		EngineManagerInterface(const std::vector<std::string>& args);
		//! DTOR
		virtual ~EngineManagerInterface() {}

		//const clan::DisplayWindow& GetDisplayWindow() const;
		//const clan::Canvas& GetCanvas() const;
		//const clan::GraphicContext& GetGC() const;
		//const clan::SoundOutput& GetSoundOutput() const;
		//const std::shared_ptr<ScriptManager>& GetScriptManager() const;
		//CameraSynchroniser* GetCameraSynchroniser() const;

		virtual void AddExtension(const std::shared_ptr<EngineExtension>& extension) = 0;

		virtual void AddSystem(std::unique_ptr<System::ISystem>&& system) = 0;

		virtual void Run() = 0;

		virtual void EnqueueSave(const std::string& name, bool quick = false) = 0;
		virtual void EnqueueLoad(const std::string& name) = 0;
	};

}

#endif
