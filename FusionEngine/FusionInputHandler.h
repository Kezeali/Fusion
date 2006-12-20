/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_FusionEngine_FusionInput
#define Header_FusionEngine_FusionInput

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

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
	 */
	class FusionInput : public FusionEngine::Singleton<FusionInput>
	{
	public:
		//! Basic constructor.
		FusionInput();
		//! Constructor. +ClientOptions
		FusionInput(const ClientOptions *from);

	public:
		//! Typedef for ship inputs
		typedef std::vector<PlayerInputMap> PlayerInputMapList;
		//! Typedef for ship inputs
		typedef std::vector<ShipInput> ShipInputList;

	public:
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
		//! Sets up the input slots.
		void Initialise();
		//! Activates the input handler.
		void Activate();
		//! Unbinds inputs. Call when going to the menu.
		void Suspend();

		//! Sets up inputs
		void SetInputMaps(const ClientOptions *from);

		//! Returns the currently pressed inputs for the given ship.
		ShipInput GetShipInputs(PlayerInd player) const;
		//! Returns the currently pressed inputs for all ships.
		ShipInputList GetAllShipInputs() const;
		//! Returns the currently pressed global inputs.
		GlobalInput GetGlobalInputs() const;

	private:
		//! The InputHandler will not be considered active till this reaches zero.
		int m_SuspendRequests;

		/*!
		 * \brief
		 * Not used yet.
		 * \todo pass the diaplay's ic to the object and use that.
		 */
		CL_InputContext *m_InputContext;
		//! Slot container for inputs
		CL_SlotContainer m_Slots;

		//! Individual input setup
		PlayerInputMapList m_PlayerInputMaps;
		//! Global input setup
		GlobalInputMap m_GlobalInputMap;

		//! Individual input state data
		ShipInputList m_ShipInputData;
		//! Global input state data
		GlobalInput m_GlobalInputData;

		//! Handle keyboard / keybased input. Down
		void onKeyDown(const CL_InputEvent &key);
		//! Handle keyboard / keybased input. Up
		void onKeyUp(const CL_InputEvent &key);
		// Other imput devices not yet implimented.
		//void OnPointerMove(const CL_InputEvent &e);
	};

}

#endif
