/*
  Copyright (c) 2006-2010 Fusion Project Team

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

#ifndef H_FusionEngine_InputManager
#define H_FusionEngine_InputManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <boost/functional/hash.hpp>
#include <ClanLib/display.h>

#include "FusionSingleton.h"

#include "FusionHashable.h"
#include "FusionInputDefinitionLoader.h"
#include "FusionLogger.h"
#include "FusionSlotContainer.h"
#include "FusionXML.h"

#ifdef _WIN32
#define FSN_USE_XINPUT
#endif
// XInput (windows only)
#ifdef FSN_USE_XINPUT
#include "FusionXInputController.h"
#endif

#define FE_INPUTMETHOD_UNBUFFERED 0
#define FE_INPUTMETHOD_BUFFERED 1

#define FE_INPUT_METHOD FE_INPUTMETHOD_UNBUFFERED

namespace FusionEngine
{
	static const float g_InputAnalogFuzz = 0.005f;

	//! \defgroup Config file device identifiers
	//!@{
	static const char* s_DevKeyboardStr = "keyboard";
	static const char* s_DevGamepadStr = "gamepad";
	static const char* s_DevGamepad_AxisStr = "gamepad-axis";
	static const char* s_DevMouseStr = "mouse";
	static const char* s_DevMouse_PointerStr = "mouse-pointer";
	static const char* s_DevMouse_AxisStr = "mouse-axis";
	static const char* s_DevXInputStr = "xinput";
	static const char* s_DevXInput_AxisStr = "xinput-axis";
	//!@}

	//! \defgroup Internal device identifiers (used by input manager)
	//!@{ 
	static const unsigned int s_DevKeyboard = 0;
	static const unsigned int s_DevGamepad = 1;
	static const unsigned int s_DevGamepad_Axis = 2;
	static const unsigned int s_DevMouse = 3;
	static const unsigned int s_DevMouse_Pointer = 4;
	static const unsigned int s_DevMouse_Axis = 5;
	static const unsigned int s_DevXInput = 10;
	static const unsigned int s_DevXInput_Axis = 11;
	static const unsigned int s_DevNothing = 0xFFFF;
	//!@}

	//! Returns the devices name for the device with the given ID
	unsigned int DeviceNameToID(const std::string& device);
	//! Returns the ID for the device with the given name
	const char*const DeviceIDToName(unsigned int device);

	//! Max devices of one type
	static const unsigned int s_DeviceCountMax = 256;
	//! For inputs bound to any device
	static const unsigned int s_DeviceIndexAny = s_DeviceCountMax - 1;

	/*!
	 * \brief
	 * Used by the control human-readable name map. Loaded from XML
	 */
	class KeyInfo
	{
	public:
		KeyInfo()
			: m_Name(""),
			m_Device(""),
			m_Code(0),
			m_Description("")
		{
		}

		KeyInfo(std::string name, std::string device, int code, std::string description)
			: m_Name(name),
			m_Device(device),
			m_Code(code),
			m_Description(description)
		{
		}
		// String mapped to this key (used in config files, internally, etc.)
		std::string m_Name;
		// Control device
		std::string m_Device;
		// Scancode, button ID - whatever the device uses to identify this input
		int m_Code;
		// Shown in the UI
		std::string m_Description;
	};

	/*!
	 * \brief
	 * Parsed input binding
	 *
	 * \see XmlInputBinding
	 */
	class InputBinding
	{
	public:
		int m_Player;
		std::string m_Input; // The 'agency' this control provides :P
		//std::string m_Key; // The short-name of the key on the keyboard / button on the controler
		KeyInfo m_Key;

		// Analog filtering settings
		double m_Threshold;
		double m_Range;
		bool m_Cubic;

	public:
		/*InputBinding(const XmlInputBinding& rawBinding)
		{
			m_Player = CL_String::to_int(rawBinding.m_Player);
			m_Input = rawBinding.m_Input;
			m_Key = rawBinding.m_Key;
		}*/
		InputBinding()
			: m_Player(0)
		{
		}

		InputBinding(int player, const std::string &input, const KeyInfo &key)
			: m_Player(player),
			m_Input(input),
			m_Key(key),
			m_Threshold(0.0),
			m_Range(1.0),
			m_Cubic(false)
		{
		}

		InputBinding(int player, const std::string &input, const KeyInfo &key, double threshold, double range, bool cubic)
			: m_Player(player),
			m_Input(input),
			m_Key(key),
			m_Threshold(threshold),
			m_Range(range),
			m_Cubic(cubic)
		{
		}

	public:
		inline void FilterValue(double &value) const
		{
			if (!fe_fzero(m_Threshold))
			{
				if (value > 0 && value > m_Threshold)
					value -= m_Threshold;
				else if (value < 0 && value < m_Threshold)
					value += m_Threshold;

				else
					value = 0.0;
			}
			// Max out at given range
			//  Note that m_Range defaults to 1.0 -> full range - in that
			//  case this simply re-normalizes taking m_Threshold into
			//  account
			if (!fe_fzero(m_Range))
			{
				if (value < m_Range)
					value = value / (m_Range - m_Threshold);
				else
					value = 1.0;
			}

			if (m_Cubic)
				value = value * value * value;

			// Clamp to valid values
			fe_clamp(value, -1.0, 1.0);
		}

		void Validate()
		{
			fe_clamp(m_Threshold, 0.0, 1.0);
			fe_clamp(m_Range, 0.0, 1.0);

			// Range must be 10%+
			if (m_Range < 0.1)
				m_Range = 1.0;
		}
	};

	struct InputState
	{
		InputState()
			: m_Down(false), m_Value(0.0f), m_ActiveRatio(0.0f), m_Active(false), m_ActiveFirst(false)
		{}
		InputState(bool active, float value, float active_ratio)
			: m_Down(active), m_Value(value), m_ActiveRatio(active_ratio), m_Active(active), m_ActiveFirst(active)
		{}
		
		bool m_Down; // Button / key was active during the step
		float m_Value; // Axis value, cursor position
		float m_ActiveRatio; // 0.0 to 1.0: indicates whether the button was pressed / released during the step

		// Used internally
		//unsigned char m_Step;
		bool m_Active; // Input is active right now
		bool m_ActiveFirst;
		//unsigned char m_TimesActivated; // Times activated during this step
		//unsigned char m_TimesChanged; // Times activated + times deactivated during this step

		inline bool IsActive() const { return m_Down; }
		inline float GetValue() const { return m_Value; }
		inline float GetActiveRatio() const { return m_ActiveRatio; }
	};

	class InputEvent
	{
	public:
		enum InputType
		{
			Binary,
			AnalogNormalized,
			AnalogAbsolute
		};

	public:
		InputEvent()
			: Down(false), Value(0.0)
		{}

	public:
		int Player;
		std::string Input;
		InputType Type;
		bool Down;
		double Value;
	};

	struct RawInputUserData
	{
		unsigned int Type;
		//std::string Name;
		unsigned int Index;
	};

	class RawInput
	{
	public:
		enum EventType
		{
			Nothing,
			Button,
			Axis,
			Pointer
		};

	public:
		RawInput()
			: DeviceIndex(0), Code(0),
			Shift(false), Control(false), Alt(false),
			CapsLock(false), NumLock(false), ScrollLock(false),
			InputType(Nothing),
			AxisPosition(0.0), ButtonPressed(false)
		{
		}
	public:
		std::string DeviceType;
		std::string DeviceName;
		unsigned int DeviceIndex;

		EventType InputType;

		int Code; // VK code (for buttons)
		// Key modifiers (for lazy people who don't want to keep track of these events themselves :P)
		bool Shift;
		bool Control;
		bool Alt;
		bool CapsLock;
		bool NumLock;
		bool ScrollLock;

		Vector2i PointerPosition;
		double AxisPosition;
		bool ButtonPressed;
	};

	/*!
	 * \brief
	 * Gets inputs based on bound keys.
	 *
	 * Fires input signals whenever a bound key is pressed. There is also a RawInput
	 * signal which is useful for making key-binding UIs.
	 */
	class InputManager : public FusionEngine::Singleton<InputManager>
	{
	public:
		//! Basic constructor.
		InputManager();
		//! Constructor.
		InputManager(const CL_DisplayWindow &window);
		//! Deconstructor
		~InputManager();

	public:
		//! Key shortname mapped to KeyInfo
		typedef std::tr1::unordered_map<std::string, KeyInfo> KeyInfoMap;

		//! Device / keycode pair type
		typedef std::pair<unsigned int, int> DeviceKeycodePair;
		typedef boost::hash<DeviceKeycodePair> DeviceKeycodePairHashFn;

		//! Keys mapped to bindings
		//typedef std::tr1::unordered_map<DeviceKeycodePair, InputBinding, DeviceKeycodePairHashFn> KeyBindingMap;
		typedef std::tr1::unordered_map<BindingKey, InputBinding> KeyBindingMap;

		//! Input shortnames mapped to input states
		typedef std::tr1::unordered_map<std::string, InputState> InputStateMap;
		//! Input state lists for each player (0 is player 1, etc.)
		typedef std::vector<InputStateMap> PlayerInputStateMaps;

		//! Current positions of each mouse
		typedef std::vector<CL_Point> MousePositionList;

#ifdef FSN_USE_XINPUT
		typedef std::vector<XInputController> XInputControllerList;
#endif

	public:
		void SetDisplayWindow(CL_DisplayWindow window);
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
		//! Begins ignoring input events. Call when going to the menu.
		void Suspend();

		void Update(float split);

		const InputDefinitionLoader *GetDefinitionLoader() const;

		//! Sets up inputs
		void LoadInputMaps(const std::string& filename);
		void SaveInputMaps(const std::string& filename);

		//! Binds a key to the given input for the given player
		/*!
		 * \param player
		 * The player who's input will be bound to this key [0-...)
		 *
		 * \param input_name
		 * Name for the input to be bound
		 *
		 * \param key_shortname
		 * The shortname for the key to be bound to the input
		 *
		 * \param controller_number
		 * The index of the XBox controller to use (for xinput only, obviously) [0, 4]
		 */
		void MapControl(unsigned int player, const std::string &input_name, const std::string &key_shortname, int controller_number = s_DeviceIndexAny);

		bool IsButtonDown(unsigned int player, const std::string& input_name) const;
		float GetAnalogValue(unsigned int player, const std::string& input_name) const;

		const InputStateMap &GetInputStateMapForPlayer(int player) const;
		const PlayerInputStateMaps &GetInputStateMaps() const;

		float GetMouseSensitivity() const;


		boost::signals2::signal<void (const InputEvent&)> SignalInputChanged;
		// Input continues to be pressed as a new step begins
		boost::signals2::signal<void (const InputEvent&)> SignalInputSustained;

		//! Fires on all input events, passing the raw input data
		/*!
		 * Useful for setting up inputs - the other input signals only fire
		 * when bound controls (keys, buttons, axis') are activated (pressed, moved).
		 */
		boost::signals2::signal<void (const RawInput&)> SignalRawInput;

	private:
#ifdef FSN_USE_XINPUT
		XInputControllerList m_XInputControllers;
#endif
		
		KeyInfoMap m_KeyInfo;
		KeyBindingMap m_KeyBindings;

		MousePositionList m_MicePositions;

		InputDefinitionLoader *m_DefinitionLoader;

		//! The InputHandler will not be considered active till this reaches zero.
		int m_SuspendRequests;

		unsigned char m_CurrentStep;

		LogPtr m_Log;

		CL_DisplayWindow m_DisplayWindow;
		mutable CL_InputContext m_InputContext;
		//! Slot container for inputs
		SlotContainer m_Slots;

		// Mouse movement multiplier
		float m_MouseSensitivity;

		// Used when polling mouse movement
		int m_DisplayCenterX;
		int m_DisplayCenterY;

		//! Loads human readable and UI control (key / button, etc) names
		void loadKeyInfo(const ticpp::Document& defDocument);
		//! Loads controls
		void loadControls(const ticpp::Document *const ctrlsDoc);
		void loadPlayerBinds(const ticpp::Element *const ctrls_root);

		//! Returns an iterator to a KeyInfo object for the given keyName
		/*!
		 * If there is no current KeyInfo listed for the given keyName
		 * and the given keyName string doesn't contain Last-Chance key-info
		 * data, an iterator to the end of the m_KeyInfo container is
		 * returned.
		 */
		KeyInfoMap::iterator findOrAddKeyInfo(const std::string &keyName);
		//! Adds key-info data for the given last-chance key string
		/*!
		 * \param[in] keyName
		 * A string containing last-chance key-info
		 *
		 * \param[out] _where
		 * Returns the iterator to the new key-info
		 */
		bool addLastChanceKeyInfo(const std::string &keyName, KeyInfoMap::iterator *_where = NULL);

		//! Saves controls
		void saveControls(ticpp::Document &ctrlsDoc);

		unsigned int getIndexOfControllerCalled(const std::string& name);

		void onKeyDown(const CL_InputEvent &ev, const CL_InputState &state);
		void onKeyUp(const CL_InputEvent &ev, const CL_InputState &state);

		void onMouseDown(const CL_InputEvent &ev, const CL_InputState &state);
		void onMouseUp(const CL_InputEvent &ev, const CL_InputState &state);
		void onMousePointerMove(const CL_InputEvent &ev, const CL_InputState &state);
		void onMouseBallMove(const CL_InputEvent &ev, const CL_InputState &state);

		void onGamepadPress(const CL_InputEvent &ev, const CL_InputState &state);
		void onGamepadRelease(const CL_InputEvent &ev, const CL_InputState &state);
		void onGamepadAxisMove(const CL_InputEvent &ev, const CL_InputState &state);

		void onXInputPress(const XInputEvent &ev);
		void onXInputRelease(const XInputEvent &ev);
		void onXInputAxisMove(const XInputEvent &ev);

		//! Invokes SignalRawInput
		void fireRawInput(const CL_InputEvent &ev, const CL_InputState &state, RawInputUserData device_info);
		//! Invokes SignalRawInput when a XInputEvent is received
		void fireRawInput_XInput(const XInputEvent& ev);

		//! Handle screen resize
		void onDisplayResize(int w, int h);
	};

}

#endif
