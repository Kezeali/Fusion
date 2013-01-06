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

//#include "PrecompiledHeaders.h"

#include "FusionEngineManagerRiffwrap.h"


#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>

#include "FusionEditorWrapscallion.h"
#include "FusionEditor.h"
#include "FusionEngineManager.h"
#include "FusionAngelScriptSystem.h"
#include "FusionBox2DSystem.h"
#include "FusionCLRenderSystem.h"
#include "FusionPhysFS.h"

namespace FusionEngine
{

	EngineManagerRiffwrap::EngineManagerRiffwrap(const std::vector<std::string>& args)
		: setupPhysFS(std::make_shared<SetupPhysFS>(clan::System::get_exe_path().c_str())),
		engineManager(std::make_shared<EngineManager>(args)),
		editor(std::make_shared<EditorWrapscallion>(std::make_shared<Editor>(args)))
	{
		engineManager->AddExtension(editor->editor);

		//engineManager->AddSystem(std::unique_ptr<AngelScriptSystem>(new AngelScriptSystem(engineManager->GetScriptManager())));
		//engineManager->AddSystem(std::unique_ptr<Box2DSystem>(new Box2DSystem));
		//engineManager->AddSystem(std::unique_ptr<CLRenderSystem>(new CLRenderSystem(manager.GetCanvas(), manager.GetCameraSynchroniser())));
	}

	EngineManagerRiffwrap::~EngineManagerRiffwrap()
	{
	}

	EditorWrapscallion* EngineManagerRiffwrap::GetEditor()
	{
		return editor.get();
	}

}
