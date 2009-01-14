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

#include "FusionInputPluginLoader.h"
#include "FusionResourceManager.h"

namespace FusionEngine
{

	InputManager::InputManager()
		: m_SuspendRequests(0),
		m_CommandBufferLength(128)
	{
		m_PluginLoader = new InputPluginLoader();
	}

	InputManager::InputManager(CL_DisplayWindow *window)
		: m_SuspendRequests(0),
		m_CommandBufferLength(128)
	{
		m_PluginLoader = new InputPluginLoader();
		//! \todo Store the window so input can be grabbed from it
	}

	InputManager::~InputManager()
	{
		delete m_PluginLoader;
	}

	bool InputManager::Test()
	{
		// Keyboard is always required (for global input)
		if (CL_Keyboard::get_device_count() == 0)
			return false;		

		return true;
	}

	void InputManager::Initialise(ResourceManager *resMan, const ClientOptions *cliOpts)
	{
		m_SuspendRequests.clear();

#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
		// Activate Key Down signal handler
		m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionInput::onKeyDown);
		if (CL_Joystick::get_device_count() > 0)
			m_Slots.connect(CL_Joystick::sig_key_down(), this, &FusionInput::onKeyDown);
		if (CL_Mouse::get_device_count() > 0)
			m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionInput::onKeyDown);

		// ... and Key Up
		m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionInput::onKeyUp);
		if (CL_Joystick::get_device_count() > 0)
			m_Slots.connect(CL_Joystick::sig_key_up(), this, &FusionInput::onKeyUp);
		if (CL_Mouse::get_device_count() > 0)
			m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionInput::onKeyUp);

		// Analog inputs
		if (CL_Joystick::get_device_count() > 0)
			m_Slots.connect(CL_Joystick::sig_move(), this, &FusionInput::onKeyDown);
		if (CL_Mouse::get_device_count() > 0)
			m_Slots.connect(CL_Mouse::sig_move(), this, &FusionInput::onKeyDown);

#endif

		m_Slots.connect(CL_Display::sig_resize(), this, &FusionInput::onDisplayResize);

		int numPlayers = 0;
		if (!cliOpts->GetOption("num_local_players", &numPlayers))
			FSN_EXCEPT(ExCode::ResourceNotLoaded, "InputManager::Initialise", "Options file is missing 'num_local_players'");
		

		ResourcePointer<TiXmlDocument> inputDoc = resMan->GetResource<TiXmlDocument>("input/coreinputs.xml");
		m_PluginLoader->LoadInputs(inputDoc.GetDataPtr());

		m_PlayerCommands.resize(g_MaxLocalPlayers);
		const InputPluginLoader::InputTypeList &inputTypes = m_PluginLoader->GetInputs();
		buildCommandBuffers(inputTypes);

		ResourcePointer<TiXmlDocument> keyDoc = resMan->GetResource<TiXmlDocument>("input/keys.xml");
		loadKeyInfo(inputDoc.GetDataPtr());

		SetInputMaps(cliOpts);
	}

	void InputManager::buildCommandBuffers(const InputPluginLoader::InputTypeList &inputTypes)
	{
		Command cmd;
		for(InputPluginLoader::InputTypeList::const_iterator it = inputTypes.begin(), end = inputTypes.end();
			it != end; ++it)
		{
			cmd[it->m_Name] = InputState();			
		}

		std::for_each(m_PlayerCommands.begin(), m_PlayerCommands.end(), resize(m_CommandBufferLength, cmd));
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
		m_PluginLoader->Clear();

		m_KeyInfo.clear();
		m_KeyBindings.clear();

		m_PlayerCommands.clear();
	}

	void InputManager::Update(unsigned int split)
	{
		//m_Impl->Update(split);
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

	void InputManager::SetInputMaps(const FusionEngine::ClientOptions *from)
	{
		// Read the controls from the options object and add them to the input bindings
		for (ClientOptions::ControlsList::const_iterator it = from->m_Controls.begin();
			it != from->m_Controls.end(); ++it)
		{
			unsigned int player = CL_String::to_int(it->m_Player);
			const std::string &input = it->m_Input;
			KeyInfo &key = m_KeyInfo[it->m_Key];
			m_KeyBindings[it->m_Key] = InputBinding(player, input, key);
		}
	}

	void InputManager::MapControl(unsigned int player, const std::string &input, const std::string &shortname)
	{
		std::string playerStr(CL_String::from_int(player));
		KeyInfo &key = m_KeyInfo[shortname];
		//m_InputBindings[playerStr + input] = InputBinding(player, input, key);
		m_KeyBindings[shortname] = InputBinding(player, input, key);
	}

	CL_InputDevice &InputManager::GetDevice(const std::string &name)
	{
		if (name == "keyboard")
			return CL_Keyboard::get_device();
		else if (name == "gamepad")
			return CL_Joystick::get_device();
		else if (name.length() > sizeof("gamepad") && name.substr(0, sizeof("gamepad")) == "gamepad")
		{
			int num = CL_String::to_int(name.substr(8));
			return CL_Joystick::get_device(num);
		}
		else
			return CL_Keyboard::get_device();
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

	//bool InputManager::IsButtonDown(const std::string &name, unsigned int filter) const
	//{
	//	ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
	//	if (it != m_ControlMap.end())
	//		return (*it).second.IsDown();

	//	return false;
	//}

	//float InputManager::GetAnalogValue(const std::string &name, unsigned int filter) const
	//{
	//	ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
	//	if (it != m_ControlMap.end())
	//		return (*it).second.GetPosition();

	//	return 0.0f;
	//}

	void InputManager::CreateCommand(int tick, unsigned int split, unsigned int player)
	{
		if (m_SuspendRequests != 0)
			return;

		//int localPlayers = 0;
		//ClientOptions::getSingleton().GetOption("num_local_players", &localPlayers);

#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
		/////
		// Event Input
		///////

		// Update the requested player's command for this tick to the current command for said player
		Command &currentCommand = m_CurrentCommands[player];
		m_PlayerCommands[player][tick % m_CommandBufferLength] = currentCommand;

#elif FE_INPUT_METHOD == FE_INPUTMETHOD_BUFFERED
		/////
		// Buffered Input
		///////
		FE_EXCEPT(ExCode::Base, "InputManager::CreateCommand", "Buffered input is not implemented");

#else
		/////
		// Unbuffered Input
		///////

		// Grab the command to be updated in the command history
		Command &tickCommand = m_PlayerCommands[player][tick%m_CommandBufferLength];
		// Update all the bindings in the command
		for (Command::iterator it = tickCommand.begin(), end = tickCommand.end(); it != end; ++it)
		{
			const std::string &inputName = it->first;
			// Grab the state for the input to be updated
			InputState &state = it->second;
			// Grab the key binding
			InputBinding &binding = m_KeyBindings[inputName];

			CL_InputDevice &dev = GetDevice(binding.m_Key.m_Device);

			bool nowDown = dev.get_keycode(binding.m_Key.m_Code);
			float nowAxis = dev.get_axis(binding.m_Key.m_Code);
			//! \todo Axis threshold option
			state.m_Changed = state.m_Down != nowDown || abs(state.m_Value - nowAxis) > 0.1f;
			state.m_Down = nowDown;
			state.m_Value = nowAxis;
		}
#endif
	}

	const Command &InputManager::GetCommand(unsigned int player, int tick)
	{
		return m_PlayerCommands[player][tick % m_CommandBufferLength];
	}

	float InputManager::GetMouseSensitivity() const
	{
		return m_MouseSensitivity;
	}

	void InputManager::onKeyDown(const CL_InputEvent &key)
	{	
#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
		if (m_SuspendRequests != 0)
			return;

		// Iterate player input lists to update the current commands
		CommandList::iterator it = m_CurrentCommands.begin(), end = m_CurrentCommands.end();
		for (; it != end; ++it)
		{
			//! \todo Reduce this to one map search. Maybe even use hash maps for speed
			std::string &shortname = m_KeyInfo[key.id ^ (int)key.device.get_type()].m_Name;
			InputBinding &binding = m_KeyBindings[shortname];
			std::string &input = binding.m_Input;
			
			InputState &state = it[input];
			state.m_Changed = !state.m_Down || abs(state.m_Value - key.axis_pos) > 0.1f;
			state.m_Down = true;
			state.m_Value = key.axis_pos;
		}
#endif
	}

	void InputManager::onKeyUp(const CL_InputEvent &key)
	{
#if FE_INPUT_METHOD == FE_INPUTMETHOD_EVENTS
		if (m_SuspendRequests != 0)
			return;

		// Iterate player input lists to update the current commands
		CommandList::iterator it = m_CurrentCommands.begin(), end = m_CurrentCommands.end();
		for (; it != end; ++it)
		{
			//! \todo Reduce this to one map search. Maybe even use hash maps for speed
			std::string &shortname = m_KeyInfo[key.id ^ (int)key.device.get_type()].m_Name;
			InputBinding &binding = m_KeyBindings[shortname];
			std::string &input = binding.m_Input;
			
			InputState &state = it[input];
			state.m_Changed = state.m_Down || abs(state.m_Value - key.axis_pos) > 0.1f;
			state.m_Down = false;
			state.m_Value = key.axis_pos;
		}
#endif
	}

	void InputManager::onDisplayResize(int w, int h)
	{
		m_DisplayCenterX = w >> 1;
		m_DisplayCenterY = h >> 1;
	}

}
