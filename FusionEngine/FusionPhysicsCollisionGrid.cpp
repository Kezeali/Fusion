#include "FusionPhysicsCollisionGrid.h"

using namespace FusionEngine;

FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid()
{
}

FusionPhysicsCollisionGrid::~FusionPhysicsCollisionGrid()
{
}

void FusionPhysicsCollisionGrid::AddBody(FusionPhysicsBody *body)
{
	m_Bodies.push_back(body);

	int pos = _getGridPosition(body);

	body->_setCGIndex(pos);
	m_Grid[pos].push_back(body);
}

void FusionPhysicsCollisionGrid::RemoveBody(FusionPhysicsBody *body)
{
}

void FusionPhysicsCollisionGrid::ResortAll()
{
}

void FusionPhysicsCollisionGrid::Clear()
{
}

void FusionPhysicsCollisionGrid::Resort()
{
}

void FusionPhysicsCollisionGrid::_updateThis(FusionEngine::FusionPhysicsBody *body)
{
	_updateThis(body->_getCGIndex());
}

void FusionPhysicsCollisionGrid::_updateThis(int cgind)
{
}

int FusionPhysicsCollisionGrid::_getGridPosition(FusionEngine::FusionPhysicsBody *body)
{
	int pos, x, y;
	x = body->GetPosition().x * m_GridScale;
	y = body->GetPosition().y * m_GridScale;

	pos = y * m_GridWidth + x;

	return pos;
}