#ifndef SHIP_H
#define SHIP_H
#if _MSC_VER > 1000
#pragma once
#endif

#include "../FusionEngine/FusionCommon.h"

using namespace FusionEngine;

static const float s_DefaultTightness = 0.25f;
static const float s_SmoothTightness = 0.1f;

class InterpolatedBase
{
public:
	void activateSmoothing()
	{
		m_Tightness = m_SmoothTightness;
	}

protected:
	InterpolatedBase()
		: m_DefaultTightness(s_DefaultTightness),
		m_SmoothTightness(s_SmoothTightness),
		m_Tightness(s_DefaultTightness)
	{
	}

	InterpolatedBase(float defaultTightness, float smoothTightness)
		: m_DefaultTightness(defaultTightness),
		m_SmoothTightness(smoothTightness),
		m_Tightness(defaultTightness)
	{
	}

	float m_DefaultTightness, m_SmoothTightness;
	float m_Tightness;

	void updateSmoothing()
	{
		m_Tightness += (m_DefaultTightness - m_Tightness) * 0.01f;
	}
};

template <class T>
class Interpolated : public InterpolatedBase
{
public:
	T m_InterpedValue;
	T m_Value;

	Interpolated()
		: InterpolatedBase()
	{
	}

	Interpolated(T value)
		: InterpolatedBase(),
		m_InterpedValue(value),
		m_Value(value)
	{
	}

	Interpolated(T value, float defaultTightness, float smoothTightness)
		: InterpolatedBase(defaultTightness, smoothTightness),
		m_InterpedValue(value),
		m_Value(value)
	{
	}

	void update()
	{
		update(m_Value);
	}

	void update(T value)
	{
		m_Value = value;

		if (fe_fequal(m_Value, value))
			InterpolatedBase::activateSmoothing();

		m_InterpedValue = m_InterpedValue + (m_Value - m_InterpedValue) * m_Tightness;

		InterpolatedBase::updateSmoothing();
	}
};

template <>
class Interpolated<unsigned long> : public InterpolatedBase
{
public:
	unsigned long m_InterpedValue;
	unsigned long m_Value;

	Interpolated()
		: InterpolatedBase(),
		m_InterpedValue(0),
		m_Value(0)
	{
	}

	Interpolated(unsigned long value)
		: InterpolatedBase(),
		m_InterpedValue(value),
		m_Value(value)
	{
	}

	Interpolated(unsigned long value, float defaultTightness, float smoothTightness)
		: InterpolatedBase(defaultTightness, smoothTightness),
		m_InterpedValue(value),
		m_Value(value)
	{
	}

	void update()
	{
		update(m_Value);
	}

	void update(unsigned long value)
	{
		m_Value = value;

		if (m_Value != value)
			InterpolatedBase::activateSmoothing();

		double temp = m_Value >= m_InterpedValue ?
			(m_InterpedValue + (m_Value - m_InterpedValue) * m_Tightness)
			: (m_InterpedValue - (m_InterpedValue - m_Value) * m_Tightness);
		m_InterpedValue = (unsigned)fe_round(temp);

		InterpolatedBase::updateSmoothing();
	}
};

struct Action
{
	Vector2 position;
	Vector2 velocity;
	float angle, angularVelocity;
	//bool valid;
	Action()
		: angle(0.f), angularVelocity(0.f)
	{
	}
	Action(const Vector2& _velocity, float _angularVelocity, const Vector2& _position, float _angle)
		: velocity(_velocity), angularVelocity(_angularVelocity), position(_position), angle(_angle)
	{
	}
};

struct AuthoritativeAction : Action
{
	//Action action;
	ObjectID authId;

	//bool valid;
	AuthoritativeAction()
		: Action(),
		authId(0)
	{
	}
	AuthoritativeAction(ObjectID _authId, const Action& _action)
		: Action(_action.velocity, _action.angularVelocity, _action.position, _action.angle),
		authId(_authId)
	{
	}
};

typedef Buffer<AuthoritativeAction> ActionHistory;


struct Command
{
	Command()
		: up(false), down(false), left(false), right(false)
	{
	}

	Command(bool _up, bool _down, bool _left, bool _right)
		: up(_up), down(_down), left(_left), right(_right)
	{
	}

	bool operator== (const Command& other)
	{
		return up == other.up && down == other.down && left == other.left && right == other.right;
	}

	bool operator!= (const Command& other)
	{
		return !(*this == other);
	}

	bool up, down, left, right;
};
typedef Buffer<Command> CommandHistory;

class Ship
{
public:
	int sendDelay;
	ObjectID id;
	Vector2 velocity;
	Vector2 position;
	float angularVelocity;
	float angle;
	Interpolated<float> interp_x;
	Interpolated<float> interp_y;
	bool up, down, left, right;

	bool local;

	// Client uses this to decide what position to interpolate to
	//unsigned long mostRecentCommand;
	// Server uses this to keep track of clients with different tick rates
	//unsigned long currentTick;
	unsigned long lastSentTick;
	ActionHistory actionList;
	CommandHistory commandList;
	ActionHistory::iterator currentAction;
	CommandHistory::iterator currentCommand;

	Ship()
		: id(0),
		velocity(0.f, 0.f),
		position(0.f, 0.f),
		interp_x(0.f), interp_y(0.f),
		angularVelocity(0.f),
		angle(0.f),
		sendDelay(0),
		//mostRecentCommand(0),
		lastSentTick(0),
		local(false)
	{
		actionList.set_capacity(64);
		commandList.set_capacity(64);
		currentAction = actionList.begin();
		currentCommand = commandList.begin();
	}

	Ship(ObjectID _id)
		: id(_id),
		velocity(0.f, 0.f),
		position(0.f, 0.f),
		interp_x(0.f), interp_y(0.f),
		angularVelocity(0.f),
		angle(0.f),
		sendDelay(0),
		//mostRecentCommand(0),
		lastSentTick(0),
		local(false)
	{
		actionList.set_capacity(64);
		commandList.set_capacity(64);
		currentAction = actionList.begin();
		currentCommand = commandList.begin();
	}

	Command GetCommand()
	{
		return Command(up, down, left, right);
	}

	Action GetAction()
	{
		return Action(velocity, angularVelocity, position, angle);
	}

	void SetCommand(const Command& command)
	{
		up = command.up;
		down = command.down;
		left = command.left;
		right = command.right;
	}

	void SetAction(const Action& state)
	{
		velocity = state.velocity;
		position = state.position;
		angularVelocity = state.angularVelocity;
		angle = state.angle;
	}

};

#endif