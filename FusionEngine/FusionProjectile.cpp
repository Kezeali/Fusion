
#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionProjectile.h"

using namespace FusionEngine;

FusionProjectile::FusionProjectile()
{
	/// State
	CurrentState.position = CL_Vector2::ZERO;
	CurrentState.velocity = CL_Vector2::ZERO;
	CurrentState.facing = 0;
	CurrentState.spin = 0;

	CurrentState.mass = 0;
	CurrentState.current_primary = -1;
	CurrentState.current_secondary = -1;
	CurrentState.current_bomb = -1;
	CurrentState.engines = LEFT | RIGHT;
	CurrentState.weapons = PRIMARY | SECONDARY;
}

FusionProjectile::FusionProjectile(ShipState initialState)
{
	/// State
	CurrentState = initialState;
}

FusionProjectile::~FusionProjectile()
{
}