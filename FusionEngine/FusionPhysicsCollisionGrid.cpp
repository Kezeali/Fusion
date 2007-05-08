
#include "Common.h"

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
	// Each body stores its node index
	BodyList node = m_Grid[body->_getCGPos()];

	// Find the body in its node
	{
		BodyList::iterator node_it = node.begin();
		for (; node_it != node.end(); ++node_it)
		{
			if ((*node_it) == body)
			{
				node.erase(node_it);
				break;
			}
		}
	}

	// Check for the body in the update list
	{
		BodyList::iterator upd_it = m_BodiesToUpdate.begin();
		for (; upd_it != m_BodiesToUpdate.end(); ++upd_it)
		{
			if ((*upd_it) == body)
			{
				node.erase(upd_it);
				break;
			}
		}
	}
}

void FusionPhysicsCollisionGrid::ResortAll()
{
	// This fn. just adds all bodies in the grid to the update list...

	m_BodiesToUpdate.clear();

	BodyListCollection::iterator node;
	BodyList::iterator body;

	for (node = m_Grid.begin(); node != m_Grid.end(); ++node)
	{
		for (body = (*node).begin(); body != (*node).end(); ++body)
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
	BodyList::iterator body = m_BodiesToUpdate.begin();
	for (; body != m_BodiesToUpdate.end(); body++)
	{
		// Tell the body it can request an update again if it wants
		(*body)->_notifyCGUpdated();

		// Find the node the body should move to, based on its physical location
		int pos = _getGridPosition((*body));
		// Check if the object actually needs to be updated :P
		if (pos != (*body)->_getCGPos())
		{
			// Remove the body form its current position in the grid

			// Each body stores its node index
			BodyList *node = &m_Grid[(*body)->_getCGPos()];

			// Find the body in its (old) node
			{
				BodyList::iterator node_it = node->begin();
				for (; node_it != node->end(); ++node_it)
				{
					if ((*node_it) == (*body))
					{
						node->erase(node_it);
						break;
					}
				}
			}

			// Put the body into its new node
			(*body)->_setCGPos(pos);
			(*body)->_setCCIndex(m_Grid[pos].size());

			m_Grid[pos].push_back((*body));
		}
	}

	m_BodiesToUpdate.clear();
}

void FusionPhysicsCollisionGrid::SetScale(float scale, int level_x, int level_y)
{
	cl_assert(scale > 0.0f && scale <= 1.0f);

	m_GridScale = scale;
	m_GridWidth = int((level_x * scale) + 0.5);
	m_GridHeight = int((level_y * scale) + 0.5);

	cl_assert(m_GridWidth * m_GridHeight < m_Grid.max_size());

	// Preallocate the grid.
	m_Grid.resize(m_GridWidth * m_GridHeight);
}

BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(FusionEngine::FusionPhysicsBody *body)
{
	return _findAdjacentBodies(body->_getCGPos());
}

BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(int x, int y)
{
	return _findAdjacentBodies(_getGridPosition(x, y));
}

BodyList FusionPhysicsCollisionGrid::_findAdjacentBodies(int cell_index)
{
	//! Number of cells accross/down from the given cell considered adjcent
	static const int _adjStart = -1, _adjEnd = 1;
	static const int _adjWidth = _adjEnd-_adjStart;

	int xStart, xEnd, yStart, yEnd, croppedWidth, croppedHeigth;


	// Constrain xStart+mid and xEnd+mid to the collision grid borders
	xStart = fe_max(_adjStart, _adjStart - ((cell_index + _adjStart) % m_GridWidth));
	xEnd = fe_min(_adjEnd, _adjEnd - ((cell_index+1 + _adjEnd) % m_GridWidth));

	// Constrain yStart and yEnd to the collision grid borders
	yStart = fe_max(_adjStart, -(int)(cell_index/m_GridWidth));
	yEnd = fe_min(_adjEnd, m_GridHeight-(int)(cell_index/m_GridWidth));

	// Constrained (cropped) width and height
	croppedWidth = xEnd-xStart;
	croppedHeigth = yEnd-yStart;


	// Size for each adjacent cell, aggregateSize
	// (notice max width&height, not cropped values, for safety)
	int size[_adjWidth*_adjWidth], aggrSize = 0;

	// ??? What's faster, checking how big the list needs to be then allocating
	//  and assigning it (in two loops - i.e. the method implemented below),
	//  or dynamicaly resizing it as necessary while assigning it in one loop?
	for (int y = yStart; y < yEnd; y++)
	{
		for (int x = xStart; x < xEnd; x++)
		{
			int xy_pos = cell_index + y * m_GridWidth + x;
			int s = m_Grid[xy_pos].size();

			size[y*_adjWidth+x] = s;
			aggrSize += s;
		}
	}

	// Create a container of the calculated size
	BodyList bodies(aggrSize);

	BodyList::iterator dest_it = bodies.begin();
	for (int y = yStart; y < yEnd; y++)
	{
		for (int x = xStart; x < xEnd; x++)
		{
			int xy_pos = cell_index + y * m_GridWidth + x;
			BodyList* source_cell = &m_Grid[xy_pos];

			std::copy(source_cell->begin(), source_cell->end(), dest_it);

			// Record the current position in the list
			dest_it += size[y*_adjWidth+x];
		}
	}

	return bodies;
}

void FusionPhysicsCollisionGrid::_updateThis(FusionEngine::FusionPhysicsBody *body)
{
	if (!body->_CGwillUpdate())
	{
		// Make sure the body doesn't try to update again till Resort has been called.
		body->_notifyCGwillUpdate();
		m_BodiesToUpdate.push_back(body);
	}
}

/* [removed] Pointer used now
void FusionPhysicsCollisionGrid::_updateThis(int cgind)
{
	m_BodiesToUpdate.push_back(cgind);
}
*/

unsigned int FusionPhysicsCollisionGrid::_getGridPosition(FusionEngine::FusionPhysicsBody *body)
{
	unsigned int x, y;
	x = (unsigned int)(body->GetPosition().x * m_GridScale);
	y = (unsigned int)(body->GetPosition().y * m_GridScale);

	return _getGridPosition(x, y);
}

unsigned int FusionPhysicsCollisionGrid::_getGridPosition(int x, int y)
{
	unsigned int pos;

	pos = y * m_GridWidth + x;

	// Make sure the pos found is inside the grid
	//cl_assert(pos < m_GridWidth * m_GridHeight);
	fe_clamp<unsigned int>(pos, 0, m_Grid.size()-1);

	return pos;
}