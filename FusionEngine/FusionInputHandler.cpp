
/// Class
#include "FusionInputHandler.h"

using namespace FusionEngine;

FusionInput::FusionInput()
: m_Active(true)
{
}

FusionInput::FusionInput(const ClientOptions &from)
: m_Active(true)
{
	m_GlobalInputMap = from.GlobalInputs;
	m_PlayerInputMaps = from.PlayerInputs;
}

bool FusionInput::Test()
{
	// Keyboard is always required (for global input)
	if (CL_Keyboard::get_device_count() == 0)
		return false;

	/* This isn't really needed... If the device isn't there it won't cause a problem
	PlayerInputMapList::iterator it;
	for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
	{
		switch (it->type)
		{
		case Gamepad:
			// Check if a gamepad exists at the required index.
			if (CL_Joystick::get_device_count() < it->index)
				return false;
			break;

		case Mouse:
			if (!CL_Mouse::get_device_count())
				return false;
			break;
		}
	}
	*/

	return true;
}

void FusionInput::Initialise()
{
	// Activate Key Down signal handler
	m_Slots.connect(CL_Keyboard::sig_key_down(), this, &FusionInput::onKeyDown);
	m_Slots.connect(CL_Joystick::sig_key_down(), this, &FusionInput::onKeyDown);
	m_Slots.connect(CL_Mouse::sig_key_down(), this, &FusionInput::onKeyDown);

	// ... and Key Up
	m_Slots.connect(CL_Keyboard::sig_key_up(), this, &FusionInput::onKeyUp);
	m_Slots.connect(CL_Joystick::sig_key_up(), this, &FusionInput::onKeyUp);
	m_Slots.connect(CL_Mouse::sig_key_up(), this, &FusionInput::onKeyUp);
}

void FusionInput::Activate()
{
	m_Active = false;
}

void FusionInput::Suspend()
{
	m_Active = true;
}

void FusionInput::SetInputMaps(const FusionEngine::ClientOptions &from)
{
	m_GlobalInputMap = from.GlobalInputs;
	m_PlayerInputMaps = from.PlayerInputs;
}

ShipInput FusionInput::GetShipInputs(PlayerInd player) const
{
	return m_ShipInputData[player];
}

std::vector<ShipInput> FusionInput::GetAllShipInputs() const
{
	return m_ShipInputData;
}

GlobalInput FusionInput::GetGlobalInputs() const
{
	return m_GlobalInputData;
}

void FusionInput::onKeyDown(const CL_InputEvent &key)
{
	// Global inputs (assumes keyboard device is used)
	if (key.device.get_name() == CL_Keyboard::get_device().get_name())
	{
		if (key.id == m_GlobalInputMap.menu)
			m_GlobalInputData.menu = true;
		if (key.id == m_GlobalInputMap.console)
			m_GlobalInputData.console = true;
	}
	// Player inputs
	if (m_Active)
	{
		//PlayerInputMapList::iterator it;
		//for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)

		for (unsigned int i = 0; i < m_PlayerInputMaps.size(); i++)
		{
			if (key.device.get_name() != m_PlayerInputMaps[i].device.get_name())
				continue;

			if(key.id == m_PlayerInputMaps[i].thrust)
			{
				m_ShipInputData[i].thrust = true;
			}
			if(key.id == m_PlayerInputMaps[i].reverse)
			{
				m_ShipInputData[i].reverse = true;
			}
			if(key.id == m_PlayerInputMaps[i].left)
			{
				m_ShipInputData[i].left = true;
			}
			if(key.id == m_PlayerInputMaps[i].right)
			{
				m_ShipInputData[i].right = true;
			}
			if(key.id == m_PlayerInputMaps[i].primary)
			{
				m_ShipInputData[i].primary = true;
			}
			if(key.id == m_PlayerInputMaps[i].secondary)
			{
				m_ShipInputData[i].secondary = true;
			}
			if(key.id == m_PlayerInputMaps[i].bomb)
			{
				m_ShipInputData[i].bomb = true;
			}
		}
	}
}

void FusionInput::onKeyUp(const CL_InputEvent &key)
{
	// Global inputs (assumes keyboard device is used)
	if (key.device.get_name() == CL_Keyboard::get_device().get_name())
	{
		if (key.id == m_GlobalInputMap.menu)
			m_GlobalInputData.menu = false;
		if (key.id == m_GlobalInputMap.console)
			m_GlobalInputData.console = false;
	}

	// Player inputs
	if (m_Active)
	{
		//PlayerInputMapList::iterator it;
		//for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
		for (unsigned int i = 0; i < m_PlayerInputMaps.size(); i++)
		{
			if (key.device.get_name() != m_PlayerInputMaps[i].device.get_name())
				continue;

			if(key.id == m_PlayerInputMaps[i].thrust)
			{
				m_ShipInputData[i].thrust = false;
			}
			if(key.id == m_PlayerInputMaps[i].reverse)
			{
				m_ShipInputData[i].reverse = false;
			}
			if(key.id == m_PlayerInputMaps[i].left)
			{
				m_ShipInputData[i].left = false;
			}
			if(key.id == m_PlayerInputMaps[i].right)
			{
				m_ShipInputData[i].right = false;
			}
			if(key.id == m_PlayerInputMaps[i].primary)
			{
				m_ShipInputData[i].primary = false;
			}
			if(key.id == m_PlayerInputMaps[i].secondary)
			{
				m_ShipInputData[i].secondary = false;
			}
			if(key.id == m_PlayerInputMaps[i].bomb)
			{
				m_ShipInputData[i].bomb = false;
			}
		}
	}
}