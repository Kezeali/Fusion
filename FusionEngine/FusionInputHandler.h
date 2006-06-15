#ifndef Header_FusionEngine_FusionInput
#define Header_FusionEngine_FusionInput

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionInputMap.h"
#include "FusionInputData.h"
#include "FusionClientOptions.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Provides an interface to an input buffer optimised for FusionEngine.
	 *
	 * FusionEngine contains only gameplay functions, so all input is either contol input
	 * for ships, or global input - such as opening the console / menu (which have their
	 * own input handlers) or quitting the game.
	 *
	 */
	class FusionInput
	{
	public:
		FusionInput();

		/*!
		 * \brief
		 * Finds required devices.
		 *
		 * This checks all input maps currently bound (by FusionInput::SetInputMaps()) to
		 * see if the device they are set to is present. If it isn't the test fails.
		 *
		 * \returns
		 * True if all devices are found.
		 *
		 * \sa PlayerInputMap | GlobalInputMap
		 */
		bool Test();
		//! Activates the input handler.
		void Activate();
		//! Unbinds inputs. Call when going to the menu.
		void Suspend();

		void SetInputMaps(const ClientOptions &from);

		//! Returns the currently pressed inputs for the given ship.
		ShipInput GetShipInputs(unsigned int player) const;
		//! Returns the currently pressed inputs for all ships.
		std::vector<ShipInput> GetAllShipInputs() const;
		//! Returns the currently pressed global inputs.
		GlobalInput GetGlobalInputs() const;

	private:
		typedef std::map<PlayerInputMap> PlayerInputMapContainer;
		typedef std::map<ShipInput> ShipInputContainer;
		/*!
		 * \brief
		 * Not used yet.
		 * \todo pass the diaplay's ic to the object and use that.
		 */
		CL_InputContext *m_InputContext;
		CL_SlotContainer m_Slots;

		std::vector<PlayerInputMap> m_PlayerInputMaps;
		GlobalInputMap m_GlobalInputMap;

		std::vector<ShipInput> m_ShipInputData;
		GlobalInput m_GlobalInputData;

		//! Handle keyboard / keybased input.
		void onKeyDown(const CL_InputEvent &key);
		// Other imput devices not yet implimented.
		//void OnPointerMove(const CL_InputEvent &e);
	};

}

#endif