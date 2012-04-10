#uses ITransform
#uses IRigidBody
#uses ISprite
#autoyield disable

class TestB : ScriptComponent
{
	TestB()
	{
		console.println("--TestB--");

		frames = 0;
		foo = 1;
		speed = 1.0f;
		go = false;
	}
	
	~TestB()
	{
		console.println("--~TestB--");
	}

	uint frames;
	uint foo;
	private bool go;
	float speed;

	void onInput(InputEvent@ ev)
	{
		//console.println(ev.inputName);
		
		if (ev.inputName == "thrust")
		{
			//console.println("at " + speed);
			if (ev.isDown)
				irigidbody.Velocity << Vector(cos(itransform.Angle) * speed, sin(itransform.Angle) * speed);
			else
				irigidbody.Velocity << Vector(0, 0);
			go = ev.isDown;
		}
		if (ev.inputName == "left")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity << -1.5;
			else
				irigidbody.AngularVelocity << 0;
		}
		if (ev.inputName == "right")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity << 1.5;
			else
				irigidbody.AngularVelocity << 0;
		}
		//if (ev.inputName == "special")
		//{
		//	if (ev.isDown)
		//		irigidbody.Interpolate = !irigidbody.Interpolate;
		//}
	}

	void update()
	{
		++frames;
		
		if (entity.input.getButton("thrust"))
		{
			irigidbody.Velocity << Vector(cos(itransform.Angle) * speed, sin(itransform.Angle) * speed);
		}
		else
			irigidbody.Velocity << Vector();
	}
}
