
/// Class
#include "FusionLevel.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	void Level::MakeHole(int x, int y, int radius)
	{
		Hole h;
		h.x = x; h.y = y;
		h.radius = radius;

		m_Holes.push_back(h);
	}

	Hole Level::PopNextHole()
	{
		Hole h = m_Holes.front();
		m_Holes.pop_front();

		return h;
	}

	void Level::_madeHole(const CL_Surface *surface)
	{
		m_Bitmask.SetFromSurface(surface, m_World->GetBitmaskRes(), 128);
	}

}
