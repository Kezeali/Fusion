void step(uint16 ind)
{
	// Finds the ship nearest this ship and moves towards it
	uint16 nearest_ship = GetNearestShip(ind);
	float distance = GetDistanceToShip(ind, nearest_ship);
	if (distance > 100)
	{		
		// If we aren't near a ship, move faster
		Vector aim = GetUnitVectorToShip(ind);
		aim.x = aim.x * Get_engineforce(ind) + 0.2;
		aim.y = aim.y * Get_engineforce(ind) + 0.2;
		// Applys the given force vector
		ApplyForce(ind, aim);
	}
	else
	{
		// If we're  near a ship, move slower
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
