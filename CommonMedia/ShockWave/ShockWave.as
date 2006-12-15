void jettison()
{
}

void jettisonedStep()
{
	ApplyForce(m_engineforce);
}

void onCollision(int x, int y)
{
	Detonate();
}