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
	 * All input handled by this class is either contol input for ships, or
	 * global input - such as opening the console / menu (which have their
	 * own input handlers) or quitting the game.
	 *
	 * \todo Remove hardcoded control map
	 */
	class FusionInput : public FusionEngine::Singleton<FusionInput>
	{
	public:
		//! Basic constructor.
		FusionInput();
		//! Constructor. +ClientOptions
		FusionInput(const ClientOptions *from);

	public:
		//! Provides control information
		class Control
		{
		public:
			std::string m_Name;
			int m_Button;
			std::string m_ButtonName;
			bool m_State;
			CL_InputDevice m_Device;
		public:
			//! Constructor
			Control()
				: m_State(false),
				m_Button(0),
				m_Device(CL_Keyboard::get_device())
			{}
			//! Constructor +stuff
			Control(std::string name, int button, std::string buttonName)
				: m_Name(name),
				m_Button(button),
				m_ButtonName(buttonName),
				m_State(false),
				m_Device(CL_Keyboard::get_device())
			{}
			//! Constructor +stuff +device
			Control(std::string name, int button, std::string buttonName, const CL_InputDevice& device)
				: m_Name(name),
				m_Button(button),
				m_ButtonName(buttonName),
				m_State(false),
				m_Device(device)
			{}
		public:
			//! Retrieves the Name property
			const std::string& GetName() const
			{
				return m_Name;
			}
			//! Sets the Name property
			void SetName(const std::string& name)
			{
				m_Name = name;
			}

			//! Retrieves the Button property
			int GetButton() const
			{
				return m_Button;
			}
			//! Sets the Button property
			void SetButton(int button)
			{
				m_Button = button;
			}

			//! Retrieves the ButtonName property
			const std::string& GetButtonName() const
			{
				return m_ButtonName;
			}
			//! Sets the ButtonName property
			void SetButton(const std::string&  buttonName)
			{
				m_ButtonName = buttonName;
			}

			//! Retrieves the State property
			int GetState() const
			{
				return m_State;
			}
			//! Sets the State property
			void SetState(bool state)
			{
				m_State = state;
			}

			//! Retrieves the Device property
			const CL_InputDevice& GetDevice() const
			{
				return m_Device;
			}
			//! Sets the Device property
			void SetButton(const CL_InputDevice&  device)
			{
				m_Device = device;
			}

			//! Returns true if the given InputEvent signifies that this control is active
			/*!
			 * (i.e. the user is pressing this button)
			 */
			virtual bool Matches(const CL_InputEvent& inputEvent)
			{
				return inputEvent.device.get_id() == m_Device.get_id() && inputEvent.id == m_Button;
			}
			//! Updates the control
			virtual void UpdateState(const CL_InputEvent& inputEvent)
			{
				m_State = true;
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

		//! Analog control
		class AnalogControl : public Control
		{
		public:
			//! Deadzone
			float m_AxisThreshold;
			float m_Position;
		public:
			//! Constructor
			AnalogControl() : Control(), m_AxisThreshold(1.0f) {}
			//! Also a constructor
			AnalogControl(const std::string& name, int button, float threshold, const std::string& buttonName) 
				: Control(name, button, buttonName, CL_Joystick::get_device()),
				m_AxisThreshold(threshold) 
			{}
			//! And another constructor!
			AnalogControl(const std::string& name, int button, float threshold, const std::string& buttonName, const CL_InputDevice& device) 
				: Control(name, button, buttonName, device),
				m_AxisThreshold(threshold)
			{}
		public:
			//! Returns true if the given InputEvent signifies that this control is active
			virtual bool Matches(const CL_InputEvent& inputEvent)
			{
				return Control::Matches(inputEvent) && inputEvent.axis_pos >= m_AxisThreshold;
			}
			//! Updates the control
			virtual void UpdateState(const CL_InputEvent& inputEvent)
			{
				Control::UpdateState(inputEvent);
				m_Position = inputEvent.axis_pos;
			}

			//! Retreives the Position property
			float GetPosition() const
			{
				return m_Position;
			}
		};

		//! Analog control
		class MouseControl : public AnalogControl
		{
		public:
			//! Defines the specific meanings of the Button property's value for this class
			enum MouseAxisID { MOUSEX, MOUSEY };
			// Used to calculate delta (which is the value stored in the Position property)
			int m_PreviousAbsPosition;
			// 1 / screen dimension
			float m_Scale;
		public:
			//! Constructor
			MouseControl() : AnalogControl(), m_PreviousAbsPosition(0) {}
			//! Also a constructor
			MouseControl(const std::string& name, int id, float threshold, const std::string& buttonName) 
				: AnalogControl(name, id, threshold, buttonName, CL_Mouse::get_device()),
				m_PreviousAbsPosition(0)
			{}
			//! And another constructor!
			MouseControl(const std::string& name, int id, float threshold, const std::string& buttonName, const CL_InputDevice& device) 
				: AnalogControl(name, id, threshold, buttonName, device),
				m_PreviousAbsPosition(0)
			{}
		public:
			//! Returns true if the given InputEvent signifies that this control is active
			virtual bool Matches(const CL_InputEvent& inputEvent)
			{
				float delta;
				if (m_Button == MOUSEX)
				{
					delta = (m_PreviousAbsPosition - inputEvent.mouse_pos.x) * FusionInput::getSingleton().GetMouseSensitivity();
					return inputEvent.device.get_id() == m_Device.get_id() && delta >= m_AxisThreshold;
				}
				else
				{
					delta = m_PreviousAbsPosition - inputEvent.mouse_pos.y;
					return inputEvent.device.get_id() == m_Device.get_id() && delta >= m_AxisThreshold;
				}
			}
			//! Updates the control
			virtual void UpdateState(const CL_InputEvent& inputEvent)
			{
				AnalogControl::UpdateState(inputEvent);
				if (m_Button == MOUSEX)
					m_Position = m_PreviousAbsPosition - inputEvent.mouse_pos.x; // m_Position stores delta
				else
					m_Position = m_PreviousAbsPosition - inputEvent.mouse_pos.y;
			}
		};

		//! Input names mapped to controls
		typedef std::map<std::string, Control> ControlMap;

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

		void MapControl(int keysym, const std::string& name);

		const Control &GetControl(const std::string& name) const;
		bool IsButtonDown(const std::string& name) const;
		float GetAnalogValue(const std::string& name) const;

		float GetMouseSensitivity() const;

		//! Returns the currently pressed inputs for the given ship.
		ShipInput GetShipInputs(ObjectID player) const;
		//! Returns the currently pressed inputs for all ships.
		ShipInputList GetAllShipInputs() const;
		//! Returns the currently pressed global inputs.
		GlobalInput GetGlobalInputs() const;

	private:
		ControlMap m_ControlMap;
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

		// Mouse movement multiplier
		float m_MouseSensitivity;

		// Used when polling mouse movement
		int m_DisplayCenterX;
		int m_DisplayCenterY;

		//! Handle keyboard / keybased input. Down
		void onKeyDown(const CL_InputEvent &key);
		//! Handle keyboard / keybased input. Up
		void onKeyUp(const CL_InputEvent &key);
		// Other imput devices not yet implimented.
		//void OnAxisMove(const CL_InputEvent &e);
		//! Handle screen resize
		void onDisplayResize(int w, int h);
	};

}

#endif
