
/// Class
#include "FusionLevel.h"

/// Fusion
#include "FusionPhysicsWorld.h"

namespace FusionEngine
{

	Level::Level(FusionPhysicsBody* body, FusionNode* node)
		: m_TerrainBody(body),
		m_Node(node)
	{
		m_BitmaskRes = body->GetColBitmask()->GetPPB();
		//surface, m_World->GetBitmaskRes(), 128
	}

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

	void Level::_madeHole(Hole h)
	{
		FusionBitmask* terrainMask = m_TerrainBody->GetColBitmask();
		FusionBitmask* holeMask = new FusionBitmask(h.radius, m_BitmaskRes);
		terrainMask->Erase(holeMask, CL_Point(h.x, h.y));
	}

	void Level::RunRegionScripts(const FusionEngine::FusionShip *ship)
	{
		RegionList::iterator it = m_Regions.begin();
		for (; it != m_Regions.end(); ++it)
		{
			CL_Pointf position = ship->GetPosition();
			if (1) {}
		}

	}

	float Level::LeastCostEstimate(void *stateStart, void *stateEnd)
	{
		// Convert states to points and find the distance
		return 0.0f;
	}

}
