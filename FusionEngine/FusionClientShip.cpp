
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionStateData.h"
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
}

void FusionClientShip::SetPosition(const CL_Vector2 &position)
{
	m_CurrentState.Position = position;

	m_Node->SetPosition(position);
	m_PhysicalBody->SetPosition(position);
}

void FusionClientShip::RevertToInitialState()
{
	CurrentState = InitialState;
}