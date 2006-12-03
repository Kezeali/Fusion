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

#ifndef Header_FusionEngine_GUI
#define Header_FusionEngine_GUI

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Inherited
#include "FusionState.h"

/// CEGUI
#include <CEGUI/CEGUI.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Wrapper for CEGUI - for "FusionEngine", the gameplay portion of fusion.
	 */
	class GUI : public FusionState
	{
	public:
		//! Basic constructor.
		GUI();

		/*!
		* \brief
		* Constructor.
		*/
		GUI(CEGUI::Renderer *renderer);

	protected:
		//! The default scheme file to load
		static const std::string DefaultScheme;
		//! The default layout file to load
		static const std::string DefaultLayout;

	public:
		//! Inits the gui
		virtual bool Initialise();

		//! Updates the inputs
		virtual bool Update(unsigned int split);

		//! Draws the gui
		virtual void Draw();

		//! Unbinds
		virtual void CleanUp();

	protected:
		//! Name of the config file for the skin
		std::string m_CurrentScheme;
		//! Name of the config file for the current gui page
		std::string m_CurrentLayout;

		//! Holds events
		CL_SlotContainer m_Slots;

	public:
		//! Tells CEGUI when a mouse button is pressed
		virtual void onMouseDown(const CL_InputEvent &key);
		//! Tells CEGUI when a mouse button is released
		virtual void onMouseUp(const CL_InputEvent &key);
		//! Tells CEGUI when the mouse moves
		virtual void onMouseMove(const CL_InputEvent &e);

		//! Tells CEGUI when a keyboard key goes down
		virtual void onKeyDown(const CL_InputEvent &key);
		//! Tells CEGUI when a keyboard key is released
		virtual void onKeyUp(const CL_InputEvent &key);

	};

}

#endif
