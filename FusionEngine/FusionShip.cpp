
#include "FusionCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"
#include "FusionPhysicsBody.h"

/// Class
#include "FusionShip.h"

using namespace FusionEngine;

FusionShip::FusionShip()
: m_StateChanged(true),
m_InputChanged(true)
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

FusionShip::FusionShip(ShipState initState, FusionPhysicsBody *body, FusionNode *node)
: m_StateChanged(true),
m_InputChanged(true)
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

	/// Body
	m_PhysicalBody = body;
	// Pass the physical body the collision callback
	body->SetCollisionCallback(boost::bind(&FusionShip::CollisionResponse, this, _1, _2));
	/*CreateCCB<FusionShip>(ship, &FusionShip::CollisionResponse);*/

	/// Node
	m_Node = node;
}

FusionShip::FusionShip(ShipState initState, ShipInput initInput, FusionPhysicsBody *body, FusionNode *node)
: m_StateChanged(true),
m_InputChanged(true)
{
	/// Input
	m_Input = initInput;

	/// State
	m_InitialState = initState;
	m_CurrentState = initState;

	/// Body
	m_PhysicalBody = body;

	/// Node
	m_Node = node;
}

FusionShip::~FusionShip()
{
	// Nothing to do here
}

void FusionShip::SetVelocity(const CL_Vector2 &velocity, bool physics)
{
	m_CurrentState.Velocity = velocity;

	m_StateChanged = true;

	if (physics)
		m_PhysicalBody->_setVelocity(velocity);
}

void FusionShip::SetPosition(const CL_Vector2 &position, bool physics)
{
	m_CurrentState.Position = position;

	m_StateChanged = true;

	m_Node->SetPosition(position);
	if (physics)
		m_PhysicalBody->_setPosition(position);
}

void FusionShip::SetRotationalVelocity(float velocity, bool physics)
{
	m_CurrentState.RotationalVelocity = velocity;

	m_StateChanged = true;

	if (physics)
		m_PhysicalBody->SetRotationalVelocity(velocity);
}

void FusionShip::SetShipState(ShipState state)
{
	m_CurrentState = state;

	m_StateChanged = true;
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

void FusionShip::SetInputState(ShipInput input)
{
	m_Input = input;

	m_InputChanged = true;
	/*
	m_Input.thrust = input.thrust;
	m_Input.left = input.left;
	m_Input.right = input.right;
	m_Input.primary = input.primary;
	m_Input.secondary = input.secondary;
	m_Input.bomb = input.bomb;
	*/
}

const ShipState &FusionShip::GetShipState() const
{
	return m_CurrentState;
}

const ShipInput &FusionShip::GetInputState() const
{
	return m_Input;
}

void FusionShip::SetSceneNode(FusionNode *node)
{
	m_Node = node;
}

const FusionNode *FusionShip::GetSceneNode() const
{
	return m_Node;
}

void FusionShip::SetPhysicalBody(FusionPhysicsBody *body)
{
	m_PhysicalBody = body;
}

const FusionPhysicsBody *FusionShip::GetPhysicalBody() const
{
	return m_PhysicalBody;
}

//std::string FusionShip::GetShipResource() const
//{
//	return m_ResourceID;
//}

void FusionShip::RevertToInitialState()
{
	m_CurrentState = m_InitialState;
}

void FusionShip::CollisionResponse(const FusionEngine::FusionPhysicsBody *other, const CL_Vector2 &collision_point)
{
	//m_PhysicalBody->_setForce(CL_Vector2::ZERO);
	m_PhysicalBody->ApplyForce(-m_PhysicalBody->GetVelocity());
}
