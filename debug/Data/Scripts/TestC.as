#uses ITransform

class TestC : ScriptComponent
{
	TestC()
	{
		//console.println("--TestC--");

		foo = 1;
		lastFoo = 1;
		go = false;
	}

	private uint lastFoo;
	uint foo;
	private bool go;

	void update()
	{
		if (foo != lastFoo)
		{
			console.println("Foo changed: " + foo);
			lastFoo = foo;
		}
	}
}
