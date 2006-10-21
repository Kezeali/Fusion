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

#ifndef Header_Fusion_FusionGUI_MainMenu
#define Header_Fusion_FusionGUI_MainMenu

#if _MSC_VER > 1000
#pragma once
#endif

namespace Fusion
{

	/*!
	 * \brief
	 * [depreciated] GUI for fusion mainmenu
	 * \todo Replace CL_GuiManager with CEGUI
	 */
	class FusionGUI_MainMenu
	{
	public:
		/*!
		* Constructor.
		*
		* \param resources
		* Where to get the components from.
		*
		* \param parent
		* The parent of this component. Usually the main CL_GUIManager.
		*
		* \param deck
		* The deck this component resides in.
		*/
		FusionGUI_MainMenu(CL_ResourceManager *resources, CL_Component *parent, CL_Deck *deck);

	public:
		static const std::string Name = "main_menu";

		virtual CL_Component *GetComponent();

		void on_createClicked();
		void on_joinClicked();
		void on_optsClicked();

	protected:
		CL_Deck *m_Deck;
		CL_ComponentManager m_ComponentManager;
		CL_SlotContainer m_Slots;
	};

}

#endif
