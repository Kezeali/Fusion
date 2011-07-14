class Test
{
	Test()
	{
		console.println("test");
		frames = 0;
		runtime = 0.0;
	}

	uint frames;
	float runtime;

	uint fib(uint n)
	{
		return n < 2 ? n : fib(n-1) + fib(n-2);
	}

	void update(float delta)
	{
		//if (frames % 10 == 0)
		//	console.println("update(" + delta + ") - frame: " + frames + " - runtime: " + runtime + " (seconds)");
		//fib(10);
		++frames;
		runtime += delta;
	}
}
