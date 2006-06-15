#ifndef Header_Fusion_FusionGUI_MainMenu
#define Header_Fusion_FusionGUI_MainMenu

#if _MSC_VER > 1000
#pragma once
#endif

namespace Fusion
{

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