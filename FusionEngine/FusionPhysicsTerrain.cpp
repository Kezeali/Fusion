
#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionPhysicsTerrain.h"

using namespace FusionEngine;

FusionPhysicsTerrain::FusionPhysicsTerrain(FusionPhysicsWorld *world)
: m_World(world)
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

Hole FusionPhysicsTerrain::GetNextHole()
{
	Hole h = m_Holes.front();
	m_Holes.pop_front();

	return h;
}

void FusionPhysicsTerrain::_madeHole(surface)
{
	m_Bitmask.SetFromSurface(surface, m_World->GetBitmaskRes(), 128);
}
