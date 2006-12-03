
/// Class
#include "FusionPhysicsTerrain.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	FusionPhysicsTerrain::FusionPhysicsTerrain(FusionPhysicsWorld *world)
		: FusionPhysicsStatic(world)
	{
	}

	FusionPhysicsTerrain::~FusionPhysicsTerrain()
	{
	}

	void FusionPhysicsTerrain::MakeHole(int x, int y, int radius)
	{
		Hole h;
		h.x = x; h.y = y;
		h.radius = radius;

		m_Holes.push_back(h);
	}

	Hole FusionPhysicsTerrain::PopNextHole()
	{
		Hole h = m_Holes.front();
		m_Holes.pop_front();

		return h;
	}

	void FusionPhysicsTerrain::_madeHole(const CL_Surface *surface)
	{
		m_Bitmask.SetFromSurface(surface, m_World->GetBitmaskRes(), 128);
	}

}
