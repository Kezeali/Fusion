/*
  Copyright (c) 2007 Fusion Project Team

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


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_Control
#define Header_FusionEngine_Control

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Provides control information
	class Control
	{
	public:
		//! Defines the specific meanings of the Button property's value for this class
		enum MouseAxisID { MOUSEX, MOUSEY };

		// The name of the input which uses this control
		std::string m_InputName;
		// Universal name for the control (key, button, stick, etc.)
		std::string m_ControlName;
		CL_InputDevice m_Device;
		// Cached control ID, used by the InputManager
		int m_Code;
		bool m_Analog;
		// 'Dead zone'
		float m_AxisThreshold;

		//InputState m_State;
		

	public:
		//! Constructor
		Control()
			: m_Code(0),
			m_Analog(false),
			m_Device(CL_Keyboard::get_device()),
			m_AxisThreshold(1.0f)
		{}
		//! Constructor +stuff
		Control(std::string name, std::string controlName)
			: m_Name(name),
			m_Analog(false),
			m_ButtonName(buttonName),
			m_Device(CL_Keyboard::get_device()),
			m_AxisThreshold(1.0f)
		{}
		//! Constructor +stuff +device
		Control(std::string name, std::string buttonName, const CL_InputDevice& device, int code)
			: m_Name(name),
			m_ButtonName(buttonName),
			m_Device(device),
			m_Button(code),
			m_AxisThreshold(1.0f)
		{
			if (device.get_type() == CL_InputDevice::joystick)
				m_Analog = true;
			else
				m_Analog = false;
		}
	public:
		//! Retrieves the Name property
		const std::string& GetInputName() const
		{
			return m_InputName;
		}
		//! Sets the Name property
		void SetInputName(const std::string& name)
		{
			m_InputName = name;
		}

		//! Retrieves the Code property
		int GetCode() const
		{
			return m_Code;
		}
		//! Sets the Code property
		void SetCode(int button)
		{
			m_Code = button;
		}

		//! Retrieves the ControlName property
		const std::string& GetButtonName() const
		{
			return m_ButtonName;
		}
		//! Sets the ButtonName property
		void SetButton(const std::string& buttonName)
		{
			m_ButtonName = buttonName;
		}

		//! Retrieves the State property
		bool IsDown() const
		{
			return m_State;
		}
		//! Sets the State property
		void SetDown(bool state)
		{
			m_State = state;
		}

		//! Retrieves the State property
		float GetPosition() const
		{
			return m_Position;
		}
		//! Sets the State property
		void SetPosition(float position)
		{
			m_Position = position;
		}

		//! Retrieves the Device property
		const CL_InputDevice& GetDevice() const
		{
			return m_Device;
		}
		//! Sets the Device property
		void SetDevice(const CL_InputDevice&  device)
		{
			m_Device = device;
		}

		//! Returns true if the given InputEvent signifies that this control is active
		/*!
		* (i.e. the user is pressing this button)
		*/
		virtual bool Matches(const CL_InputEvent& inputEvent)
		{
			return inputEvent.device.get_id() == m_Device.get_id() && 
				inputEvent.id == m_Button && 
				(!m_Analog || inputEvent.axis_pos >= m_AxisThreshold);
		}
		//! Updates the control
		virtual void UpdateState(const CL_InputEvent& inputEvent)
		{
			m_State = (inputEvent.type == CL_InputEvent::pressed) ? true : false;
			if (m_Analog)
				m_Position = inputEvent.axis_pos;

			//m_Position = inputEvent.mouse_pos.x;
			//m_Position = inputEvent.mouse_pos.y;
		}
		//! Updates the control if it matches
		virtual bool UpdateIfMatches(const CL_InputEvent& inputEvent)
		{
			if (Matches(inputEvent))
			{
				UpdateState(inputEvent);
				return true;
			}
			else
				return false;
		}
	};

		////! Provides control information
		//class Control
		//{
		//public:
		//	std::string m_Name;
		//	int m_Button;
		//	std::string m_ButtonName;
		//	bool m_State;
		//	CL_InputDevice m_Device;
		//public:
		//	//! Constructor
		//	Control()
		//		: m_State(false),
		//		m_Button(0),
		//		m_Device(CL_Keyboard::get_device())
		//	{}
		//	//! Constructor +stuff
		//	Control(std::string name, int button, std::string buttonName)
		//		: m_Name(name),
		//		m_Button(button),
		//		m_ButtonName(buttonName),
		//		m_State(false),
		//		m_Device(CL_Keyboard::get_device())
		//	{}
		//	//! Constructor +stuff +device
		//	Control(std::string name, int button, std::string buttonName, const CL_InputDevice& device)
		//		: m_Name(name),
		//		m_Button(button),
		//		m_ButtonName(buttonName),
		//		m_State(false),
		//		m_Device(device)
		//	{}
		//public:
		//	//! Retrieves the Name property
		//	const std::string& GetName() const
		//	{
		//		return m_Name;
		//	}
		//	//! Sets the Name property
		//	void SetName(const std::string& name)
		//	{
		//		m_Name = name;
		//	}

		//	//! Retrieves the Button property
		//	int GetButton() const
		//	{
		//		return m_Button;
		//	}
		//	//! Sets the Button property
		//	void SetButton(int button)
		//	{
		//		m_Button = button;
		//	}

		//	//! Retrieves the ButtonName property
		//	const std::string& GetButtonName() const
		//	{
		//		return m_ButtonName;
		//	}
		//	//! Sets the ButtonName property
		//	void SetButton(const std::string&  buttonName)
		//	{
		//		m_ButtonName = buttonName;
		//	}

		//	//! Retrieves the State property
		//	int GetState() const
		//	{
		//		return m_State;
		//	}
		//	//! Sets the State property
		//	void SetState(bool state)
		//	{
		//		m_State = state;
		//	}

		//	//! Retrieves the Device property
		//	const CL_InputDevice& GetDevice() const
		//	{
		//		return m_Device;
		//	}
		//	//! Sets the Device property
		//	void SetDevice(const CL_InputDevice&  device)
		//	{
		//		m_Device = device;
		//	}

		//	//! Returns true if the given InputEvent signifies that this control is active
		//	/*!
		//	 * (i.e. the user is pressing this button)
		//	 */
		//	virtual bool Matches(const CL_InputEvent& inputEvent)
		//	{
		//		return inputEvent.device.get_id() == m_Device.get_id() && inputEvent.id == m_Button;
		//	}
		//	//! Updates the control
		//	virtual void UpdateState(const CL_InputEvent& inputEvent)
		//	{
		//		m_State = true;
		//	}
		//	//! Updates the control if it matches
		//	virtual bool UpdateIfMatches(const CL_InputEvent& inputEvent)
		//	{
		//		if (Matches(inputEvent))
		//		{
		//			UpdateState(inputEvent);
		//			return true;
		//		}
		//		else
		//			return false;
		//	}
		//};

		////! Analog control
		//class AnalogControl : public Control
		//{
		//public:
		//	//! Deadzone
		//	float m_AxisThreshold;
		//	float m_Position;
		//public:
		//	//! Constructor
		//	AnalogControl() : Control(), m_AxisThreshold(1.0f) {}
		//	//! Also a constructor
		//	AnalogControl(const std::string& name, int button, float threshold, const std::string& buttonName) 
		//		: Control(name, button, buttonName, CL_Joystick::get_device()),
		//		m_AxisThreshold(threshold) 
		//	{}
		//	//! And another constructor!
		//	AnalogControl(const std::string& name, int button, float threshold, const std::string& buttonName, const CL_InputDevice& device) 
		//		: Control(name, button, buttonName, device),
		//		m_AxisThreshold(threshold)
		//	{}
		//public:
		//	//! Returns true if the given InputEvent signifies that this control is active
		//	virtual bool Matches(const CL_InputEvent& inputEvent)
		//	{
		//		return Control::Matches(inputEvent) && inputEvent.axis_pos >= m_AxisThreshold;
		//	}
		//	//! Updates the control
		//	virtual void UpdateState(const CL_InputEvent& inputEvent)
		//	{
		//		Control::UpdateState(inputEvent);
		//		m_Position = inputEvent.axis_pos;
		//	}

		//	//! Retreives the Position property
		//	float GetPosition() const
		//	{
		//		return m_Position;
		//	}
		//};

		////! Mouse control
		//class MouseControl : public AnalogControl
		//{
		//public:
		//	//! Defines the specific meanings of the Button property's value for this class
		//	enum MouseAxisID { MOUSEX, MOUSEY };
		//	// Used to calculate delta (which is the value stored in the Position property)
		//	int m_PreviousAbsPosition;
		//	// 1 / screen dimension
		//	float m_Scale;
		//public:
		//	//! Constructor
		//	MouseControl() : AnalogControl(), m_PreviousAbsPosition(0) {}
		//	//! Also a constructor
		//	MouseControl(const std::string& name, int id, float threshold, const std::string& buttonName) 
		//		: AnalogControl(name, id, threshold, buttonName, CL_Mouse::get_device()),
		//		m_PreviousAbsPosition(0)
		//	{}
		//	//! And another constructor!
		//	MouseControl(const std::string& name, int id, float threshold, const std::string& buttonName, const CL_InputDevice& device) 
		//		: AnalogControl(name, id, threshold, buttonName, device),
		//		m_PreviousAbsPosition(0)
		//	{}
		//public:
		//	//! Returns true if the given InputEvent signifies that this control is active
		//	virtual bool Matches(const CL_InputEvent& inputEvent)
		//	{
		//		float delta;
		//		if (m_Button == MOUSEX)
		//		{
		//			delta = (m_PreviousAbsPosition - inputEvent.mouse_pos.x) * FusionInput::getSingleton().GetMouseSensitivity();
		//			return inputEvent.device.get_id() == m_Device.get_id() && delta >= m_AxisThreshold;
		//		}
		//		else
		//		{
		//			delta = m_PreviousAbsPosition - inputEvent.mouse_pos.y;
		//			return inputEvent.device.get_id() == m_Device.get_id() && delta >= m_AxisThreshold;
		//		}
		//	}
		//	//! Updates the control
		//	virtual void UpdateState(const CL_InputEvent& inputEvent)
		//	{
		//		AnalogControl::UpdateState(inputEvent);
		//		if (m_Button == MOUSEX)
		//			m_Position = m_PreviousAbsPosition - inputEvent.mouse_pos.x; // m_Position stores delta
		//		else
		//			m_Position = m_PreviousAbsPosition - inputEvent.mouse_pos.y;
		//	}
		//};


}

#endif