#uses ITransform

class TestB : ScriptComponent
{
	TestB()
	{
		//console.println("--TestB--");

		frames = 0;
	}

	uint frames;

	void update(float delta)
	{
		++frames;
	}
}
