void seekerCreation(uint16 ind)
{
	// ind is the object index of this object
	// I just made this to show that you can have a creation method for projectiles... I don't have anything important for it to do...
	
	// Applies a force toward the current facing
	ApplyEngineForce(ind, m_engineforce);
}

void seekerStep(uint16 ind)
{
	// Finds the ship nearest this ship and moves towards it
	float distance = GetDistanceToNearestShip(ind);
	if (distance < 100)
	{
		if (GetObjectVelocity.length() <= Get_engineforce(ind))
		{
			Vector pos = GetObjectPosition(ind);
			CreateEffect("targetAquired", pos.x, pos.y);
		}
		
		// If we're near a ship, move faster
		Vector aim = GetUnitVectorToNearestShip();
		aim.x = aim.x * Get_engineforce(ind) + 0.2;
		aim.y = aim.y * Get_engineforce(ind) + 0.2;
		// Applys the given force vector
		ApplyForce(ind, aim);
	}
	else
	{
		// If we aren't near a ship, move slower
		ApplyEngineForce(ind);
	}
}

void clusterStep()
{
	// Applys a force towards the current facing
	ApplyEngineForce(ind);
}

void seekerOnCollision(uint16 ind, int x, int y)
{
	// Mwahahahaha
	for (int i = 0; i<10; i+=1)
	{
		a = 36 * i;
		// Creates a projectile of the given type at the given coords and starting angle.
		CreateProjectile(GetOwner(ind), "cluster", x, y, a);
	}
	Detonate(ind);
}

void clusterOnCollision(uint16 ind, int x, int y)
{
	// Deletes this projectile, after doing the damage specified in the XML config
	Detonate(ind);
}

void onFire(uint16 ship)
{
	// FireProjectile creates a projectile in the appropriate location for the ship firing it
	FireProjectile(ship, "seeker");
}