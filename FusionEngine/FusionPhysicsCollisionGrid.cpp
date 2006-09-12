#include "FusionPhysicsCollisionGrid.h"

using namespace FusionEngine;

FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid()
: m_GridHeight(1),
m_GridWidth(1),
m_GridScale(1.f)
{
}

FusionPhysicsCollisionGrid::~FusionPhysicsCollisionGrid()
{
	Clear();
}

void FusionPhysicsCollisionGrid::AddBody(FusionPhysicsBody *body)
{
	int pos = _getGridPosition(body);

	body->_setCGIndex(pos);
	m_Grid[pos].push_back(body);
}

void FusionPhysicsCollisionGrid::RemoveBody(FusionPhysicsBody *body)
{
	m_Grid[body->_getCGPos()][body->_getCCIndex()] = NULL;
}

void FusionPhysicsCollisionGrid::ResortAll()
{
	// This just adds all bodies in the grid to the update list...
	BodyListCollection::iterator cell;
	BodyList::iterator body;

	for (cell = m_Grid.begin(); cell != m_Grid.end(); ++cell)
	{
		for (body = (*cell).begin(); body != (*cell).end(); ++body)
		{
			m_BodiesToUpdate.push_back((*body));
		}
	}
}

void FusionPhysicsCollisionGrid::ForceResortAll()
{
	ResortAll();
	Resort();
}

void FusionPhysicsCollisionGrid::Clear()
{
	// Clean the grid, but don't destroy the grid itself.
	BodyListCollection::iterator it;

	for (it = m_Grid.begin(); it != m_Grid.end(); ++it)
	{
		(*it).clear();
	}

	m_BodiesToUpdate.clear();
}

void FusionPhysicsCollisionGrid::Resort()
{
	BodyListCollection::iterator cell;
	BodyList::iterator body;

	for (cell = m_Grid.begin(); cell != m_Grid.end(); ++cell)
	{
		for (body = (*cell).begin(); body != (*cell).end(); ++body)
		{
			m_BodiesToUpdate.push_back((*body));
		}
	}
}

void FusionPhysicsCollisionGrid::SetScale(float scale, int level_x, int level_y)
{
	s

void FusionPhysicsCollisionGrid::_updateThis(FusionEngine::FusionPhysicsBody *body)
{
	_updateThis(body->_getCGIndex());
}

void FusionPhysicsCollisionGrid::_updateThis(int cgind)
{
	m_BodiesToUpdate.push_back(cgind);
}

int FusionPhysicsCollisionGrid::_getGridPosition(FusionEngine::FusionPhysicsBody *body)
{
	int pos, x, y;
	x = body->GetPosition().x * m_GridScale;
	y = body->GetPosition().y * m_GridScale;

	pos = y * m_GridWidth + x;

	return pos;
}