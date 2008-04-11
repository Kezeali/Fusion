/*
  Copyright (c) 2006-2008 Fusion Project Team

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

#include <OIS/OIS.h>

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionInputMap.h"
#include "FusionInputData.h"
#include "FusionClientOptions.h"
#include "FusionControl.h"

namespace FusionEngine
{
	static const float g_InputAnalogFuzz = 0.005f;

	/*!
	 * \brief
	 * Used by the control human-readable name map. Loaded from XML
	 */
	class KeyName
	{
	public:
		// String mapped to this key (used in config files, internally, etc.)
		std::string m_Name;
		// Control device
		std::string m_Device;
		// Scancode, button ID - however the device identifies this control
		int m_Code;
		// Shown in the UI
		std::string m_Description;
	};

	class InputState
	{
	public:
		InputState()
			: m_Down(false), m_Value(0.0f)
		{}
		bool m_Down;
		bool m_Changed; // If the button was pressed / released / both since the last command
		float m_Value;
	};

	class Command
	{
	public:
		typedef std::map<std::string, InputState> InputStateMap;
		InputStateMap m_States;

		void SetState(const std::string &input_name, bool isDown, bool value)
		{
			InputState &state = m_States[input_name];
			state.m_Down = isDown;
			state.m_Value = value;
		}

		void GetDelta(const Command &previousCommand)
		{
			for (InputStateMap::iterator it = previousCommand.m_States.begin(), end = previousCommand.m_States.end(); it != end; ++it)
			{
				InputState &currentState = m_States[it->first];
				if (it->second.m_Down != currentState.m_Down || abs(it->second.m_Value - currentState.m_Value) > g_InputAnalogFuzz)
					currentState.m_Changed = true;
		}

		bool IsDown(const std::string &input)
		{
			return m_States[input].m_Down;
		}
		float GetValue(const std::string &input)
		{
			return m_States[input].m_Value;
		}
	};

	/*!
	 * \brief
	 * Provides an interface to an input buffer optimised for FusionEngine.
	 *
	 * \todo
	 * Another design possibility for Input would be to make it a non-singleton
	 * and have an instance per player, then have a class (which /is/ a singleton)
	 * to provide access to each player's Input. Like 
	 * <code>PlayerInputs::getSingleton().player[1].IsButtonPressed("Left");</code>
	 */
	class InputManager : public FusionEngine::Singleton<InputManager>
	{
	public:
		//! Basic constructor.
		InputManager();
		//! Constructor.
		InputManager(CL_DisplayWindow *window);
		//! Constructor. +ClientOptions
		InputManager(CL_DisplayWindow *window, const ClientOptions *from);
		//! Deconstructor
		~InputManager();

	public:
		//! Input names mapped to controls
		typedef std::map<std::string, Control> ControlMap;

		//typedef std::map<std::string, InputState> Command;
		typedef std::vector<Command> CommandList;
		typedef std::vector<CommandList> PlayerCommandLists;

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
		//! Sets up the input manager.
		void Initialise();
		//! Drops any settings
		void CleanUp();
		//! Activates the input handler.
		void Activate();
		//! Unbinds inputs. Call when going to the menu.
		void Suspend();

		void Update(unsigned int split);

		//! Sets up inputs
		void SetInputMaps(const ClientOptions *from);

		void MapControl(int keysym, const std::string& name, unsigned int filter = 0);
		void MapControl(int keysym, const std::string& name, CL_InputDevice device, unsigned int filter = 0);

		const Control &GetControl(const std::string& name, unsigned int filter = 0) const;
		bool IsButtonDown(const std::string& name, unsigned int filter = 0) const;
		float GetAnalogValue(const std::string& name, unsigned int filter = 0) const;

		void CreateCommand(int tick, unsigned int split, unsigned int player);

		float GetMouseSensitivity() const;

		////! Returns the currently pressed inputs for the given ship.
		//ShipInput GetShipInputs(ObjectID player) const;
		////! Returns the currently pressed inputs for all ships.
		//ShipInputList GetAllShipInputs() const;
		////! Returns the currently pressed global inputs.
		//GlobalInput GetGlobalInputs() const;

	private:
		ControlMap m_ControlMap;

		//PlayerCommandLists m_PlayerCommands;

		InputPluginLoader *m_PluginLoader;

		//! The InputHandler will not be considered active till this reaches zero.
		int m_SuspendRequests;

		unsigned int m_CommandBufferLength;

		/*!
		 * \brief
		 * Not used yet.
		 * \todo pass the diaplay's ic to the object and use that.
		 */
		CL_InputContext *m_InputContext;
		//! Slot container for inputs
		CL_SlotContainer m_Slots;

		////! Individual input setup
		//PlayerInputMapList m_PlayerInputMaps;
		////! Global input setup
		//GlobalInputMap m_GlobalInputMap;

		////! Individual input state data
		//ShipInputList m_ShipInputData;
		////! Global input state data
		//GlobalInput m_GlobalInputData;

		// Mouse movement multiplier
		float m_MouseSensitivity;

		// Used when polling mouse movement
		int m_DisplayCenterX;
		int m_DisplayCenterY;

		//! Loads human readable and UI control (key / button, etc) names
		void loadControlNames(const ticpp::Document& defDocument);

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
