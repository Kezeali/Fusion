void jettison(int p_index)
{
}

void jettisonedStep(uint16 p_index)
{
	ApplyEngineForce(p_index);
}

void onCollision(uint16 p_index, int x, int y)
{
	Detonate(p_index);
}