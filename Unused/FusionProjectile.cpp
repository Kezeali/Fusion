
/// Class
#include "FusionProjectile.h"

/// Fusion
#include "FusionScriptingEngine.h"


namespace FusionEngine
{

	FusionProjectile::FusionProjectile()
	{
		// State
		m_CurrentState.PID = 0;
		m_CurrentState.OID = 0;
		m_CurrentState.Position = Vector2::zero();
		m_CurrentState.Velocity = Vector2::zero();
		m_CurrentState.Rotation = 0;
		m_CurrentState.RotationalVelocity = 0;

		// AngelScript
		ScriptingEngine::RegisterScript(m_CollisionScript);
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

}
