#ifndef Header_FusionEngine_FusionInputMaps
#define Header_FusionEngine_FusionInputMaps

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	//! Possable input devices.
	struct InputDevType
	{
		enum { Keyboard = 0, Gamepad = 1, Mouse = 2};
	};

	/*!
	 * \brief
	 * Keybindings for player specific functions.
	 */
	class PlayerInputMap
	{
	public:
		PlayerInputMap()
			: type(InputDevType::Keyboard),
			index(0)
		{
		}

		int thrust;
		int reverse;
		int left;
		int right;
		int primary;
		int secondary;
		int bomb;

		//! The device these inputs should be taken from.
		unsigned int type;
		//! The index of the device these inputs should be taken from.
		int index;
		CL_InputDevice *device;
	};

	/*!
	 * \brief
	 * Keybindings for global functions.
	 *
	 * \remarks
	 * Note that this class has no 'type' (device) attribute because global inputs are
	 * allways from the keyboard.
	 */
	class GlobalInputMap
	{
	public:
		int menu;
		int console;
	};

}

#endif
