#include "FusionPhysicsCollisionGrid.h"

using namespace FusionEngine;

FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid()
: m_GridWidth(1),
m_GridHeight(1),
m_GridScale(1.f)
{
}

FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid(float scale, int level_x, int level_y)
: m_GridWidth(1),
m_GridHeight(1),
m_GridScale(scale)
{
	SetScale(scale, level_x, level_y);
}

FusionPhysicsCollisionGrid::~FusionPhysicsCollisionGrid()
{
	Clear();
}

void FusionPhysicsCollisionGrid::AddBody(FusionPhysicsBody *body)
{
	int pos = _getGridPosition(body);

	body->_setCGPos(pos);
	body->_setCCIndex(m_Grid[pos].size());
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
	BodyList::iterator body;

	for (body = m_BodiesToUpdate.begin(); body != m_BodiesToUpdate.end(); ++body)
	{
		int pos = _getGridPosition((*body));
		// Check if the object actually needs to be updated :P
		if (pos == (*body)->_getCGPos())
		{
			// Remove the body form its current position in the grid
			m_Grid[(*body)->_getCGPos()][(*body)->_getCCIndex()] = NULL;

			(*body)->_setCGPos(pos);
			(*body)->_setCCIndex(m_Grid[pos].size());

			m_Grid[pos].push_back((*body));
		}
	}
}

void FusionPhysicsCollisionGrid::SetScale(float scale, int level_x, int level_y)
{
	m_GridScale = scale;
	m_GridWidth = int(ceil(level_x * scale));
	m_GridHeight = int(ceil(level_y * scale));

	// Speed up later insertions by preallocating the grid.
	m_Grid.resize(m_GridWidth * m_GridHeight);
}

FusionPhysicsCollisionGrid::BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(FusionEngine::FusionPhysicsBody *body)
{
	FusionPhysicsCollisionGrid::BodyList bodies;
	FusionPhysicsCollisionGrid::BodyList *cell;

	int grid_pos = body->_getCGPos();

	// Find adjacent cells and copy their data
	// Local cell:
	cell = &m_Grid[grid_pos];

	bodies.reserve(cell->size());
	std::copy(cell->begin(), cell->end(), bodies.begin());

	// Left Hand cell (only if this isn't the left edge):
	if (grid_pos % m_GridWidth > 0)
	{
		cell = &m_Grid[grid_pos - 1];

		bodies.reserve(bodies.size() + cell->size());
		std::copy(cell->begin(), cell->end(), bodies.end());
	}

	// Right Hand cell:
	if ((grid_pos + 1) % m_GridWidth > 0)
	{
		cell = &m_Grid[grid_pos + 1];

		bodies.reserve(bodies.size() + cell->size());
		std::copy(cell->begin(), cell->end(), bodies.end());
	}

	// Cell above:
	if (grid_pos >= m_GridWidth)
	{
		cell = &m_Grid[grid_pos - m_GridWidth];
		
		bodies.reserve(bodies.size() + cell->size());
		std::copy(cell->begin(), cell->end(), bodies.end());
	}

	// Cell below:
	if (grid_pos < m_GridWidth * (m_GridHeight -1))
	{
		cell = &m_Grid[grid_pos + m_GridWidth];
		
		bodies.reserve(bodies.size() + cell->size());
		std::copy(cell->begin(), cell->end(), bodies.end());
	}

	return bodies;
}

void FusionPhysicsCollisionGrid::_updateThis(FusionEngine::FusionPhysicsBody *body)
{
	m_BodiesToUpdate.push_back(body);
}

/* [depreciated]
void FusionPhysicsCollisionGrid::_updateThis(int cgind)
{
	m_BodiesToUpdate.push_back(cgind);
}
*/

int FusionPhysicsCollisionGrid::_getGridPosition(FusionEngine::FusionPhysicsBody *body)
{
	int pos, x, y;
	x = int(body->GetPosition().x * m_GridScale);
	y = int(body->GetPosition().y * m_GridScale);

	pos = y * m_GridWidth + x;

	return pos;
}