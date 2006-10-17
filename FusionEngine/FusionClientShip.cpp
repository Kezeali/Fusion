
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"
#include "FusionPhysicsBody.h"

/// Class
#include "FusionClientShip.h"

using namespace FusionEngine;

FusionClientShip::FusionClientShip()
{
	/// Input
	m_Input.pid = 0;
	m_Input.thrust = false;
	m_Input.reverse = false;
	m_Input.left = false;
	m_Input.right = false;
	m_Input.primary = false;
	m_Input.secondary = false;
	m_Input.bomb = false;

	/// State
	m_InitialState.PID = 0;
	m_InitialState.Position = CL_Vector2::ZERO;
	m_InitialState.Velocity = CL_Vector2::ZERO;
	m_InitialState.Rotation = 0;
	m_InitialState.RotationalVelocity = 0;

	m_InitialState.health = 0;
	m_InitialState.current_primary = -1;
	m_InitialState.current_secondary = -1;
	m_InitialState.current_bomb = -1;
	m_InitialState.engines = LEFT | RIGHT;
	m_InitialState.weapons = PRIMARY | SECONDARY;

	m_CurrentState = m_InitialState;
}

FusionClientShip::FusionClientShip(ShipState initState)
{
	/// Input
	m_Input.pid = initState.PID;
	m_Input.thrust = false;
	m_Input.reverse = false;
	m_Input.left = false;
	m_Input.right = false;
	m_Input.primary = false;
	m_Input.secondary = false;
	m_Input.bomb = false;

	/// State
	m_InitialState = initState;
	m_CurrentState = initState;
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
	m_CurrentState = state;
	/*
	m_CurrentState.Velocity = state.Velocity;
	m_CurrentState.Position = state.Position;
	m_CurrentState.Rotation = state.Rotation;
	m_CurrentState.RotationalVelocity = state.RotationalVelocity;

	m_CurrentState.health = state.health;

	m_CurrentState.current_primary = state.current_primary;
	m_CurrentState.current_secondary = state.current_secondary;
	m_CurrentState.current_bomb = state.current_bomb;

	m_CurrentState.engines = state.engines;
	m_CurrentState.weapons = state.weapons;
	*/
}

void FusionClientShip::SetInputState(ShipInput input)
{
	m_Input = input;
	/*
	m_Input.thrust = input.thrust;
	m_Input.left = input.left;
	m_Input.right = input.right;
	m_Input.primary = input.primary;
	m_Input.secondary = input.secondary;
	m_Input.bomb = input.bomb;
	*/
}

const ShipState &FusionClientShip::GetShipState() const
{
	return m_CurrentState;
}

const ShipInput &FusionClientShip::GetInputState() const
{
	return m_Input;
}

//std::string FusionClientShip::GetShipResource() const
//{
//	return m_ResourceID;
//}

void FusionClientShip::RevertToInitialState()
{
	m_CurrentState = m_InitialState;
}