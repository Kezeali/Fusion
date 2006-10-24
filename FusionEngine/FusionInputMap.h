#ifndef Header_FusionEngine_FusionInputMaps
#define Header_FusionEngine_FusionInputMaps

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

namespace FusionEngine
{

	/* Possable input devices.
	struct InputDevType
	{
		enum { Keyboard = 0, Gamepad = 1, Mouse = 2};
	};
	*/

	/*!
	 * \brief
	 * Keybindings for player specific functions.
	 */
	class PlayerInputMap
	{
	public:
		//! Constructor
		PlayerInputMap() {}

	public:

		int thrust;
		int reverse;
		int left;
		int right;
		int primary;
		int secondary;
		int bomb;

		//! The device from which these inputs should be taken.
		CL_InputDevice device;
	};

	/*!
	 * \brief
	 * Keybindings for global functions.
	 *
	 * \remarks
	 * Note that this class has no 'type' (device) attribute because global inputs are
	 * always from the keyboard.
	 */
	class GlobalInputMap
	{
	public:
		//! Open menu
		int menu;
		//! Bring up console
		int console;

		//! [depreciated] Input manager assumes KB for simplicity's sake. 
		//! The device from which these inputs should be taken.
		CL_InputDevice device;
	};

}

#endif
