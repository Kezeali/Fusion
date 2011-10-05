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

#include "FusionStableHeaders.h"

#include "FusionInputHandler.h"

#include "FusionExceptionFactory.h"
#include "FusionPhysFS.h"
#include "FusionLogger.h"
#include "FusionStringFormatting.h"
#include "FusionXML.h"

namespace FusionEngine
{

	inline unsigned int DeviceNameToID(const std::string& device)
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

		else
			return s_DevNothing;
	}

	inline const char*const DeviceIDToName(unsigned int device)
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

		else
			return "";
	}

	InputManager::InputManager()
		: m_SuspendRequests(0),
		m_CurrentStep(0)
	{
		m_DefinitionLoader = new InputDefinitionLoader();

		m_Log = Logger::getSingletonPtr()->OpenLog(g_LogGeneral);
	}

	InputManager::InputManager(const CL_DisplayWindow &window)
		: m_SuspendRequests(0)
	{
		m_DefinitionLoader = new InputDefinitionLoader();
		m_InputContext = window.get_ic();
		m_DisplayWindow = window;

		m_Log = Logger::getSingletonPtr()->OpenLog(g_LogGeneral);
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

		for (unsigned int i = 0; i < (unsigned int)m_InputContext.get_keyboard_count(); i++)
		{
			m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_down(), this, &InputManager::onKeyDown);
			m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_up(), this, &InputManager::onKeyUp);

			// RawInput signals
			RawInputUserData ud;
			ud.Index = i;
			ud.Type = s_DevKeyboard;
			m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_down(), this, &InputManager::fireRawInput, ud);
			m_Slots.connect(m_InputContext.get_keyboard(i).sig_key_up(), this, &InputManager::fireRawInput, ud);
		}
		for (unsigned int i = 0; i < (unsigned int)m_InputContext.get_mouse_count(); i++)
		{
			//m_Slots.connect(m_InputContext.get_mouse(i).sig_axis_move(), this, &InputManager::onMouseAxisMove);
			//m_Slots.connect(m_InputContext.get_mouse(i).sig_ball_move(), this, &InputManager::onMouseBallMove);
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_down(), this, &InputManager::onMouseDown);
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_up(), this, &InputManager::onMouseUp);
			m_Slots.connect(m_InputContext.get_mouse(i).sig_pointer_move(), this, &InputManager::onMousePointerMove);

			// Raw-input signals
			RawInputUserData ud;
			ud.Index = i;
			ud.Type = s_DevMouse;
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_down(), this, &InputManager::fireRawInput, ud);
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_up(), this, &InputManager::fireRawInput, ud);
			ud.Type = s_DevMouse_Pointer;
			m_Slots.connect(m_InputContext.get_mouse(i).sig_pointer_move(), this, &InputManager::fireRawInput, ud);
		}
		for (unsigned int i = 0; i < (unsigned int)m_InputContext.get_joystick_count(); i++)
		{
			m_Slots.connect(m_InputContext.get_joystick(i).sig_axis_move(), this, &InputManager::onGamepadAxisMove);
			m_Slots.connect(m_InputContext.get_joystick(i).sig_key_down(), this, &InputManager::onGamepadPress);
			m_Slots.connect(m_InputContext.get_joystick(i).sig_key_up(), this, &InputManager::onGamepadRelease);

			// Raw-input signals
			RawInputUserData ud;
			ud.Index = i;
			ud.Type = s_DevGamepad_Axis;
			m_Slots.connect(m_InputContext.get_mouse(i).sig_axis_move(), this, &InputManager::fireRawInput, ud);
			ud.Type = s_DevGamepad;
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_down(), this, &InputManager::fireRawInput, ud);
			m_Slots.connect(m_InputContext.get_mouse(i).sig_key_up(), this, &InputManager::fireRawInput, ud);
		}
#ifdef FSN_USE_XINPUT
		// List all controllers
		for (unsigned int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XInputController c(i);
			m_XInputControllers.push_back( c );

			m_Slots.connect(c.sig_axis_move, this, &InputManager::onXInputAxisMove);
			m_Slots.connect(c.sig_key_down, this, &InputManager::onXInputPress);
			m_Slots.connect(c.sig_key_up, this, &InputManager::onXInputRelease);

			// Raw-input signals
			m_Slots.connect(c.sig_axis_move, this, &InputManager::fireRawInput_XInput);
			m_Slots.connect(c.sig_key_down, this, &InputManager::fireRawInput_XInput);
			m_Slots.connect(c.sig_key_up, this, &InputManager::fireRawInput_XInput);
		}
#endif

		m_Slots.connect(m_DisplayWindow.sig_resize(), this, &InputManager::onDisplayResize);

		//int numPlayers = 0;
		//if (!cliOpts->GetOption("num_local_players", &numPlayers))
		//	FSN_EXCEPT(ExCode::IO, "Options file is missing 'num_local_players'");
		try
		{
			// Load the input-definitions file for the current game
			ticpp::Document inputDoc(OpenXml_PhysFS("inputdefinitions.xml"));
			m_DefinitionLoader->Load(inputDoc);
			// Load the core key-info file
			ticpp::Document keyDoc(OpenXml_PhysFS("core/input/keyinfo.xml"));
			loadKeyInfo(keyDoc);
			// Load the controls file - will probably be in the user's home folder (the virtual file-system (PhysFS) takes care of this)
			LoadInputMaps("controls.xml");
		}
		catch (FileSystemException &ex)
		{
			m_Log->AddEntry(ex.ToString(), LOG_NORMAL);
		}
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

	void InputManager::Update(float split)
	{
		//m_PlayerInputStates.assign(m_NumPlayers, InputStateMap());


		if (m_SuspendRequests == 0)
		{
			m_CurrentStep++;

#ifdef FSN_USE_XINPUT
			//m_TimeSinceLastPoll += split;
			//if (m_TimeSinceLastPoll >= m_PollingInterval)
			{
				//m_TimeSinceLastPoll = 0.f;
				for (XInputControllerList::iterator it = m_XInputControllers.begin(), end = m_XInputControllers.end();
					it != end; ++it)
				{
					it->Poll();
				}
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

	const InputDefinitionLoader *InputManager::GetDefinitionLoader() const
	{
		return m_DefinitionLoader;
	}

	void InputManager::LoadInputMaps(const std::string &filename)
	{
		TiXmlDocument* inner = nullptr;
		// Create the file if it doesn't exist
		if (PHYSFS_exists(filename.c_str()) == 0)
		{
			std::vector<char> buffer = PhysFSHelp::copy_file("default-" + filename, filename);
			inner = new TiXmlDocument();
			inner->Parse(buffer.data());
		}
		else
			inner = OpenXml_PhysFS(filename);
		// Read the file
		ticpp::Document doc(inner);
		loadControls(&doc);
	}

	void InputManager::SaveInputMaps(const std::string& filename)
	{
		ticpp::Document doc;
		
		saveControls(doc);

		SaveString_PhysFS(doc.Value(), filename);
	}

	void InputManager::loadControls(const ticpp::Document *const ctrlsDoc)
	{
		ticpp::Element* rootElem = ctrlsDoc->FirstChildElement();

		ticpp::Iterator< ticpp::Element > child("binds");
		for ( child = child.begin( rootElem ); child != child.end(); child++ )
		{
			loadPlayerBinds(child.Get());
		}
	}

	void InputManager::loadPlayerBinds(const ticpp::Element *const bindsElem)
	{
		// Get the player number for this group
		std::string player = bindsElem->GetAttribute("player");
		if (!fe_issimplenumeric(player))
			return;
		unsigned int playerNum = CL_StringHelp::local8_to_uint(player.c_str()) - 1;

		ticpp::Iterator< ticpp::Element > child("bind");
		for ( child = child.begin( bindsElem ); child != child.end(); child++ )
		{
			// command and input are synonyms
			std::string input = child->GetAttribute("input");
			if (input.empty())
				input = child->GetAttribute("command");
			if (input.empty()) continue;

			// The key shortname (unique identifier)
			std::string keyName = child->GetAttribute("key");

			// Check that the given input is defined
			if (!m_DefinitionLoader->IsDefined(input))
			{
				m_Log->AddEntry("Can't bind " + keyName + " to " + input + ": " + input + " doesn't exist.", LOG_TRIVIAL);
				continue;
			}

			// Controller number (for XInput)
			//! \todo Perhaps non-xinput controlers should be accessable by device name?
			std::string contrlName = child->GetAttribute("controller_number");

			double threshold = 0.0;
			double range = 1.0;
			bool cubic = false;
			child->GetAttribute("threshold", &threshold, false);
			child->GetAttribute("range", &range, false);
			child->GetAttribute("cubic", &cubic, false);


			KeyInfoMap::const_iterator _where = findOrAddKeyInfo(keyName);
			if (_where == m_KeyInfo.end())
			{
				// Invalid shortname: log error and skip
				m_Log->AddEntry("Can't bind " + keyName + " to " + input + ": " + keyName + " doesn't exist.", LOG_TRIVIAL);
				continue;
			}
			const KeyInfo &key = _where->second;
			
			unsigned int deviceId = DeviceNameToID(key.m_Device);
			if (deviceId == s_DevNothing)
			{
				// Invalid shortname: log error and skip
				m_Log->AddEntry("Can't bind " + keyName + " to " + input + ": the device '" + key.m_Device + "' doesn't exist.", LOG_TRIVIAL);
				continue;
			}

			// Get the controller index
			unsigned int ctrlNum = 0;
			if (deviceId == s_DevXInput || deviceId == s_DevXInput_Axis)
			{
				ctrlNum = 0; // Default to first controller
				if (!contrlName.empty())
				{
					if (contrlName == "any")
						ctrlNum = XUSER_INDEX_ANY;
					else
						ctrlNum = CL_StringHelp::local8_to_uint(contrlName.c_str());
				}
			}
			else
			{
				ctrlNum = s_DeviceIndexAny; // Default to 'any' controller
			}

			InputBinding inputBinding(playerNum, input, key, threshold, range, cubic);
			inputBinding.Validate();
			m_KeyBindings[BindingKey(deviceId, ctrlNum, key.m_Code)] = inputBinding;
		}
	}

	InputManager::KeyInfoMap::iterator InputManager::findOrAddKeyInfo(const std::string &keyName)
	{
		KeyInfoMap::iterator _where = m_KeyInfo.find(keyName);
		if (_where == m_KeyInfo.end())
			addLastChanceKeyInfo(keyName, &_where);

		return _where;
	}

	bool InputManager::addLastChanceKeyInfo(const std::string &keyName, InputManager::KeyInfoMap::iterator *_where)
	{
		// Check for inline key-info
		StringVector keyInfoTokens = fe_splitstring(keyName, ":");
		if (keyInfoTokens.size() > 1)
		{
			const std::string& inline_DevName = keyInfoTokens[0];
			if (DeviceNameToID(inline_DevName) != s_DevNothing)
			{
				const std::string& inline_KeyCode = keyInfoTokens[1];
				int keyCodeInt = CL_StringHelp::local8_to_int(inline_KeyCode.c_str());

				KeyInfoMap::value_type mapping(keyName, KeyInfo(keyName, inline_DevName, keyCodeInt, keyName));
				std::pair<KeyInfoMap::iterator, bool> pib = m_KeyInfo.insert( mapping );
				if (_where != NULL)
					*_where = pib.first;
				
				return true;
			}
		}
		// Didn't find valid key-info
		return false;
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
			const BindingKey &key = it->first;
			InputBinding &binding = it->second;
			binding.Validate();

			// Get / create the group element for the player this bind applies to
			ticpp::Element* playerElement = getPlayerElement(binding.m_Player, playerElements, root);

			// Create the key bind
			ticpp::Element* bindElement = new ticpp::Element("bind");

			bindElement->SetAttribute("key", binding.m_Key.m_Name);

			if (key.device == s_DevXInput || key.device == s_DevXInput_Axis)
				bindElement->SetAttribute("controller_number", key.index);

			bindElement->SetAttribute("input", binding.m_Input);

			// These are optional attributes, so in order to keep the element
			//  un-cluttered they are only added if they are set to a value
			//  other than their default.
			if (binding.m_Threshold > 0.0)
				bindElement->SetAttribute("threshold", binding.m_Threshold);
			if (binding.m_Range < 1.0)
				bindElement->SetAttribute("range", binding.m_Range);
			if (binding.m_Cubic)
				bindElement->SetAttribute("cubic", "true");

			playerElement->LinkEndChild(bindElement);
		}
	}

	void InputManager::MapControl(unsigned int player, const std::string &input, const std::string &key_shortname, int controller_number)
	{
		if (!m_DefinitionLoader->IsDefined(input))
			FSN_EXCEPT(ExCode::InvalidArgument, input + " doesn't exist");

		//std::string playerStr(CL_StringHelp::int_to_local8(player));
		KeyInfoMap::iterator _where = findOrAddKeyInfo(key_shortname);
		if (_where != m_KeyInfo.end())
		{
			KeyInfo &key = _where->second;
			m_KeyBindings[BindingKey(DeviceNameToID(key.m_Device), controller_number, key.m_Code)] = InputBinding(player, input, key);
			//m_KeyBindings[shortname] = InputBinding(player, input, key);
		}
		else
			FSN_EXCEPT(ExCode::InvalidArgument, key_shortname + " doesn't exist");
	}

	bool InputManager::IsButtonDown(unsigned int player, const std::string &input) const
	{
		//const InputStateMap& inputStateMap = m_PlayerInputStates[player];
		//InputStateMap::const_iterator _where = inputStateMap.find(input);
		//if (_where != inputStateMap.end())
		//{
		//	const InputState &state = _where->second;
		//	
		//	return state.m_Down;
		//}

		return false;
	}

	float InputManager::GetAnalogValue(unsigned int player, const std::string &input) const
	{
		//const InputStateMap& inputStateMap = m_PlayerInputStates[player];
		//InputStateMap::const_iterator _where = inputStateMap.find(input);
		//if (_where != inputStateMap.end())
		//{
		//	const InputState &state = _where->second;

		//	return state.m_Value;
		//}

		return 0.f;
	}

	float InputManager::GetMouseSensitivity() const
	{
		return m_MouseSensitivity;
	}

	// Checks which key is actually being pressed for keys with multiple locations
	// TODO: catch these by calling GetAsyncKeyState in InputManager::Update(...)
	static int specifySide(int id)
	{
#ifdef _WIN32
		switch (id)
		{
		case CL_KEY_SHIFT:
			if (GetKeyState(VK_LSHIFT))
				id = VK_LSHIFT;
			else if (GetKeyState(VK_RSHIFT))
				id = VK_RSHIFT;
			break;
		case CL_KEY_CONTROL:
			if (GetKeyState(VK_LCONTROL))
				id = VK_LCONTROL;
			else if (GetKeyState(VK_RCONTROL))
				id = VK_RCONTROL;
			break;
		case CL_KEY_MENU:
			if (GetKeyState(VK_LMENU))
				id = VK_LMENU;
			else if (GetKeyState(VK_RMENU))
				id = VK_RMENU;
			break;
		}		
#endif
		return id;
	}

	void InputManager::onKeyDown(const CL_InputEvent &event, const CL_InputState &state)
	{
		if (event.repeat_count == 0)
		{
			auto _where = m_KeyBindings.find(BindingKey(s_DevKeyboard, s_DeviceIndexAny, event.id));
			if (_where != m_KeyBindings.end())
			{
				InputEvent synthedEvent;
				synthedEvent.Player = _where->second.m_Player;
				synthedEvent.Input = _where->second.m_Input;

				synthedEvent.Type = InputEvent::Binary;
				synthedEvent.Value = 0.0;
				synthedEvent.Down = true;

				SignalInputChanged(synthedEvent);

				//processInputEvent(s_DevKeyboard, event);
			}
		}
	}

	void InputManager::onKeyUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevKeyboard, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = false;

			SignalInputChanged(synthedEvent);

			//processInputEvent(s_DevKeyboard, event);
		}
	}

	void InputManager::onMouseDown(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = true;

			SignalInputChanged(synthedEvent);

			//processInputEvent(s_DevKeyboard, event);
		}
	}

	void InputManager::onMouseUp(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = false;

			SignalInputChanged(synthedEvent);

			//processInputEvent(s_DevKeyboard, event);
		}
	}

	void InputManager::onMousePointerMove(const CL_InputEvent &event, const CL_InputState &state)
	{
		// Check for pointer-x bindings
		{
			KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse_Pointer, s_DeviceIndexAny, 0));
			if (_where != m_KeyBindings.end())
			{
				const InputBinding& binding = _where->second;

				InputEvent synthedEvent;
				synthedEvent.Player = binding.m_Player;
				synthedEvent.Input = binding.m_Input;

				synthedEvent.Type = InputEvent::AnalogAbsolute;
				synthedEvent.Value = (double)event.mouse_pos.x;
				synthedEvent.Down = false;

				SignalInputChanged(synthedEvent);
			}
		}

		// Check for pointer-y bindings
		{
			KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse_Pointer, s_DeviceIndexAny, 1));
			if (_where != m_KeyBindings.end())
			{
				const InputBinding& binding = _where->second;

				InputEvent synthedEvent;
				synthedEvent.Player = binding.m_Player;
				synthedEvent.Input = binding.m_Input;

				synthedEvent.Type = InputEvent::AnalogAbsolute;
				synthedEvent.Value = (double)event.mouse_pos.y;
				synthedEvent.Down = false;

				SignalInputChanged(synthedEvent);
			}
		}

		// Check for delta-x bindings
		{
			KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse_Pointer, s_DeviceIndexAny, 2));
			if (_where != m_KeyBindings.end())
			{
				const InputBinding& binding = _where->second;

				InputEvent synthedEvent;
				synthedEvent.Player = binding.m_Player;
				synthedEvent.Input = binding.m_Input;

				double normalizedValue = (double)(event.mouse_pos.x - m_MicePositions[0].x);
				normalizedValue = normalizedValue / m_DisplayWindow.get_viewport().get_width();
				binding.FilterValue(normalizedValue);

				// If the normalized value is zero, no event will be fired
				if (!fe_fzero(normalizedValue))
				{
					synthedEvent.Type = InputEvent::AnalogNormalized;
					synthedEvent.Value = normalizedValue;
					synthedEvent.Down = false;

					SignalInputChanged(synthedEvent);
				}
			}
		}

		// Check for delta-y bindings
		{
			KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevMouse_Pointer, s_DeviceIndexAny, 3));
			if (_where != m_KeyBindings.end())
			{
				const InputBinding& binding = _where->second;

				InputEvent synthedEvent;
				synthedEvent.Player = binding.m_Player;
				synthedEvent.Input = binding.m_Input;

				double normalizedValue = (double)(event.mouse_pos.y - m_MicePositions[0].y);
				normalizedValue = normalizedValue / m_DisplayWindow.get_viewport().get_height();
				binding.FilterValue(normalizedValue);

				if (!fe_fzero(normalizedValue))
				{
					synthedEvent.Type = InputEvent::AnalogNormalized;
					synthedEvent.Value = normalizedValue;
					synthedEvent.Down = false;

					SignalInputChanged(synthedEvent);
				}
			}
		}
	}

	void InputManager::onMouseBallMove(const CL_InputEvent &event, const CL_InputState &state)
	{
		// Mouse ball events are never fired in the Windows impl. of ClanLib, so this is useless
	}

	void InputManager::onGamepadPress(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevGamepad, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = true;

			SignalInputChanged(synthedEvent);

			//processInputEvent(s_DevKeyboard, event);
		}
	}

	void InputManager::onGamepadRelease(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevGamepad, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = true;

			SignalInputChanged(synthedEvent);

			//processInputEvent(s_DevKeyboard, event);
		}
	}

	void InputManager::onGamepadAxisMove(const CL_InputEvent &event, const CL_InputState &state)
	{
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevGamepad_Axis, s_DeviceIndexAny, event.id));
		if (_where != m_KeyBindings.end())
		{
			const InputBinding& binding = _where->second;

			InputEvent synthedEvent;
			synthedEvent.Player = binding.m_Player;
			synthedEvent.Input = binding.m_Input;

			synthedEvent.Type = InputEvent::AnalogNormalized;
			synthedEvent.Value = event.axis_pos;
			synthedEvent.Down = false;
			// Filter the value (apply deadzone / range / NLT)
			binding.FilterValue(synthedEvent.Value);

			SignalInputChanged(synthedEvent);
		}
	}

	void InputManager::onXInputPress(const XInputEvent &event)
	{
		unsigned int index = event.controller->GetUserIndex();
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevXInput, index, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = true;

			SignalInputChanged(synthedEvent);
		}
	}

	void InputManager::onXInputRelease(const XInputEvent &event)
	{
		unsigned int index = event.controller->GetUserIndex();
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevXInput, index, event.id));
		if (_where != m_KeyBindings.end())
		{
			InputEvent synthedEvent;
			synthedEvent.Player = _where->second.m_Player;
			synthedEvent.Input = _where->second.m_Input;

			synthedEvent.Type = InputEvent::Binary;
			synthedEvent.Value = 0.0;
			synthedEvent.Down = false;

			SignalInputChanged(synthedEvent);
		}
	}

	void InputManager::onXInputAxisMove(const XInputEvent &event)
	{
		unsigned int index = event.controller->GetUserIndex();
		KeyBindingMap::iterator _where = m_KeyBindings.find(BindingKey(s_DevXInput_Axis, index, event.id));
		if (_where != m_KeyBindings.end())
		{
			const InputBinding& binding = _where->second;

			InputEvent synthedEvent;
			synthedEvent.Player = binding.m_Player;
			synthedEvent.Input = binding.m_Input;

			synthedEvent.Type = InputEvent::AnalogNormalized;
			synthedEvent.Value = event.axis_pos;
			synthedEvent.Down = false;
			// Filter the value (apply deadzone / range / NLT)
			binding.FilterValue(synthedEvent.Value);

			SignalInputChanged(synthedEvent);
		}
	}

	void InputManager::fireRawInput(const CL_InputEvent &ev, const CL_InputState &state, RawInputUserData dev_info)
	{
		RawInput rawInput;
		rawInput.DeviceType = DeviceIDToName(dev_info.Type);
		rawInput.DeviceIndex = dev_info.Index;
		rawInput.DeviceName = CL_StringHelp::text_to_local8(ev.device.get_name()); //= dev_info.Name;

		rawInput.Shift = ev.shift;
		rawInput.Control = ev.ctrl;
		rawInput.Alt = ev.alt;
		// TODO: capslock, numlock, scrolllock

		if (ev.type == CL_InputEvent::pointer_moved)
		{
			rawInput.InputType = RawInput::Pointer;
			rawInput.Code = 0;
			
			rawInput.PointerPosition = Vector2T<int>(ev.mouse_pos.x, ev.mouse_pos.y);
			//rawInput.AxisPosition = 0.0;
			//rawInput.ButtonPressed = false;
		}
		else if (ev.type == CL_InputEvent::axis_moved)
		{
			rawInput.InputType = RawInput::Axis;
			rawInput.Code = ev.id;
			
			//rawInput.PointerPosition = Vector2T<int>();
			rawInput.AxisPosition = ev.axis_pos;
			//rawInput.ButtonPressed = false;
		}
		else if (ev.type == CL_InputEvent::pressed)
		{
			rawInput.InputType = RawInput::Button;
			rawInput.Code = ev.id;
			
			if (ev.device.get_type() == CL_InputDevice::pointer)
				rawInput.PointerPosition = Vector2T<int>(ev.mouse_pos.x, ev.mouse_pos.y);
			//rawInput.AxisPosition = 0.0;
			rawInput.ButtonPressed = true;
		}
		else if (ev.type == CL_InputEvent::released)
		{
			rawInput.InputType = RawInput::Button;
			rawInput.Code = ev.id;
			
			if (ev.device.get_type() == CL_InputDevice::pointer)
				rawInput.PointerPosition = Vector2T<int>(ev.mouse_pos.x, ev.mouse_pos.y);
			//rawInput.AxisPosition = 0.0;
			rawInput.ButtonPressed = false;
		}
		else if (ev.type == CL_InputEvent::doubleclick)
		{
			rawInput.InputType = RawInput::Button;
			rawInput.Code = ev.id;
			
			rawInput.PointerPosition = Vector2T<int>(ev.mouse_pos.x, ev.mouse_pos.y);
			//rawInput.AxisPosition = 0.0;
			rawInput.ButtonPressed = false;
		}
		else
		{
			rawInput.InputType = RawInput::Nothing;
			rawInput.Code = ev.id;
		}

		SignalRawInput(rawInput);
	}

	void InputManager::fireRawInput_XInput(const XInputEvent &ev)
	{
		RawInput rawInput;
		if (ev.type == XInputEvent::axis_moved)
			rawInput.DeviceType = s_DevXInput_AxisStr;
		else
			rawInput.DeviceType = s_DevXInputStr;

		rawInput.DeviceIndex = ev.controller->GetUserIndex();
		rawInput.DeviceName = makestring() << "Xbox 360 Gamepad " << rawInput.DeviceIndex;

		if (ev.type == XInputEvent::axis_moved)
		{
			rawInput.InputType = RawInput::Axis;
			rawInput.Code = ev.id;
			
			//rawInput.PointerPosition = Vector2T<int>();
			rawInput.AxisPosition = ev.axis_pos;
			//rawInput.ButtonPressed = false;
		}
		else if (ev.type == XInputEvent::pressed)
		{
			rawInput.InputType = RawInput::Button;
			rawInput.Code = ev.id;
			
			//rawInput.PointerPosition = Vector2T<int>();
			//rawInput.AxisPosition = 0.0;
			rawInput.ButtonPressed = true;
		}
		else if (ev.type == XInputEvent::released)
		{
			rawInput.InputType = RawInput::Button;
			rawInput.Code = ev.id;
			
			//rawInput.PointerPosition = Vector2T<int>();
			//rawInput.AxisPosition = 0.0;
			rawInput.ButtonPressed = false;
		}
		else
		{
			rawInput.InputType = RawInput::Nothing;
			rawInput.Code = ev.id;
		}

		SignalRawInput(rawInput);
	}

	void InputManager::onDisplayResize(int w, int h)
	{
		m_DisplayCenterX = w >> 1;
		m_DisplayCenterY = h >> 1;
	}

}
