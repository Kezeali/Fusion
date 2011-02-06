/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionXInputController
#define Header_FusionXInputController

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

// ClanLib/core.h includes windows.h which is a prerequisite of xinput.h
//  thus, since core.h has to be included anyway, it is included before xinput.h
#include <ClanLib/core.h>

#include <XInput.h>

#pragma comment(lib, "XInput.lib")

#define FSN_XINPUT_DEADZONE_LINEAR

namespace FusionEngine
{

	// Forward decl.
	class XInputController;

	//! State information passed through input event signals
	class XInputEvent
	{
	public:
		/// \brief Event types.
		enum Type
		{
			none,
			pressed,
			released,
			axis_moved
		};

	public:
		XInputEvent() : type(none), controller(NULL)
		{}
		~XInputEvent()
		{}

	public:
		/// \brief Key or axis identifier.
		int id;

		/// \brief Event type.
		Type type;

		/// \brief Device that event originates from.
		XInputController *controller;

		/// \brief Axis position.
		double axis_pos;

		/// \brief The repeat count for this event.
		/** <p>The variable contains the number of times the keystroke is
		autorepeated as a result of the user holding down the key.</p>*/
		int repeat_count;
	};

	class XInputState
	{
	private:
		float m_LeftTrigger;
		float m_RightTrigger;
    double m_ThumbLX;
    double m_ThumbLY;
    double m_ThumbRX;
    double m_ThumbRY;

	public:
		DWORD GetPacketNumber() const;

		float GetLeftTrigger() const;
		float GetRightTrigger() const;

		double GetThumbLeftX() const;
		double GetThumbLeftY() const;

		double GetThumbRightX() const;
		double GetThumbRightY() const;

	};

	//! Wraps XInput functions
	class XInputController
	{
	private:
		DWORD m_Index;
		XINPUT_STATE m_State;

		int m_LeftDeadzone;
		int m_RightDeadzone;
		int m_TriggerThreshold;

	public:
		XInputController(unsigned int index);
		~XInputController();

		//! The thresholds for creating input events.
		/*!
		 * Poll() will not create input events for the given analog
		 * inputs (left thumb, right thumb, triggers) until they report
		 * values above the given thresholds.
		 *
		 * \param left
		 * Left thumb threshold
		 *
		 * \param right
		 * Right thumb threshold
		 *
		 * \param triggers
		 * Trigger threshold
		 */
		void SetDeadzones(int left, int right, int triggers);

		//! Returns the User Index (1-4) of this controller
		unsigned int GetUserIndex() const;

		//! Returns the current state
		const XINPUT_STATE& GetState();
		//! Returns true if the controller is connected
		bool IsConnected();
		//! Activates vibration
		void Vibrate(int leftVal, int rightVal);

		//! Checks the controller's state and creates input events accordingly
		void Poll();

		static CL_String GetKeyName(WORD id);

		CL_Signal_v1<const XInputEvent &> sig_key_down;
		CL_Signal_v1<const XInputEvent &> sig_key_up;
		CL_Signal_v1<const XInputEvent &> sig_axis_move;

	private:
		void processThumbInput(SHORT x, int x_axis_id, SHORT y, int y_axis_id, SHORT deadzone);
	};


	static WORD XInputKeycodeToButtonFlag(WORD keycode)
	{
		if (keycode == VK_PAD_A)
			return XINPUT_GAMEPAD_A;
		else if (keycode == VK_PAD_B)
			return XINPUT_GAMEPAD_B;
		else if (keycode == VK_PAD_X)
			return XINPUT_GAMEPAD_X;
		else if (keycode == VK_PAD_Y)
			return XINPUT_GAMEPAD_Y;

		else if (keycode == VK_PAD_LSHOULDER)
			return XINPUT_GAMEPAD_LEFT_SHOULDER;
		else if (keycode == VK_PAD_RSHOULDER)
			return XINPUT_GAMEPAD_RIGHT_SHOULDER;

		else if (keycode == VK_PAD_LTHUMB_PRESS)
			return XINPUT_GAMEPAD_LEFT_THUMB;
		else if (keycode == VK_PAD_RTHUMB_PRESS)
			return XINPUT_GAMEPAD_RIGHT_THUMB;

		else if (keycode == VK_PAD_START)
			return XINPUT_GAMEPAD_START;
		else if (keycode == VK_PAD_START)
			return XINPUT_GAMEPAD_START;

		else if (keycode == VK_PAD_DPAD_UP)
			return XINPUT_GAMEPAD_DPAD_UP;
		else if (keycode == VK_PAD_DPAD_DOWN)
			return XINPUT_GAMEPAD_DPAD_DOWN;
		else if (keycode == VK_PAD_DPAD_LEFT)
			return XINPUT_GAMEPAD_DPAD_LEFT;
		else if (keycode == VK_PAD_DPAD_RIGHT)
			return XINPUT_GAMEPAD_DPAD_RIGHT;
	}

	//class InputDeviceProvider_XInput : public CL_InputDeviceProvider
	//{
	//public:
	//	//InputDeviceProvider_XInput(XInputController* controller)
	//	//	: m_Controller(controller)
	//	//{
	//	//}
	//	InputDeviceProvider_XInput(unsigned int userIndex)
	//		: m_Controller(new XInputController(userIndex))
	//	{
	//		m_Slots.connect(m_Controller->sig_axis_move, this, &InputDeviceProvider_XInput::onInputEvent);
	//		m_Slots.connect(m_Controller->sig_key_down, this, &InputDeviceProvider_XInput::onInputEvent);
	//		m_Slots.connect(m_Controller->sig_key_up, this, &InputDeviceProvider_XInput::onInputEvent);
	//	}
	//	~InputDeviceProvider_XInput()
	//	{
	//		delete m_Controller;
	//	}

	//	void destroy() { delete this; }

	//public:
	//	CL_InputDevice::Type get_type() const { return CL_InputDevice::joystick; }

	//	//! Retuns 0
	//	int get_x() const { return 0; }

	//	//! \brief Returns 0.
	//	int get_y() const { return 0; }

	//	/// \brief Returns true if the passed key code is down for this device.
	//	bool get_keycode(int keycode) const
	//	{
	//		return (m_Controller->GetState().Gamepad.wButtons & XInputKeycodeToButtonFlag(keycode)) ?
	//			true : false;
	//	}

	//	/// \brief Key name for specified identifier (A, B, C, Space, Enter, Backspace).
	//	CL_String get_key_name(int id) const
	//	{
	//		switch (id)
	//		{
	//		case VK_PAD_A:
	//			return "A Button";
	//		case VK_PAD_B:
	//			return "B Button";
	//		case VK_PAD_X:
	//			return "X Button";
	//		case VK_PAD_Y:
	//			return "Y Button";

	//		case VK_PAD_RSHOULDER:
	//			return "Right Sholder";
	//		case VK_PAD_LSHOULDER:
	//			return "Left Sholder";
	//		case VK_PAD_LTRIGGER:
	//			return "Left Trigger";
	//		case VK_PAD_RTRIGGER:
	//			return "Right Trigger";

	//		case VK_PAD_DPAD_UP:
	//			return "DPAD Up";
	//		case VK_PAD_DPAD_DOWN:
	//			return "DPAD Down";
	//		case VK_PAD_DPAD_LEFT:
	//			return "DPAD Left";
	//		case VK_PAD_DPAD_RIGHT:
	//			return "DPAD Right";

	//		case VK_PAD_START:
	//			return "Start";
	//		case VK_PAD_BACK:
	//			return "Back";
	//		case VK_PAD_LTHUMB_PRESS:
	//			return "Left Thumb Press";
	//		case VK_PAD_RTHUMB_PRESS:
	//			return "Right Thumb Press";

	//		default:
	//			return cl_format("XInput Key %1", id);
	//		}
	//	}

	//	static inline float normaize(SHORT value)
	//	{
	//		return value / 32767;
	//	}

	//	/// \brief Returns the the current position of a joystick axis.
	//	float get_axis(int index) const
	//	{
	//		// Left Thumb
	//		if (index == 0)
	//			return normaize(m_Controller->GetState().Gamepad.sThumbLX);
	//		else if (index == 1)
	//			return normaize(m_Controller->GetState().Gamepad.sThumbLY);
	//		// Right Thumb
	//		else if (index == 2)
	//			return normaize(m_Controller->GetState().Gamepad.sThumbRX);
	//		else if (index == 3)
	//			return normaize(m_Controller->GetState().Gamepad.sThumbRY);

	//		// Triggers
	//		else if (index == 4)
	//			return normaize(m_Controller->GetState().Gamepad.bLeftTrigger);
	//		else if (index == 5)
	//			return normaize(m_Controller->GetState().Gamepad.bRightTrigger);

	//		else
	//			return 0.f;
	//	}

	//	/// \brief Returns the name of the device (i.e. 'Microsoft Sidewinder 3D').
	//	CL_String get_name() const
	//	{
	//		return "Xbox 360 Gamepad";
	//	}

	//	/// \brief Return the hardware id/device for this device (i.e. /dev/input/js0)
	//	CL_String get_device_name() const
	//	{
	//		return cl_format("Xbox 360 Gamepad %1", m_Controller->GetUserIndex());
	//	}

	//	//! \brief Returns the number of axes available on this device.
	//	int get_axis_count() const
	//	{
	//		return 6;
	//	}

	//	//! \brief Returns the number of buttons available on this device.
	//	int get_button_count() const
	//	{
	//		return 14;
	//	}

	//	/// \brief Returns true if a tablet stylus is in proximity (hovering close enough over the tablet surface).
	//	bool in_proximity() const { return false; }
	//public:
	//	/// \brief Initialize input device provider.
	//	/** <p>The device field of CL_InputEvent should not be set when emitting events.</p>*/
	//	void init(CL_Signal_v1<const CL_InputEvent &> *new_sig_provider_event)
	//	{
	//		sig_provider_event = new_sig_provider_event;
	//	}

	//	/// \brief Sets the position of the device.
	//	void set_position(int x, int y) { return; }


	//	/// \brief Update device
	//	///
	//	/// \param peek_only Treat as a request to see if an event would occur
	//	///
	//	/// \return true when the device event has occurred
	//	bool poll(bool peek_only)
	//	{
	//		if (peek_only)
	//			return false;

	//		m_Controller->Poll();
	//	}

	//	void onInputEvent(const CL_InputEvent& ev, const CL_InputState& state)
	//	{
	//		if (sig_provider_event != NULL)
	//			sig_provider_event->invoke(ev);
	//	}

	//private:
	//	CL_Signal_v1<const CL_InputEvent &> *sig_provider_event;

	//	XInputController* m_Controller;

	//	CL_SlotContainer m_Slots;

	//};

}

#endif
