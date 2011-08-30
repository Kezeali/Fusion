#uses ITransform
#uses IRigidBody

class TestB : ScriptComponent
{
	TestB()
	{
		console.println("--TestB--");

		frames = 0;
		foo = 1;
		speed = 1.0f;
		go = false;
		@setcampos_con = null;
	}
	
	~TestB()
	{
		console.println("--~TestB--");
	}

	uint frames;
	uint foo;
	private bool go;
	float speed;
	
	private Camera cam;
	
	void onInput(InputEvent@ ev)
	{
		//console.println(ev.inputName);
		if (ev.inputName == "thrust")
		{
			//console.println("at " + speed);
			if (ev.isDown)
				irigidbody.Velocity = Vector(cos(itransform.Angle.value) * speed, sin(itransform.Angle.value) * speed);
			else
				irigidbody.Velocity = Vector(0, 0);
			go = ev.isDown;
		}
		if (ev.inputName == "left")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity = -1.5;
			else
				irigidbody.AngularVelocity = 0;
		}
		if (ev.inputName == "right")
		{
			if (ev.isDown)
				irigidbody.AngularVelocity = 1.5;
			else
				irigidbody.AngularVelocity = 0;
		}
		if (ev.inputName == "special")
		{
			if (ev.isDown)
				irigidbody.Interpolate.value = !irigidbody.Interpolate.value;
		}
	}
	
	private SignalConnection@ setcampos_con;
	void setCameraPosition(const Vector &in pos)
	{
		cam.setPosition(pos);
	}

	void update()
	{
		++frames;
		
		if (frames == 1)
		{
			cam = Camera(itransform.Position);
			streaming.addCamera(cam);
			renderer.addViewport(cam);
			
			@setcampos_con = itransform.Position.connect("void setCameraPosition(const Vector &in)");
		}
		//if (frames > 1)
		//	cam.setPosition(itransform.Position);
		
		if (entity.input.getButton("thrust"))
		{
			irigidbody.Velocity = Vector(cos(itransform.Angle.value) * speed, sin(itransform.Angle.value) * speed);
		}
	}
}
