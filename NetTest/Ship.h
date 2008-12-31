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
	float x, y;
	//bool valid;
	Action()
		: x(0.f), y(0.f)/*, valid(false)*/
	{
	}
	Action(float _x, float _y)
		: x(_x), y(_y)/*, valid(true)*/
	{
	}
};
typedef Buffer<Action> ActionHistory;
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
typedef HistorySet<Command> CommandHistory;

class Ship
{
public:
	int sendDelay;
	ObjectID id;
	float x, y;
	Interpolated<float> interp_x;
	Interpolated<float> interp_y;
	bool up, down, left, right;

	bool local;

	// Client uses this to decide what position to interpolate to
	unsigned long mostRecentCommand;
	// Server uses this to keep track of clients with different tick rates
	unsigned long currentTick;
	ActionHistory actionList;
	CommandHistory commandList;
	ActionHistory::iterator currentAction;
	CommandHistory::iterator currentCommand;

	Ship()
		: id(0),
		x(0.f), y(0.f),
		interp_x(0.f), interp_y(0.f),
		sendDelay(0),
		mostRecentCommand(0),
		currentTick(0),
		local(false)
	{
		actionList.set_capacity(1000);
		commandList.set_capacity(500);
		currentAction = actionList.begin();
		currentCommand = commandList.begin();
	}

	Ship(ObjectID _id)
		: id(_id),
		x(0.f), y(0.f),
		interp_x(0.f), interp_y(0.f),
		sendDelay(0),
		mostRecentCommand(0),
		currentTick(0),
		local(false)
	{
		actionList.set_capacity(1000);
		commandList.set_capacity(1000);
		currentAction = actionList.begin();
		currentCommand = commandList.begin();
	}

	Command GetCommand()
	{
		return Command(up, down, left, right);
	}

	void saveCommand(unsigned long tick)
	{
		commandList.add(tick, Command(up, down, left, right));
	}

	void saveCommand(unsigned long tick, const Command &command)
	{
		commandList.add(tick, command);
	}

	//void changeCommand(unsigned long tick, const Command &command)
	//{
	//	CommandHistory::iterator _where = commandList.find_closest(tick);
	//	if (_where == commandList.end())
	//	{
	//		commandList.add(tick, command);
	//		return;
	//	}

	//	if (_where->first == tick)
	//	{
	//		commandList.set(_where, tick, command);
	//		return;
	//	}

	//	if (_where->first > tick && _where != commandList.begin())
	//		--_where;
	//	else if (_where->first < tick)
	//		++_where;
	//		
	//	commandList.insert(_where, tick, command);
	//}

	bool checkCommand(unsigned long tick, const Command &expected)
	{
		CommandHistory::iterator _where = commandList.lower_bound(tick);
		if (_where == commandList.end())
			return true;

		return _where->second != expected;
	}

	void rewindCommand(unsigned long tick)
	{
		if (commandList.empty())
			return;

		currentCommand = commandList.lower_bound(tick);
		if (currentCommand == commandList.end())
			return;

		if (currentCommand->first > tick && currentCommand != commandList.begin())
			--currentCommand;

		//// Confirm the found command (in case the buffer is out of order)
		//if (currentCommand->first > tick)
		//{
		//	currentCommand = commandList.begin();
		//	for (CommandHistory::iterator it = currentCommand, end = commandList.end(); it != end && it->first < tick; ++it)
		//	{
		//		// Find the command closest to the given tick
		//		if (tick - it->first < tick - currentCommand->first)
		//			currentCommand = it;
		//	}
		//	if (currentCommand == commandList.end())
		//		return;

		//	if (currentCommand->first > tick && currentCommand != commandList.begin())
		//		--currentCommand;
		//}

		Command &command = currentCommand->second;
		up = command.up;
		down = command.down;
		left = command.left;
		right = command.right;
	}

	void nextCommand()
	{
		if (commandList.empty() || currentCommand == commandList.end())
			return;

		if (++currentCommand == commandList.end())
			return;

		Command &command = currentCommand->second;
		up = command.up;
		down = command.down;
		left = command.left;
		right = command.right;
	}

	// Finds the command closest to the given time
	void nextCommand(unsigned long tick)
	{
		if (commandList.empty())
			return;

		if (currentCommand == commandList.end())
			return;

		//CommandHistory::iterator end = currentCommand;
		//commandList.previous(end);
		//CommandHistory::iterator nextCommand = currentCommand;
		//commandList.next(nextCommand);
		//while (nextCommand != end && nextCommand->first < tick)
		//{
		//	// Find the command closest to the given tick
		//	if (tick - nextCommand->first < tick - currentCommand->first)
		//		currentCommand = nextCommand;
		//	commandList.next(nextCommand);
		//}

		// Make sure we start the search before the requested tick
		//if (currentCommand->first > tick)
		//	currentCommand = commandList.begin();

		for (CommandHistory::iterator it = currentCommand, end = commandList.end();
			it != end && it->first <= tick; ++it)
		{
			// Find the command closest to the given tick
			//if (tick - it->first < tick - currentCommand->first)
			currentCommand = it;
			//if (it->first > tick)
			//{
			//	break;
			//}
		}

		if (currentCommand == commandList.end())
			return;

		Command &command = currentCommand->second;
		up = command.up;
		down = command.down;
		left = command.left;
		right = command.right;
	}

	void cullCommands(unsigned long tick)
	{
		commandList.remove_before(tick);
	}

	void saveAction(unsigned long tick)
	{
		actionList.push(tick, Action(x, y));
	}

	bool checkAction(unsigned long tick, float x, float y)
	{
		return checkAction(tick, Action(x, y), (float)s_FloatComparisonEpsilon);
	}

	// Returns true if the stored action at the given tick is different to the given x, y values
	bool checkAction(unsigned long tick, const Action& expected, float e)
	{
		if (actionList.empty())
			return false;

		ActionHistory::iterator _where = actionList.find(tick);
		if (_where == actionList.end())
			return false;

		Action &action = _where->second;
		return !fe_fequal(action.x, expected.x, e) || !fe_fequal(action.y, expected.y, e);
	}


	// Returns true if a suitable action was found
	bool checkRewindAction(unsigned long tick, float x, float y)
	{
		if (actionList.empty())
			return false;

		currentAction = actionList.find_closest(tick);
		if (currentAction == actionList.end())
			return false;

		Action &action = currentAction->second;
		if (!fe_fequal(action.x, x, 0.5f) || !fe_fequal(action.y, y, 0.5f))
			return false;

		actionList.erase_after(currentAction);

		x = action.x;
		y = action.y;

		return true;
	}

	// Returns true if an action was found
	bool rewindAction(unsigned long tick)
	{
		if (actionList.empty())
			return true;

		currentAction = actionList.find(tick);
		if (currentAction == actionList.end())
			return false;

		actionList.erase_after(currentAction);
		Action &action = currentAction->second;
		x = action.x;
		y = action.y;

		return true;
	}

	void cullActions()
	{
		actionList.clear();
		//actionList.erase_before(currentAction);
	}

};

#endif