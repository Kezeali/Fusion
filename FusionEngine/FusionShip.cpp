
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
	m_InitialState.position = Vector2::zero();
	m_InitialState.velocity = Vector2::zero();
	m_InitialState.rotation = 0;
	m_InitialState.rotationalVelocity = 0;

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
m_InputChanged(true),
m_InitialState(initState),
m_CurrentState(initState),
m_PhysicalBody(body),
m_Node(node)
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

	// Ships are their own collision handlers
	body->SetCollisionHandler(this);

	/*body->SetCollisionCallback(boost::bind(&FusionShip::CollisionResponse, this, _1, _2));*/
	/*CreateCCB<FusionShip>(ship, &FusionShip::CollisionResponse);*/
}

FusionShip::FusionShip(ShipState initState, ShipInput initInput, FusionPhysicsBody *body, FusionNode *node)
: 
m_Input(initInput)
{
	FusionShip(initState, body, node);
	/// Input
	m_Input = initInput;

	/// State
	m_InitialState = initState;
	m_CurrentState = initState;

	/// Body
	m_PhysicalBody = body;
	body->SetCollisionHandler(this);

	/// Node
	m_Node = node;
}

FusionShip::~FusionShip()
{
	// Nothing to do here
}

void FusionShip::SetVelocity(const Vector2 &velocity, bool physics)
{
	m_CurrentState.velocity = velocity;

	m_StateChanged = true;

	if (physics)
		m_PhysicalBody->_setVelocity(velocity);
}

void FusionShip::SetPosition(const Vector2 &position, bool physics)
{
	m_CurrentState.position = position;

	m_StateChanged = true;

	m_Node->SetPosition(position);
	if (physics)
		m_PhysicalBody->_setPosition(position);
}

void FusionShip::SetRotationalVelocity(float velocity, bool physics)
{
	m_CurrentState.rotationalVelocity = velocity;

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

bool FusionShip::CanCollideWith(const FusionEngine::FusionPhysicsBody *other)
{
	return true;
}

void FusionShip::CollisionWith(const FusionEngine::FusionPhysicsBody *other, const Vector2 &collision_point)
{
	
}
