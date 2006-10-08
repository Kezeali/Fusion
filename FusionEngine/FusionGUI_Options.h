#ifndef Header_Fusion_FusionGUI_Options
#define Header_Fusion_FusionGUI_Options

#if _MSC_VER > 1000
#pragma once
#endif

namespace Fusion
{

	class FusionGUI_Options
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
		FusionGUI_Options(CL_ResourceManager *resources, CL_Component *parent, CL_Deck *deck);

	public:
		static const std::string Name = "options";

		virtual CL_Component *GetComponent();

	protected:
		CL_Deck *m_Deck;
		CL_ComponentManager m_ComponentManager;
		CL_SlotContainer m_Slots;
	};

}

#endif
