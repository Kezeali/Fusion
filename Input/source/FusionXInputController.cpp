/*
*  Copyright (c) 2009-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionXInputController.h"

namespace FusionEngine
{

	XInputController::XInputController(unsigned int index)
		: m_Index((DWORD)index),
		m_LeftDeadzone(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
		m_RightDeadzone(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
		m_TriggerThreshold(XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
	}

	XInputController::~XInputController()
	{
		Vibrate(0, 0);
	}

	void XInputController::SetDeadzones(int left, int right, int trigger_threshold)
	{
		m_LeftDeadzone = left;
		m_RightDeadzone = right;
		m_TriggerThreshold = trigger_threshold;
	}

	unsigned int XInputController::GetUserIndex() const
	{
		return (unsigned int)m_Index;
	}

	const XINPUT_STATE& XInputController::GetState()
	{
		ZeroMemory(&m_State, sizeof(XINPUT_STATE));

		XInputGetState(m_Index, &m_State);

		return m_State;
	}

	bool XInputController::IsConnected()
	{
		ZeroMemory(&m_State, sizeof(XINPUT_STATE));

		DWORD result = XInputGetState(m_Index, &m_State);

		if(result == ERROR_SUCCESS)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void XInputController::Vibrate(int leftVal, int rightVal)
	{
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

		vibration.wLeftMotorSpeed = leftVal;
		vibration.wRightMotorSpeed = rightVal;

		// Vibrate the controller
		XInputSetState(m_Index, &vibration);
	}

	void XInputController::processThumbInput(SHORT x, int x_axis_id, SHORT y, int y_axis_id, SHORT deadzone)
	{
#ifdef FSN_XINPUT_DEADZONE_LINEAR
		// Check that the axis is outside a linear dead zone
		if (abs(x) > deadzone)
		{
			// Adjust value relative to the end of the dead zone
			if (x < 0) // Position value may be negative, in this case the deadzone is added to remove it
				x += deadzone;
			else
				x -= deadzone;

			// Normalize the value with respect to the expected range
			//  giving a value of 0.0 to 1.0
			double normalizedX = x / (32767 - deadzone);

			XInputEvent event;
			event.controller = this;
			event.type = XInputEvent::axis_moved;
			event.id = x_axis_id;
			event.axis_pos = (double)normalizedX;
			sig_axis_move.invoke(event);
		}

		if (abs(y) > deadzone)
		{
			// Adjust value relative to the end of the dead zone
			if (y < 0)
				y += deadzone;
			else
				y -= deadzone;

			// Normalize the value with respect to the expected range
			//  giving a value of 0.0 to 1.0
			double normalizedY = y / (32767 - deadzone);

			XInputEvent event;
			event.controller = this;
			event.type = XInputEvent::axis_moved;
			event.id = y_axis_id;
			event.axis_pos = (double)normalizedY;
			sig_axis_move.invoke(event);
		}
#else
		Vector2 thumbPosition(x, y);
		//determine how far the controller is pushed
		float magnitude = leftStick.length();
		//check if the controller is outside a circular dead zone
		if (magnitude > deadzone)
		{
			// Adjust components relative to the end of the dead zone
			thumbPosition -= deadzone;

			// Normalize the components with respect to the expected range
			//  giving a value of 0.0 to 1.0
			v2Divide(thumbPosition, (32767 - deadzone), thumbPosition);

			XInputEvent event;
			event.controller = this;
			event.type = XInputEvent::axis_moved;
			event.id = x_axis_id;
			event.axis_pos = thumbPosition.x;
			sig_axis_move.invoke(event);

			event = XInputEvent();
			event.controller = this
			event.type = XInputEvent::axis_moved;
			event.id = y_axis_id;
			event.axis_pos = thumbPosition.y;
			sig_axis_move.invoke(event);
		}
#endif
	}

	void XInputController::Poll()
	{
		DWORD lastPacketNumber = m_State.dwPacketNumber;
		if (!IsConnected() || m_State.dwPacketNumber == lastPacketNumber)
			return;

		// Create events for analog inputs
		processThumbInput(m_State.Gamepad.sThumbLX, 0, m_State.Gamepad.sThumbLY, 1, m_LeftDeadzone);
		processThumbInput(m_State.Gamepad.sThumbRX, 2, m_State.Gamepad.sThumbRY, 3, m_RightDeadzone);

		float leftTrigger = m_State.Gamepad.bLeftTrigger;
		float rightTrigger = m_State.Gamepad.bRightTrigger;
		if (leftTrigger > m_TriggerThreshold)
		{
			// Normalize
			leftTrigger -= m_TriggerThreshold;
			leftTrigger = leftTrigger / (255 - m_TriggerThreshold);

			XInputEvent event;
			event.controller = this;
			event.type = XInputEvent::axis_moved;
			event.id = 4;
			event.axis_pos = leftTrigger;
			sig_axis_move.invoke(event);
		}
		if (rightTrigger > m_TriggerThreshold)
		{
			// Normalize
			rightTrigger -= m_TriggerThreshold;
			rightTrigger = rightTrigger / (255 - m_TriggerThreshold);

			XInputEvent event;
			event.controller = this;
			event.type = XInputEvent::axis_moved;
			event.id = 5;
			event.axis_pos = rightTrigger;
			sig_axis_move.invoke(event);
		}

		// Create events for digital inputs
		XINPUT_KEYSTROKE keystroke;
		while (XInputGetKeystroke(m_Index, XINPUT_FLAG_GAMEPAD, &keystroke) == ERROR_SUCCESS)
		{
			XInputEvent event;
			event.controller = this;
			event.id = keystroke.VirtualKey;
			event.repeat_count = 0;

			if (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
			{
				event.type = XInputEvent::pressed;
				sig_key_down.invoke(event);
			}
			if (keystroke.Flags & XINPUT_KEYSTROKE_KEYUP)
			{
				event.type = XInputEvent::released;
				sig_key_up.invoke(event);
			}
		}
	}

	CL_String XInputController::GetKeyName(WORD id)
	{
		switch (id)
		{
		case VK_PAD_A:
			return "A Button";
		case VK_PAD_B:
			return "B Button";
		case VK_PAD_X:
			return "X Button";
		case VK_PAD_Y:
			return "Y Button";

		case VK_PAD_RSHOULDER:
			return "Right Sholder";
		case VK_PAD_LSHOULDER:
			return "Left Sholder";
		case VK_PAD_LTRIGGER:
			return "Left Trigger";
		case VK_PAD_RTRIGGER:
			return "Right Trigger";

		case VK_PAD_DPAD_UP:
			return "DPAD Up";
		case VK_PAD_DPAD_DOWN:
			return "DPAD Down";
		case VK_PAD_DPAD_LEFT:
			return "DPAD Left";
		case VK_PAD_DPAD_RIGHT:
			return "DPAD Right";

		case VK_PAD_START:
			return "Start";
		case VK_PAD_BACK:
			return "Back";
		case VK_PAD_LTHUMB_PRESS:
			return "Left Thumb Press";
		case VK_PAD_RTHUMB_PRESS:
			return "Right Thumb Press";

		default:
			return cl_format("Unknown XInput Key %1", id);
		}
	}

}
