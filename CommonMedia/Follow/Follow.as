void step(uint16 ind)
{
	// Finds the ship nearest this ship and moves towards it
	float distance = GetDistanceToNearestShip(ind);
	if (distance < 100)
	{
		if (GetObjectVelocity(ind).length() <= Get_engineforce(ind))
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


void OnCollision(uint16 ind, int x, int y)
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
