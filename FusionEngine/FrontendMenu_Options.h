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

#ifndef Header_Fusion_FusionGUI_Options
#define Header_Fusion_FusionGUI_Options

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Fusion
#include "FusionGUI.h"

#include "FusionClientOptions.h"

namespace Fusion
{

	//! Options gui
	class FusionGUI_Options : public FusionGUI
	{
	public:
		//! Basic constructor
		FusionGUI_Options();
		//! Constructor
		FusionGUI_Options(FusionEngine::ClientOptions *clientopts);

	public:
		//! Init the gui
		bool Initialise();

		//! Called when the Save button is clicked
		bool onSaveClicked(const CEGUI::EventArgs& e);

	protected:
		//! The options to edit / save to
		FusionEngine::ClientOptions *m_ClientOpts;

	};

}

#endif
