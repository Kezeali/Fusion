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

#ifndef H_FusionEditor
#define H_FusionEditor

#include "FusionPrerequisites.h"

#include "FusionEngineExtension.h"

#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <memory>

namespace FusionEngine
{

	class ISystemWorld;
	class AngelScriptWorld;

	class Editor : public EngineExtension
	{
	public:
		Editor(const std::vector<CL_String> &args);
		virtual ~Editor();

		void SetDisplay(const CL_DisplayWindow& display);

		void OnWorldCreated(const std::shared_ptr<ISystemWorld>& world);

		void SetAngelScriptWorld(const std::shared_ptr<AngelScriptWorld>& asw) { m_AngelScriptWorld = asw; }

		void Update(float time, float dt);

		std::vector<std::shared_ptr<RendererExtension>> MakeRendererExtensions() const;

	private:
		std::shared_ptr<Camera> m_EditCam;

		CL_DisplayWindow m_DisplayWindow;
		std::shared_ptr<AngelScriptWorld> m_AngelScriptWorld;

		CL_Slot m_KeyUpSlot;

		bool m_Rebuild;

		void onKeyUp(const CL_InputEvent& ev, const CL_InputState& state);

	};

}

#endif
