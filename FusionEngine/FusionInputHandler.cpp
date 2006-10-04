
/// Class
#include "FusionInputHandler.h"

using namespace FusionEngine;

FusionInput::FusionInput()
: m_Suspend(false)
{
}

FusionInput::FusionInput(const ClientOptions &from)
: m_Suspend(false)
{
	m_GlobalInputMap = from.GlobalInputs;
	m_PlayerInputMaps = from.PlayerInputs;
}

bool FusionInput::Test()
{
	// Keyboard is always required (for global input)
	if (CL_Keyboard::get_device_count() == 0)
		return false;

	PlayerInputMapList::iterator it;
	for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
	{
		switch (it->type)
		{
		case InputDevType::Gamepad:
			// Check if a gamepad exists at the required index.
			if (CL_Joystick::get_device_count() < it->index)
				return false;
			break;

		case InputDevType::Mouse:
			if (!CL_Mouse::get_device_count())
				return false;
			break;
		}
	}

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
	m_Suspend = false;
}

void FusionInput::Suspend()
{
	m_Suspend = true;
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
	PlayerInputMapList::iterator it;
	for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
	{
		if (key.device.get_name() != it->device->get_name())
			continue;

		if(key.id == it->thrust)
		{
		    m_ShipInputData[i].thrust = true;
		}
		if(key.id == it->reverse)
		{
		    m_ShipInputData[i].reverse = true;
		}
		if(key.id == it->left)
		{
		    m_ShipInputData[i].left = true;
		}
		if(key.id == it->right)
		{
		    m_ShipInputData[i].right = true;
		}
		if(key.id == it->primary)
		{
		    m_ShipInputData[i].primary = true;
		}
		if(key.id == it->secondary)
		{
		    m_ShipInputData[i].secondary = true;
		}
		if(key.id == it->bomb)
		{
			m_ShipInputData[i].bomb = true;
		}
	}
}

void FusionInput::onKeyUp(const CL_InputEvent &key)
{
	PlayerInputMapList::iterator it;
	for (it = m_PlayerInputMaps.begin(); it != m_PlayerInputMaps.end(); ++it)
	{
		if (key.device.get_name() != it->device.get_name())
			continue;

		if(key.id == it->thrust)
		{
		    m_ShipInputData[i].thrust = false;
		}
		if(key.id == it->reverse)
		{
		    m_ShipInputData[i].reverse = false;
		}
		if(key.id == it->left)
		{
		    m_ShipInputData[i].left = false;
		}
		if(key.id == it->right)
		{
		    m_ShipInputData[i].right = false;
		}
		if(key.id == it->primary)
		{
		    m_ShipInputData[i].primary = false;
		}
		if(key.id == it->secondary)
		{
		    m_ShipInputData[i].secondary = false;
		}
		if(key.id == it->bomb)
		{
			m_ShipInputData[i].bomb = false;
		}
	}
}