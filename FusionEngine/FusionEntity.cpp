
#include "FusionCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"
#include "FusionPhysicsBody.h"

/// Class
#include "FusionEntity.h"

namespace FusionEngine
{

	Entity::Entity()
		: m_Name("default")
	{
	}

	Entity::Entity(const std::string &name)
		: m_Name(name)
	{
	}


	Entity::~Entity()
	{
		// Nothing to do here
	}

	void Entity::SetVelocity(const Vector2 &velocity)
	{
		m_StateChanged = true;

		if (m_PhysicalBody != 0)
			m_PhysicalBody->_setVelocity(velocity);
	}

	void Entity::SetPosition(const Vector2 &position)
	{
		m_CurrentState.position = position;

		m_StateChanged = true;

		m_Node->SetPosition(position);
		if (physics)
			m_PhysicalBody->_setPosition(position);
	}

	void Entity::SetRotationalVelocity(float velocity, bool physics)
	{
		m_CurrentState.rotationalVelocity = velocity;

		m_StateChanged = true;

		if (physics)
			m_PhysicalBody->SetRotationalVelocity(velocity);
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

}
