
#include "FusionStatePackSync.h"

using namespace FusionEngine;

bool StatePackSync::Update()
{
	if (resources->LoadVerified() == false)
		return false;

	m_ShipResources = resources->GetLoadedShips();
}