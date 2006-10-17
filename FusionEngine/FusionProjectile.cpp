
#include "FusionEngineCommon.h"

/// STL

/// Fusion

/// Class
#include "FusionProjectile.h"

using namespace FusionEngine;

FusionProjectile::FusionProjectile()
{
	/// State
	m_CurrentState.PID = 0;
	m_CurrentState.OID = 0;
	m_CurrentState.Position = CL_Vector2::ZERO;
	m_CurrentState.Velocity = CL_Vector2::ZERO;
	m_CurrentState.Rotation = 0;
	m_CurrentState.RotationalVelocity = 0;
}

FusionProjectile::FusionProjectile(ProjectileState initialState)
{
	/// State
	m_CurrentState = initialState;
}

FusionProjectile::~FusionProjectile()
{
}

void FusionProjectile::SetState(ProjectileState state)
{
	m_CurrentState = state;
}

const ProjectileState &FusionProjectile::GetState() const
{
	return m_CurrentState;
}