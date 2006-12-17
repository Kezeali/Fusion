void seekerCreation()
{
	// I just made this to show that you can have a creation method for projectiles... I don't have anything important for it to do...
	
	// Applies a force toward the current facing
	ApplyEngineForce(m_engineforce);
}

void seekerStep()
{
	// Finds the ship nearest this ship and moves towards it
	float distance = GetDistanceToNearestShip();
	if (distance < 100)
	{
		if (m_velocity.length() <= m_engineforce)
		{
			CreateEffect("targetAquired", m_position.x, m_position.y);
		}
		
		// If we're near a ship, move faster
		Vector aim = GetUnitVectorToNearestShip();
		aim.x = aim.x * m_engineforce + 0.2;
		aim.y = aim.y * m_engineforce + 0.2;
		// Applys the given force vector
		ApplyForce(aim);
	}
	else
	{
		// If we aren't near a ship, move slower
		ApplyEngineForce(m_engineforce);
	}
}

void clusterStep()
{
	// Applys a force towards the current facing
	ApplyForce(m_engineforce);
}

void seekerOnCollision(int x, int y)
{
	// Mwahahahaha
	for (int i = 0; i<10; i+=1)
	{
		a = 36 * i;
		// Creates a projectile of the given type at the given coords and starting angle.
		CreateProjectile("cluster", x, y, a);
	}
	Detonate();
}

void clusterOnCollision(int x, int y)
{
	// Deletes this projectile, after doing the damage specified in the XML config
	Detonate();
}

void onFire()
{
	// FireProjectile creates a projectile in the appropriate location for the ship firing it
	FireProjectile("seeker");
}