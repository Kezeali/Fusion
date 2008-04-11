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
	}

	InputManager::FusionInput(CL_DisplayWindow *window, const ClientOptions *from)
		: m_SuspendRequests(0),
		m_CommandBufferLength(128)
	{
		m_PluginLoader = new InputPluginLoader();

		SetInputMaps(from);
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

		// This isn't really needed... If the device isn't there it won't cause a problem
		//PlayerInputMapList::iterator it;
		//for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
		//{
		//switch (it->type)
		//{
		//case Gamepad:
		//// Check if a gamepad exists at the required index.
		//if (CL_Joystick::get_device_count() < it->index)
		//return false;
		//break;

		//case Mouse:
		//if (!CL_Mouse::get_device_count())
		//return false;
		//break;
		//}
		//}
		

		return true;
	}

	void InputManager::Initialise()
	{
		m_SuspendRequests = 0;
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

		m_Slots.connect(CL_Display::sig_resize(), this, &FusionInput::onDisplayResize);

		ResourcePointer<TiXmlDocument> inputDoc = m_ResMan->GetResource<TiXmlDocument>("input/coreinputs.xml");
		m_PluginLoader->LoadInputs(inputDoc.GetDataPtr());
		m_PluginLoader->GetInputs();
	}

	void InputManager::loadControlNames(const ticpp::Document &doc)
	{
		ticpp::Element* elem = doc.FirstChildElement();

		if (elem->Value() != "controlnames")
			throw FileTypeException("InputManager::loadControlNames", "Not a control definition file", __FILE__, __LINE__);

		ticpp::Iterator< ticpp::Element > child( "key" );
		for ( child = child.begin( elem ); child != child.end(); child++ )
		{
			KeyName kname;
			kname.m_Device = child->GetAttributeOrDefault("device", "keyboard");
			child->GetAttributeOrDefault("id", &kname.m_Code, 0);
			kname.m_Name = child->GetAttribute("shortname");
			kname.m_Description = child->GetAttributeOrDefault("name", kname.m_Name);

			if (kname.m_Name.empty())
				throw FileTypeException("InputManager::loadControlNames", "The keyname document contains incomplete tags", __FILE__, __LINE__);
		}
	}

	void InputManager::CleanUp()
	{
		m_PluginLoader->Clear();
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
		// Read the controls from the options object and add them to the input mappings
		for (ClientOptions::ControlsList::const_iterator it = from->m_Controls.begin();
			it != from->m_Controls.end(); ++it)
		{
			MapControl(it->m_Key.c_str(), it->m_Input, it->m_Player);
		}
	}

	void InputManager::MapControl(int keysym, const std::string &name, unsigned int filter)
	{
		std::string buttonName = "Unknown";
		CL_InputDevice dev = CL_Keyboard::get_device();
		buttonName = dev.get_key_name(keysym);

		m_ControlMap[CL_String::from_int(filter) + name] = 
			Control(CL_String::from_int(filter) + name, keysym, buttonName, dev);
	}

	void InputManager::MapControl(int keysym, const std::string &name, CL_InputDevice device, unsigned int filter)
	{
		std::string buttonName = "Unknown";
		buttonName = device.get_key_name(keysym);

		m_ControlMap[CL_String::from_int(filter) + name] = 
			Control(CL_String::from_int(filter) + name, keysym, buttonName, device);
	}

	const Control& InputManager::GetControl(const std::string &name, unsigned int filter) const
	{
		ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
		if (it != m_ControlMap.end())
			return (*it).second;

		throw Exception();
		//return m_ControlMap[CL_String::from_int(filter) + name];
	}

	bool InputManager::IsButtonDown(const std::string &name, unsigned int filter) const
	{
		ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
		if (it != m_ControlMap.end())
			return (*it).second.IsDown();

		return false;
	}

	float InputManager::GetAnalogValue(const std::string &name, unsigned int filter) const
	{
		ControlMap::const_iterator it = m_ControlMap.find(CL_String::from_int(filter) + name);
		if (it != m_ControlMap.end())
			return (*it).second.GetPosition();

		return 0.0f;
	}

	void InputManager::CreateCommand(int tick, unsigned int split, unsigned int player)
	{
		int localPlayers = 0;
		ClientOptions::getSingleton().GetOption("num_local_players", &localPlayers);

		for (ControlMap::iterator it = m_ControlMap.begin(), end = m_ControlMap.end(); it != end; ++it)
		{
			std::string inputName = it->first.substr(1);
			for (int i=0; i<localPlayers; i++)
			{
				std::string strI = CL_String::from_int(i);
				if (it->first.substr(0,strI.length()) == strI)
				{
					Command &cmd = m_PlayerCommands[i][tick%m_CommandBufferLength];
					cmd.SetState(inputName, it->second.IsDown(), it->second.GetValue());
				}
			}
		}
	}

	float InputManager::GetMouseSensitivity() const
	{
		return m_MouseSensitivity;
	}

	//ShipInput FusionInput::GetShipInputs(ObjectID player) const
	//{
	//	return m_ShipInputData[player];
	//}

	//std::vector<ShipInput> FusionInput::GetAllShipInputs() const
	//{
	//	return m_ShipInputData;
	//}

	//GlobalInput FusionInput::GetGlobalInputs() const
	//{
	//	return m_GlobalInputData;
	//}

	void InputManager::onKeyDown(const CL_InputEvent &key)
	{	
		if (m_SuspendRequests != 0)
			return;

		// Control Map system
		ControlMap::iterator it = m_ControlMap.begin();
		for (; it != m_ControlMap.end(); ++it)
		{
			Control* control = &(*it).second;
			if (control->Matches(key))
				control->UpdateState(key);
		}

		
		//// Global inputs (assumes keyboard device is used)
		//if (key.device.get_name() == CL_Keyboard::get_device().get_name())
		//{
		//	if (key.id == m_GlobalInputMap.menu)
		//		m_GlobalInputData.menu = true;
		//	if (key.id == m_GlobalInputMap.console)
		//		m_GlobalInputData.console = true;
		//}


		// Player inputs

		//PlayerInputMapList::iterator it;
		//for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)

		//for (unsigned int i = 0; i < m_PlayerInputMaps.size(); i++)
		//{
		//	if (key.device.get_name() != m_PlayerInputMaps[i].device.get_name())
		//		continue;

		//	if(key.id == m_PlayerInputMaps[i].thrust)
		//	{
		//		m_ShipInputData[i].thrust = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].reverse)
		//	{
		//		m_ShipInputData[i].reverse = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].left)
		//	{
		//		m_ShipInputData[i].left = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].right)
		//	{
		//		m_ShipInputData[i].right = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].primary)
		//	{
		//		m_ShipInputData[i].primary = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].secondary)
		//	{
		//		m_ShipInputData[i].secondary = true;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].bomb)
		//	{
		//		m_ShipInputData[i].bomb = true;
		//	}
		//}
	}

	void InputManager::onKeyUp(const CL_InputEvent &key)
	{
		if (m_SuspendRequests != 0)
			return;

		// Control Map system
		ControlMap::iterator it = m_ControlMap.begin();
		for (; it != m_ControlMap.end(); ++it)
		{
			Control *control = &(*it).second;
			if (control->Matches(key))
				control->UpdateState(key);
		}


		//// Global inputs (assumes keyboard device is used)
		//if (key.device.get_name() == CL_Keyboard::get_device().get_name())
		//{
		//	if (key.id == m_GlobalInputMap.menu)
		//		m_GlobalInputData.menu = false;
		//	if (key.id == m_GlobalInputMap.console)
		//		m_GlobalInputData.console = false;
		//}


		// Player inputs

		//PlayerInputMapList::iterator it;
		//for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
		//for (unsigned int i = 0; i < m_PlayerInputMaps.size(); i++)
		//{
		//	if (key.device.get_name() != m_PlayerInputMaps[i].device.get_name())
		//		continue;

		//	if(key.id == m_PlayerInputMaps[i].thrust)
		//	{
		//		m_ShipInputData[i].thrust = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].reverse)
		//	{
		//		m_ShipInputData[i].reverse = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].left)
		//	{
		//		m_ShipInputData[i].left = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].right)
		//	{
		//		m_ShipInputData[i].right = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].primary)
		//	{
		//		m_ShipInputData[i].primary = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].secondary)
		//	{
		//		m_ShipInputData[i].secondary = false;
		//	}
		//	if(key.id == m_PlayerInputMaps[i].bomb)
		//	{
		//		m_ShipInputData[i].bomb = false;
		//	}
		//}
	}

	void InputManager::onDisplayResize(int w, int h)
	{
		m_DisplayCenterX = w >> 1;
		m_DisplayCenterY = h >> 1;
	}

}
