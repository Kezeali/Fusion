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
	assert(scale > 0.0f && scale <= 1.0f);

	m_GridScale = scale;
	m_GridWidth = int((level_x * scale) + 0.5);
	m_GridHeight = int((level_y * scale) + 0.5);

	assert(m_GridWidth * m_GridHeight < m_Grid.max_size());

	// Preallocate the grid.
	m_Grid.resize(m_GridWidth * m_GridHeight);
}

FusionPhysicsCollisionGrid::BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(FusionEngine::FusionPhysicsBody *body)
{
	FusionPhysicsCollisionGrid::BodyList bodies;
	FusionPhysicsCollisionGrid::BodyList *node;

	int grid_pos = body->_getCGPos();

	// Static objects (always returned) -- 2006/12/09: statics are now handled entirely by PhysicsWorld, so they don't bounce on contact!
	//bodies.resize(m_Static.size());
	//std::copy(m_Static.begin(), m_Static.end(), bodies.begin());

	// Find adjacent cells and copy their data
	//  Local cell:
	node = &m_Grid[grid_pos];

	bodies.resize(bodies.size() + node->size());
	std::copy(node->begin(), node->end(), bodies.begin());

	//  Left Hand cell (only if this isn't the left edge):
	if (grid_pos % m_GridWidth > 0)
	{
		node = &m_Grid[grid_pos - 1];

		// Store the length before resize
		size_t length = bodies.size();

		bodies.resize(bodies.size() + node->size());
		std::copy(node->begin(), node->end(), bodies.begin() + length);
	}

	//  Right Hand cell:
	if ((grid_pos + 1) % m_GridWidth > 0)
	{
		node = &m_Grid[grid_pos + 1];

		// Store the length before resize
		size_t length = bodies.size();

		bodies.resize(bodies.size() + node->size());
		std::copy(node->begin(), node->end(), bodies.begin() + length);
	}

	//  Cell above:
	if (grid_pos >= m_GridWidth)
	{
		node = &m_Grid[grid_pos - m_GridWidth];
		
		// Store the length before resize
		size_t length = bodies.size();

		bodies.resize(bodies.size() + node->size());
		std::copy(node->begin(), node->end(), bodies.begin() + length);
	}

	//  Cell below:
	if (grid_pos < m_GridWidth * (m_GridHeight -1))
	{
		node = &m_Grid[grid_pos + m_GridWidth];
		
		// Store the length before resize
		size_t length = bodies.size();

		bodies.resize(bodies.size() + node->size());
		std::copy(node->begin(), node->end(), bodies.begin() + length);
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
	unsigned int pos, x, y;
	x = unsigned int(body->GetPosition().x * m_GridScale);
	y = unsigned int(body->GetPosition().y * m_GridScale);

	pos = y * m_GridWidth + x;

	// Make sure the pos found is inside the grid
	//assert(pos < m_GridWidth * m_GridHeight);
	fe_clamp<unsigned int>(pos, 0, m_Grid.size());

	return pos;
}