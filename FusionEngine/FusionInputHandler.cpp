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

#include "FusionInputHandler.h"

//#include "FusionInputPluginLoader.h"
#include "FusionLogger.h"
#include "FusionXML.h"

namespace FusionEngine
{

	// Config file device identifiers
	static const char* s_DevKeyboardStr = "keyboard";
	static const char* s_DevGamepadStr = "gamepad";
	static const char* s_DevGamepad_AxisStr = "gamepad-axis";
	static const char* s_DevMouseStr = "mouse";
	static const char* s_DevMouse_PointerStr = "mouse-pointer";
	static const char* s_DevMouse_AxisStr = "mouse-axis";
	static const char* s_DevXInputStr = "xinput";
	static const char* s_DevXInput_AxisStr = "xinput-axis";

	// Internal device identifiers (used by input manager)
	static const int s_DevKeyboard = 0;
	static const int s_DevGamepad = 1;
	static const int s_DevGamepad_Axis = 2;
	static const int s_DevMouse = 3;
	static const int s_DevMouse_Pointer = 4;
	static const int s_DevMouse_Axis = 5;
	static const int s_DevXInput = 10;
	static const int s_DevXInput_Axis = 11;

	static inline int DeviceNameToID(const std::string& device)
	{
		if (device == s_DevKeyboardStr)
			return s_DevKeyboard;

		else if (device == s_DevGamepadStr)
			return s_DevGamepad;
		else if (device == s_DevGamepad_AxisStr)
			return s_DevGamepad_Axis;

		else if (device == s_DevMouseStr)
			return s_DevMouse;
		else if (device == s_DevMouse_PointerStr)
			return s_DevMouse_Pointer;
		else if (device == s_DevMouse_AxisStr)
			return s_DevMouse_Axis;

		else if (device == s_DevXInputStr)
			return s_DevXInput;
		else if (device == s_DevXInput_AxisStr)
			return s_DevXInput_Axis;
	}

	static inline const std::string& DeviceIDToName(int device)
	{
		if (device == s_DevKeyboard)
			return s_DevKeyboardStr;

		else if (device == s_DevGamepad)
			return s_DevGamepadStr;
		else if (device == s_DevGamepad_Axis)
			return s_DevGamepad_AxisStr;

		else if (device == s_DevMouse)
			return s_DevMouseStr;
		else if (device == s_DevMouse_Pointer)
			return s_DevMouse_PointerStr;
		else if (device == s_DevMouse_Axis)
			return s_DevMouse_AxisStr;

		else if (device == s_DevXInput)
			return s_DevXInputStr;
		else if (device == s_DevXInput_Axis)
			return s_DevXInput_AxisStr;
	}

	static const int s_DeviceIndexAny = 255;

	InputManager::InputManager()
		: m_SuspendRequests(0),
		m_CurrentStep(0)
	{
		m_DefinitionLoader = new InputDefinitionLoader();
	}

	InputManager::InputManager(CL_DisplayWindow window)
		: m_SuspendRequests(0)
	{
		m_DefinitionLoader = new InputDefinitionLoader();
		m_InputContext = window.get_ic();
		m_DisplayWindow = window;
	}

	InputManager::~InputManager()
	{
		delete m_DefinitionLoader;
	}

	void InputManager::SetDisplayWindow(CL_DisplayWindow window)
	{
		m_DisplayWindow = window;
		m_InputContext = window.get_ic();
	}

	bool InputManager::Test()
	{
		// Keyboard is always required
		if (m_InputContext.get_keyboard_count() == 0)
			return false;		

		return true;
	}

	void InputManager::Initialise()
	{
		m_SuspendRequests = 0;

		if (m_InputContext.get_keyboard_count() > 0)
		{
			for (int i = 0; i < m_InputContext.get_keyboard_count(); i++)
			{
				m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_down(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_up(), this, &InputManager::onInputEvent);
			}
		}
		if (m_InputContext.get_mouse_count() > 0)
		{
			for (int i = 0; i < m_InputContext.get_mouse_count(); i++)
			{
				m_Slots.connect(m_InputContext.get_mouse(i).sig_axis_move(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_mouse(i).sig_ball_move(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_mouse(i).sig_key_down(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_mouse(i).sig_key_up(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_mouse(i).sig_pointer_move(), this, &InputManager::onInputEvent);
			}
		}
		if (m_InputContext.get_joystick_count() > 0)
		{
			for (int i = 0; i < m_InputContext.get_joystick_count(); i++)
			{
				m_Slots.connect(m_InputContext.get_joystick(i).sig_axis_move(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_joystick(i).sig_key_down(), this, &InputManager::onInputEvent);
				m_Slots.connect(m_InputContext.get_joystick(i).sig_key_up(), this, &InputManager::onInputEvent);
			}
		}
#ifdef FSN_USE_XINPUT
		// List all controllers
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			m_XInputControllers.push_back( XInputController(i) );
		}
#endif

		m_Slots.connect(m_DisplayWindow.sig_resize(), this, &InputManager::onDisplayResize);

		//int numPlayers = 0;
		//if (!cliOpts->GetOption("num_local_players", &numPlayers))
		//	FSN_EXCEPT(ExCode::IO, "InputManager::Initialise", "Options file is missing 'num_local_players'");
		
		ticpp::Document inputDoc(OpenXml_PhysFS(L"input/coreinputs.xml"));
		m_DefinitionLoader->Load(inputDoc);
		const InputDefinitionLoader::InputDefinitionMap &inputs = m_DefinitionLoader->GetInputDefinitions();

		ticpp::Document keyDoc(OpenXml_PhysFS(L"input/keys.xml"));
		loadKeyInfo(keyDoc);

		LoadInputMaps(L"controls.xml");
	}

	void InputManager::loadKeyInfo(const ticpp::Document &doc)
	{
		ticpp::Element* elem = doc.FirstChildElement();

		if (elem->Value() != "keyinfo")
			throw FileTypeException("InputManager::loadKeyInfo", "Not a keyinfo definition file", __FILE__, __LINE__);

		ticpp::Iterator< ticpp::Element > child( "key" );
		for ( child = child.begin( elem ); child != child.end(); child++ )
		{
			KeyInfo kinfo;
			kinfo.m_Device = child->GetAttributeOrDefault("device", "keyboard");
			child->GetAttributeOrDefault("id", &kinfo.m_Code, 0);
			kinfo.m_Name = child->GetAttribute("shortname");
			kinfo.m_Description = child->GetAttributeOrDefault("name", kinfo.m_Name);

			if (kinfo.m_Name.empty())
				throw FileTypeException("InputManager::loadKeyInfo", "The keyinfo document contains incomplete tags", __FILE__, __LINE__);

			m_KeyInfo[kinfo.m_Name] = kinfo;
		}
	}

	void InputManager::CleanUp()
	{
		m_DefinitionLoader->Clear();

		m_KeyInfo.clear();
		m_KeyBindings.clear();

		m_Slots.disconnect_all();

		//m_InputBindings.clear();
	}

	class update_state
	{
	public:
		update_state ( )
		{
		}

		void operator() (const InputState &elem )
		{
			elem.m_ActiveRatio = elem.m_Active ? 1.f : 0.f;
			elem.m_ActiveFirst = elem.m_Active;
		}
	};

	void InputManager::Update(unsigned int split)
	{
		//m_PlayerInputStates.assign(m_NumPlayers, InputStateMap());

		// Perhaps processing should be limited by:
		//  if ((m_TicksToNextStep - split) < 0)
		//  {
		//    m_TicksToNextStep = m_MinTicksPerStep;
		//  ...
		if (m_SuspendRequests == 0)
		{
			m_CurrentStep++;

			for (PlayerInputStateMaps::iterator it = m_PlayerInputStates.begin(), end = m_PlayerInputStates.end();
				it != end; ++it)
			{
				std::for_each(it->begin(), it->end(), update_state);
			}

#ifdef FSN_USE_XINPUT
			for (XInputKeyBindingMap::iterator it = m_XInputBindings.begin(), end = m_XInputBindings.end();
				it != end; ++it)
			{
			}
#endif
		}
	}

	void InputManager::Activate()
	{
		if (--m_SuspendRequests < 0)
			m_SuspendRequests = 0;
	}

	void InputManager::Suspend()
	{
		m_SuspendRequests++;
	}

	void InputManager::LoadInputMaps(const std::wstring &filename)
	{
		// Read file
		ticpp::Document doc(OpenXml_PhysFS(filename));

		loadControls(&doc);
	}

	void InputManager::SaveInputMaps(const std::wstring& filename)
	{
		ticpp::Document doc;
		
		saveControls(doc);

		SaveString_PhysFS(doc.Value(), filename);
	}

	void InputManager::loadControls(const ticpp::Document const* ctrlsDoc)
	{
		ticpp::Iterator< ticpp::Element > child("binds");
		for ( child = child.begin( &ctrlsDoc ); child != child.end(); child++ )
		{
			loadPlayerBinds(child.Get());
		}
	}

	void InputManager::loadPlayerBinds(const ticpp::Element const* bindsElem)
	{
		// Get the player number for this group
		std::string player = bindsElem.GetAttribute("player");
		if (!fe_issimplenumeric(player))
			return;
		unsigned int playerNum = CL_StringHelp::local8_to_uint(player);

		ticpp::Iterator< ticpp::Element > child("bind");
		for ( child = child.begin( &bindsElem ); child != child.end(); child++ )
		{
			// command and input are synonyms
			std::string input = child->GetAttribute("input");
			if (input.empty())
				input = child->GetAttribute("command");
			if (input.empty()) continue;

			// The key shortname (unique identifier)
			std::string keyName = child->GetAttribute("key");

			std::string ctrlNum = child->GetAttribute("controller_number");


			KeyInfoMap::const_iterator _where = m_KeyInfo.find(keyName);
			if (_where == m_KeyInfo.end())
			{
				// Invalid shortname -> log error and skip
				Logger::getSingleton().Add(
					"Can't bind " + keyName + " to " + input + ": " + keyName + " doesn't exist.",
					g_LogException, LOG_TRIVIAL);
				continue;
			}
			KeyInfo &key = m_KeyInfo[keyName];
			
			if (key.m_Device == s_DevXInputStr)
			{
				unsigned int ctrlNumInt = 0; // Default to first controller
				if (!ctrlNum.empty())
				{
					if (ctrlNum == "any")
						ctrlNumInt = XUSER_INDEX_ANY;
					else
						ctrlNumInt = CL_StringHelp::local8_to_uint(ctrlNum);
				}
				m_XInputBindings[XUserKeycodePair(ctrlNumInt, key.m_Code)] = InputBinding(playerNum, input, key);
			}
			else
				m_KeyBindings[DeviceKeycodePair(key.m_Device, key.m_Code)] = InputBinding(playerNum, input, key);
		}
	}

	typedef std::tr1::unordered_map<int, ticpp::Element*> PlayerElementMap;

	// Gets or creates (if needed) a player xml-element
	static ticpp::Element* getPlayerElement(int player, PlayerElementMap &playerElements, ticpp::Element *parent)
	{
		PlayerElementMap::const_iterator _where = playerElements.find(player);
		if (_where == playerElements.end())
		{
			ticpp::Element* playerElement = new ticpp::Element("binds");
			playerElements.insert(PlayerElementMap::value_type(player, playerElement));

			playerElement->SetAttribute("player", player);

			parent->LinkEndChild(playerElement);

			return playerElement;
		}
		else
		{
			return _where->second;
		}
	}

	void InputManager::saveControls(ticpp::Document& ctrlsDoc)
	{
		// Decl
		ticpp::Declaration *decl = new ticpp::Declaration( XML_STANDARD, "", "" );
		ctrlsDoc.LinkEndChild( decl ); 

		// Root
		ticpp::Element* root = new ticpp::Element("controls");
		ctrlsDoc.LinkEndChild( root );

		PlayerElementMap playerElements;
		for (KeyBindingMap::iterator it = m_KeyBindings.begin(), end = m_KeyBindings.end();
			it != end; ++it)
		{
			const InputBinding &binding = it->second;

			// Get / create the group element for the player this bind applies to
			ticpp::Element* playerElement = getPlayerElement(binding.m_Player, playerElements, root);

			// Create the key bind
			ticpp::Element* bindElement = new ticpp::Element("bind");

			bindElement->SetAttribute("key", binding.m_Key.m_Name);
			bindElement->SetAttribute("input", binding.m_Input);

			playerElement->LinkEndChild(bindElement);
		}
	}

	void InputManager::MapControl(unsigned int player, const std::string &input, const std::string &key_shortname)
	{
		//std::string playerStr(CL_StringHelp::int_to_local8(player));
		KeyInfo &key = m_KeyInfo[key_shortname];
		m_KeyBindings[DeviceKeycodePair(key.m_Device, key.m_Code)] = InputBinding(player, input, key);
		//m_KeyBindings[shortname] = InputBinding(player, input, key);
	}

	int InputManager::GetDeviceIdByName(const std::string &name) const
	{
		DeviceNameMap::iterator _where = m_DevNameToId.find(name);
		if (_where != m_DevNameToId.end())
			return _where;
	}

	//void InputManager::MapControl(int keysym, const std::string &name, unsigned int filter)
	//{
	//	std::string buttonName = "Unknown";
	//	CL_InputDevice dev = CL_Keyboard::get_device();
	//	buttonName = dev.get_key_name(keysym);

	//	m_ControlMap[CL_String::from_int(filter) + name] = 
	//		Control(CL_String::from_int(filter) + name, keysym, buttonName, dev);
	//}

	//void InputManager::MapControl(int keysym, const std::string &name, CL_InputDevice device, unsigned int filter)
	//{
	//	std::string buttonName = "Unknown";
	//	buttonName = device.get_key_name(keysym);

	//	m_ControlMap[CL_String::from_int(filter) + name] = 
	//		Control(CL_String::from_int(filter) + name, keysym, buttonName, device);
	//}

	//const Control& InputManager::GetControl(const std::string &name, unsigned int filter) const
	//{
	//	ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
	//	if (it != m_ControlMap.end())
	//		return (*it).second;

	//	throw Exception();
	//	//return m_ControlMap[CL_String::from_int(filter) + name];
	//}

	bool InputManager::IsButtonDown(unsigned int player, const std::string &input) const
	{
		const InputStateMap& inputStateMap = m_PlayerInputStates[player];
		InputStateMap::const_iterator _where = inputStateMap.find(input);
		if (_where != inputStateMap.end())
		{
			const InputState &state = _where->second;
			
			return state.m_Down;
		}

		return false;
	}

	float InputManager::GetAnalogValue(unsigned int player, const std::string &input) const
	{
		const InputStateMap& inputStateMap = m_PlayerInputStates[player];
		InputStateMap::const_iterator _where = inputStateMap.find(input);
		if (_where != inputStateMap.end())
		{
			const InputState &state = _where->second;

			return state.m_Value;
		}

		return 0.f;
	}

//	Command InputManager::CreateCommand(unsigned int player)
//	{
//		if (m_SuspendRequests != 0)
//			return Command();
//
//		//int localPlayers = 0;
//		//ClientOptions::getSingleton().GetOption("num_local_players", &localPlayers);
//
//#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
//		/////
//		// Event Input
//		///////
//
//		// Update the requested player's command for this tick to the current command for said player
//		Command &currentCommand = m_CurrentCommands[player];
//		m_PlayerCommands[player][tick % m_CommandBufferLength] = currentCommand;
//
//#elif FE_INPUT_METHOD == FE_INPUTMETHOD_BUFFERED
//		/////
//		// Buffered Input
//		///////
//		FE_EXCEPT(ExCode::Base, "InputManager::CreateCommand", "Buffered input is not implemented");
//
//#else
//		/////
//		// Unbuffered Input
//		///////
//
//		// Grab the command to be updated in the command history
//		//Command &tickCommand = m_PlayerCommands[player][tick%m_CommandBufferLength];
//
//		Command command;
//		// Update all the bindings in the command
//		for (KeyMap::iterator it = m_KeyBindings.begin(), end = m_KeyBindings.end(); it != end; ++it)
//		{
//			const std::string &keyName = it->first;
//			// Grab the key binding
//			InputBinding &binding = it->second;
//
//			CL_InputDevice &dev = GetDevice(binding.m_Key.m_Device);
//
//			bool nowDown = dev.get_keycode(binding.m_Key.m_Code);
//			float nowAxis = dev.get_axis(binding.m_Key.m_Code);
//			//! \todo Axis threshold option
//			Command::InputStatePtr state = command.GetInputState(binding.m_Input);
//			state->m_Changed = state->m_Down != nowDown || fe_fequal(state->m_Value, nowAxis, 0.1f);
//			state->m_Down = nowDown;
//			state->m_Value = nowAxis;
//		}
//
//		return command;
//#endif
//	}

	//const Command &InputManager::GetCommand(unsigned int player)
	//{
	//	return m_PlayerCommands[player];
	//}

	float InputManager::GetMouseSensitivity() const
	{
		return m_MouseSensitivity;
	}

	void InputManager::onKeyboardEvent(const CL_InputEvent &event, const CL_InputState &state)
	{
		processBinaryInputEvent("keyboard", event, state);
	}

	void InputManager::onMousePointerEvent(const CL_InputEvent &event, const CL_InputState &state)
	{
		// Check for pointer-x bindings
		KeyBindingMap::iterator _where = m_KeyBindings.find(DeviceKeycodePair("mouse-pointer", 0));
		if (_where != m_KeyInfo.end())
		{
			const InputBinding &binding = _where->second;

			InputState& inputState = m_PlayerInputStates[binding.m_Player][binding.m_Input];

			inputState.m_Value = (float)event.mouse_pos.x;
		}

		// Check for pointer-y bindings
		_where = m_KeyBindings.find(DeviceKeycodePair("mouse-pointer", 1));
		if (_where != m_KeyInfo.end())
		{
			const InputBinding &binding = _where->second;

			InputState& inputState = m_PlayerInputStates[binding.m_Player][binding.m_Input];

			inputState.m_Value = (float)event.mouse_pos.y;
		}
	}

	void InputManager::onMouseBallEvent(const CL_InputEvent &event, const CL_InputState &state)
	{
		// Check for ball-x bindings
		KeyBindingMap::iterator _where = m_KeyBindings.find(DeviceKeycodePair("mouse-axis", 0));
		if (_where != m_KeyInfo.end())
		{
			const InputBinding &binding = _where->second;

			InputState& inputState = m_PlayerInputStates[binding.m_Player][binding.m_Input];

			inputState.m_Value = (float)event.mouse_pos.x;
		}

		// Check for ball-y bindings
		_where = m_KeyBindings.find(DeviceKeycodePair("mouse-axis", 1));
		if (_where != m_KeyInfo.end())
		{
			const InputBinding &binding = _where->second;

			InputState& inputState = m_PlayerInputStates[binding.m_Player][binding.m_Input];

			inputState.m_Value = (float)event.mouse_pos.y;
		}
	}

	void InputManager::onGamepadAxisEvent(const CL_InputEvent &event, const CL_InputState &state)
	{
		processAxisInputEvent("gamepad-axis", event, state);
	}

	void InputManager::processBinaryInputEvent(const std::string &deviceName, const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(DeviceKeycodePair(deviceName, event.id));
		if (_where != m_KeyBindings.end())
		{
			const InputBinding &binding = _where->second;

			InputState& inputState = m_PlayerInputStates[binding.m_Player][binding.m_Input];
			
			inputState.m_Active = event.type == CL_InputEvent::pressed;

			if (event.type == CL_InputEvent::pressed)
			{
				// If a button is pressed at any time during a step,
				//  the given input is considered active for the entire
				//  step. Conversely if, and only if, a button is never pressed
				//  again during a step will it be set to inactive (false)
				inputState.m_Down = true;

				// If it was active at the start of this step, since we 
				//  are now receiving a 'pressed' event, it must have become
				//  inactive ('released') at some point in-between. Therefor
				//  this is most likely the 3rd state during this step so
				//  active ratio = 1/3 = 0.34 (rounded up. since the ratio
				//  increases every press and there may have been more
				//  than one press->release flip-flop during this step, even
				//  unlikely as that is). Otherwise the active ratio is
				//  0.5, no matter how many times it has been pressed and
				//  released (given that it is now being pressed.)
				inputState.m_ActiveRatio = inputState.m_ActiveFirst ? 0.34f : 0.5f;
			}
			if (event.type == CL_InputEvent::released)
			{
				// Converse of the above statement
				inputState.m_ActiveRatio = inputState.m_ActiveFirst ? 0.5f : 0.34f;
			}

			// Clamp ActiveRatio
			if (fe_fequal(inputState.m_ActiveRatio, 1.f, 0.05f))
				inputState.m_ActiveRatio = 1.f;
			else if (fe_fequal(inputState.m_ActiveRatio, 0.f, 0.05f))
				inputState.m_ActiveRatio = 0.f;

			inputState.m_Value = (inputState.m_Active ? 1.f : 0.f) * inputState.m_ActiveRatio;\
		}
	}

	void InputManager::onAxisInputEvent()
	{
		inputState.m_Value = event.axis_pos;
	}

//	void InputManager::onKeyDown(const CL_InputEvent &key)
//	{
//#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
//		if (m_SuspendRequests != 0)
//			return;
//
//		// Iterate player input lists to update the current commands
//		CommandList::iterator it = m_CurrentCommands.begin(), end = m_CurrentCommands.end();
//		for (; it != end; ++it)
//		{
//			//! \todo Reduce this to one map search. Maybe even use hash maps for speed
//			std::string &shortname = m_KeyInfo[key.id ^ (int)key.device.get_type()].m_Name;
//			InputBinding &binding = m_KeyBindings[shortname];
//			std::string &input = binding.m_Input;
//
//			InputState &state = it[input];
//			state.m_Changed = !state.m_Down || abs(state.m_Value - key.axis_pos) > 0.1f;
//			state.m_Down = true;
//			state.m_Value = key.axis_pos;
//		}
//#endif
//	}
//
//	void InputManager::onKeyUp(const CL_InputEvent &key)
//	{
//#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
//		if (m_SuspendRequests != 0)
//			return;
//
//		// Iterate player input lists to update the current commands
//		CommandList::iterator it = m_CurrentCommands.begin(), end = m_CurrentCommands.end();
//		for (; it != end; ++it)
//		{
//			//! \todo Reduce this to one map search. Maybe even use hash maps for speed
//			std::string &shortname = m_KeyInfo[key.id ^ (int)key.device.get_type()].m_Name;
//			InputBinding &binding = m_KeyBindings[shortname];
//			std::string &input = binding.m_Input;
//
//			InputState &state = it[input];
//			state.m_Changed = state.m_Down || abs(state.m_Value - key.axis_pos) > 0.1f;
//			state.m_Down = false;
//			state.m_Value = key.axis_pos;
//		}
//#endif
//	}

	void InputManager::onDisplayResize(int w, int h)
	{
		m_DisplayCenterX = w >> 1;
		m_DisplayCenterY = h >> 1;
	}

}
