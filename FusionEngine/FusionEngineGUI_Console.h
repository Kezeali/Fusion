/*
  Copyright (c) 2006 FusionTeam

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
*/

#ifndef Header_FusionEngine_GUI_Console
#define Header_FusionEngine_GUI_Console

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionEngineGUI.h"
#include "FusionInputHandler.h"

#include <CEGUI/CEGUI.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Wrapper for CEGUI - for "FusionEngine", the gameplay portion of fusion.
	 */
	class GUI_Console : public GUI
	{
	public:
		//! Basic constructor.
		GUI_Console();

	public:
		//! Inits the gui
		bool Initialise();

		//! Updates the inputs
		bool Update(unsigned int split);

		//! Draws the gui
		void Draw();

		//! Unbinds
		void CleanUp();

	};

}

#endif