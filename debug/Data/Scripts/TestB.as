#uses ITransform
#uses IRigidBody

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
	
	void onInput(InputEvent@ ev)
	{
		float speed = 1.0f;
		//string inName = ev.inputName;
		//console.println(inName);
		if (ev.inputName == "thrust")
		{
			//console.println("thrust");
			if (ev.isDown)
				irigidbody.Velocity = Vector(cos(itransform.Angle.value) * speed, sin(itransform.Angle.value) * speed);
			else
				irigidbody.Velocity = Vector(0, 0);
		}
		if (ev.inputName == "left")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity = -1;
			else
				irigidbody.AngularVelocity = 0;
		}
		if (ev.inputName == "right")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity = 1;
			else
				irigidbody.AngularVelocity = 0;
		}
	}

	void update()
	{
		++frames;
	}
}
