#uses ITransform

class TestB : ScriptComponent
{
	TestB()
	{
		//console.println("--TestB--");

		frames = 0;
		foo = 1;
	}

	uint frames;
	uint foo;

	void update(float delta)
	{
		++frames;
	}
}
