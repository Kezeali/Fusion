
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"

/// Class
#include "FusionClientShip.h"

using namespace FusionEngine;

FusionClientShip::FusionClientShip()
{
	/// Input
	Input.thrust = false;
	Input.reverse = false;
	Input.left = false;
	Input.right = false;
	Input.primary = false;
	Input.secondary = false;
	Input.bomb = false;

	/// State
	InitialState.position = CL_Vector2::ZERO;
	InitialState.velocity = CL_Vector2::ZERO;
	InitialState.facing = 0;
	InitialState.spin = 0;

	InitialState.mass = 0;
	InitialState.current_primary = -1;
	InitialState.current_secondary = -1;
	InitialState.current_bomb = -1;
	InitialState.engines = LEFT | RIGHT;
	InitialState.weapons = PRIMARY | SECONDARY;

	CurrentState = InitialState;
}

FusionClientShip::FusionClientShip(ShipState initState)
{
	/// Input
	Input.thrust = false;
	Input.reverse = false;
	Input.left = false;
	Input.right = false;
	Input.primary = false;
	Input.secondary = false;
	Input.bomb = false;

	/// State
	InitialState = initState;
	CurrentState = initState;
}

FusionClientShip::~FusionClientShip()
{
	// Nothing to do here
}

void FusionClientShip::SetVelocity(const CL_Vector2 &velocity, bool physics)
{
	m_CurrentState.Velocity = velocity;

	if (physics)
		m_PhysicalBody->_setVelocity(velocity);
}

void FusionClientShip::SetPosition(const CL_Vector2 &position, bool physics)
{
	m_CurrentState.Position = position;

	m_Node->SetPosition(position);
	if (physics)
		m_PhysicalBody->_setPosition(position);
}

void FusionClientShip::SetShipState(ShipState state)
{
	m_CurrentState.Velocity = state.Velocity;
	m_CurrentState.Position = state.Position;
	m_CurrentState.Rotation = state.Rotation;
	m_CurrentState.RotationalVelocity = state.RotationalVelocity;

	m_CurrentState.current_primary = state.current_primary;
	m_CurrentState.current_secondary = state.current_secondary;
	m_CurrentState.current_bomb = state.current_bomb;

	m_CurrentState.engines = state.engines;
	m_CurrentState.weapons = state.weapons;
}

void FusionClientShip::SetInputState(ShipInput input)
{
	m_Input.thrust = input.thrust;
	m_Input.left = input.left;
	m_Input.right = input.right;
	m_Input.primary = input.primary;
	m_Input.secondary = input.secondary;
	m_Input.bomb = input.bomb;
}

const ShipState &FusionClientShip::GetShipState() const
{
	return m_CurrentState;
}

const ShipInput &FusionClientShip::GetInputState() const
{
	return m_InitialState;
}

//std::string FusionClientShip::GetShipResource() const
//{
//	return m_ResourceID;
//}

void FusionClientShip::RevertToInitialState()
{
	m_CurrentState = m_InitialState;
}