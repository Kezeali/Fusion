
#include "Common.h"

#include "FusionPhysicsCollisionGrid.h"

namespace FusionEngine
{

	FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid()
		: m_CellWidth(1),
		m_CellHeight(1),
		m_GridXScale(1.f),
		m_GridYScale(1.f),
		m_GridWidth(1),
		m_GridHeight(1)
	{
	}

	FusionPhysicsCollisionGrid::FusionPhysicsCollisionGrid(int cell_w, int cell_h, int level_w, int level_h)
	{
		SetCellSize(cell_w, cell_h, level_w, level_h);
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
		assert(0);
		//cl_assert(scale > 0.0f && scale <= 1.0f);

		//m_GridXScale = scale;
		//m_GridYScale = scale;

		//m_GridWidth = int((level_x * scale) + 0.5);
		//m_GridHeight = int((level_y * scale) + 0.5);

		//cl_assert(m_GridWidth * m_GridHeight < m_Grid.max_size());

		//// Preallocate the grid.
		//m_Grid.resize(m_GridWidth * m_GridHeight);
	}

	void FusionPhysicsCollisionGrid::SetCellSize(int cell_w, int cell_h, int level_w, int level_h)
	{
		m_CellWidth = cell_w;
		m_CellHeight = cell_h;

		m_GridXScale = 1.0f/cell_w;
		m_GridYScale = 1.0f/cell_h;

		m_GridWidth = (int)(level_w / cell_w)+1; // ceiling of level_w/cell_w
		m_GridHeight = (int)(level_h / cell_h)+1;

		cl_assert((unsigned int)(m_GridWidth*m_GridHeight) < m_Grid.max_size());

		// Preallocate the grid.
		m_Grid.resize(m_GridWidth * m_GridHeight);
	}

	int FusionPhysicsCollisionGrid::GetCellWidth() const
	{
		return m_CellWidth;
	}

	int FusionPhysicsCollisionGrid::GetCellHeight() const
	{
		return m_CellHeight;
	}

	BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(FusionEngine::FusionPhysicsBody *body)
	{
		Vector2 p = body->GetPosition();
		return _findAdjacentBodies(_scaleX(p.x), _scaleY(p.y), body->_getCGPos());
	}

	BodyList FusionPhysicsCollisionGrid::FindAdjacentBodies(float x, float y)
	{
		int gx = _scaleX(x);
		int gy = _scaleY(y);
		return _findAdjacentBodies(gx, gy, _getIndex(gx, gy));
	}

	BodyList FusionPhysicsCollisionGrid::_findAdjacentBodies(int grid_x, int grid_y, int cell_index)
	{
		// Number of cells accross/down from the given cell considered adjcent
		static const int _adjStart = -1, _adjEnd = 1;
		static const int _adjWidth = 3;

		int xStart, xEnd, yStart, yEnd;

		// Constrain xStart and xEnd to the collision grid borders
		xStart = std::max<int>(_adjStart, -grid_x);
		xEnd = std::min<int>(_adjEnd, m_GridWidth - (grid_x+1));
		// Constrain yStart and yEnd to the collision grid borders
		yStart = std::max<int>(_adjStart, -grid_y);
		yEnd = std::min<int>(_adjEnd, m_GridHeight - (grid_y+1));

		// aggregateSize
		int aggrSize = 0;
		// ??? What's faster, checking how big the list needs to be then allocating
		//  and assigning it (in two loops - i.e. the method implemented below),
		//  or dynamicaly resizing it as necessary while assigning it in one loop?
		for (int y = yStart; y <= yEnd; y++)
		{
			for (int x = xStart; x <= xEnd; x++)
			{
				int xy_pos = cell_index + y * m_GridWidth + x;
				if (xy_pos > m_Grid.size()-1)
					continue;

				aggrSize += m_Grid[xy_pos].size();
			}
		}

		// Create a container of the calculated size
		BodyList bodies(aggrSize);

		BodyList::iterator dest_it = bodies.begin();
		for (int y = yStart; y <= yEnd; y++)
		{
			for (int x = xStart; x <= xEnd; x++)
			{
				int xy_pos = cell_index + y * m_GridWidth + x;
				if (xy_pos > m_Grid.size()-1)
					continue;
				BodyList* source_cell = &m_Grid[xy_pos];

				std::copy(source_cell->begin(), source_cell->end(), dest_it);

				// Record the current position in the list
				//dest_it += size[(y-yStart)*_adjWidth+(x-xStart)];
				dest_it += source_cell->size();
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

	unsigned int FusionPhysicsCollisionGrid::_getGridPosition(FusionEngine::FusionPhysicsBody *body) const
	{
		Vector2 p = body->GetPosition();
		return _getGridPosition(p.x, p.y);
	}

	unsigned int FusionPhysicsCollisionGrid::_getGridPosition(float x, float y) const
	{
		// gridx, gridy
		unsigned int gx, gy;

		// Convert the PHYSICAL co-ord to a GRID co-ord
		gx = _scaleX(x);
		gy = _scaleY(y);

		return _getIndex(gx, gy);
	}

	inline unsigned int FusionPhysicsCollisionGrid::_scaleX(float x) const
	{
		return (unsigned int)(x * m_GridXScale);
	}

	inline unsigned int FusionPhysicsCollisionGrid::_scaleY(float y) const
	{
		return (unsigned int)(y * m_GridYScale);
	}

	inline int FusionPhysicsCollisionGrid::_getIndex(int gx, int gy) const
	{
		int index = gy * m_GridWidth + gx;

		// Make sure the index found is inside the grid
		fe_clamp<int>(index, 0, (int)m_Grid.size()-1);

		return (int)index;
	}

	void FusionPhysicsCollisionGrid::DebugDraw() const
	{
		for (int y = 0; y < m_GridHeight; y++)
			for (int x = 0; x < m_GridWidth; x++)
			{
				CL_Pointf point((float)(x*m_CellWidth), (float)(y*m_CellHeight));
				CL_Rectf rect(point, CL_Sizef((float)m_CellWidth, (float)m_CellHeight));

				int size = (int)m_Grid[y*m_GridWidth+x].size();
				if (size == 0)
				{
					CL_Display::draw_rect(rect, CL_Color::white);
				}
				else
				{
					CL_Display::draw_rect(rect, CL_Color::aquamarine);
					//CL_Display::draw_pixel(x*m_PPB + x_offset, y*m_PPB + y_offset, CL_Color::azure);
				}
			}
	}

}


//////////////
// Old Methods

		//int croppedWidth, croppedHeigth;
		//xStart = std::max<int>( _adjStart, -(cell_index%m_GridWidth) );
		//xEnd = std::min<int>( _adjEnd, m_GridWidth-(cell_index%m_GridWidth + 1) );
		//yStart = std::max<int>(_adjStart, -(int)(cell_index/m_GridWidth));
		//yEnd = std::min<int>(_adjEnd, m_GridHeight-(int)(cell_index/m_GridWidth));

		// Constrained (cropped) width and height
		//croppedWidth = xEnd-xStart;
		//croppedHeigth = yEnd-yStart;

	//BodyList FusionPhysicsCollisionGrid::_findAdjacentBodies(int cell_index)
	//{
	//	//! Number of cells accross/down from the given cell considered adjcent
	//	static const int _adjStart = -1, _adjEnd = 1;
	//	static const int _adjWidth = 3;

	//	int xStart, xEnd, yStart, yEnd;//, croppedWidth, croppedHeigth;

	//	// Constrain xStart+mid and xEnd+mid to the collision grid borders
	//	xStart = std::max<int>( _adjStart, -(cell_index%m_GridWidth) );
	//	xEnd = std::min<int>( _adjEnd, m_GridWidth-(cell_index%m_GridWidth + 1) );

	//	// Constrain yStart and yEnd to the collision grid borders
	//	yStart = std::max<int>(_adjStart, -(int)(cell_index/m_GridWidth));
	//	yEnd = std::min<int>(_adjEnd, m_GridHeight-(int)(cell_index/m_GridWidth));

	//	// Constrained (cropped) width and height
	//	//croppedWidth = xEnd-xStart;
	//	//croppedHeigth = yEnd-yStart;


	//	// Size for each adjacent cell
	//	// (notice max width&height, not cropped values, for safety)
	//	//std::vector<int> size(_adjWidth*_adjWidth, 0);
	//	// aggregateSize
	//	int aggrSize = 0;

	//	// ??? What's faster, checking how big the list needs to be then allocating
	//	//  and assigning it (in two loops - i.e. the method implemented below),
	//	//  or dynamicaly resizing it as necessary while assigning it in one loop?
	//	for (int y = yStart; y <= yEnd; y++)
	//	{
	//		for (int x = xStart; x <= xEnd; x++)
	//		{
	//			int xy_pos = cell_index + y * m_GridWidth + x;
	//			int s = m_Grid[xy_pos].size();

	//			//size[(y-yStart)*_adjWidth + (x-xStart)] = s;
	//			aggrSize += s;
	//		}
	//	}

	//	// Create a container of the calculated size
	//	BodyList bodies(aggrSize);

	//	BodyList::iterator dest_it = bodies.begin();
	//	for (int y = yStart; y <= yEnd; y++)
	//	{
	//		for (int x = xStart; x <= xEnd; x++)
	//		{
	//			int xy_pos = cell_index + y * m_GridWidth + x;
	//			BodyList* source_cell = &m_Grid[xy_pos];

	//			std::copy(source_cell->begin(), source_cell->end(), dest_it);

	//			// Record the current position in the list
	//			//dest_it += size[(y-yStart)*_adjWidth+(x-xStart)];
	//			dest_it += source_cell->size();
	//		}
	//	}

	//	return bodies;
	//}

	/* [removed] Pointer used now
	void FusionPhysicsCollisionGrid::_updateThis(int cgind)
	{
	m_BodiesToUpdate.push_back(cgind);
	}
	*/
