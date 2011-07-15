class Test
{
	Test()
	{
		console.println("--Test--");
		frames = 0;
		runtime = 0.0;

		console.println("itransform implemented by: " + itransform.getType());
		console.println("isprite implemented by: " + isprite.getType());
		console.println("icircleshape implemented by: " + icircleshape.getType());
		console.println("iscript implemented by: " + iscript.getType());
	}

	uint frames;
	float runtime;

	void update(float delta)
	{
		//if (frames % 10 == 0)
		//	console.println("update(" + delta + ") - frame: " + frames + " - runtime: " + runtime + " (seconds)");
		++frames;
		runtime += delta;
	}
}
