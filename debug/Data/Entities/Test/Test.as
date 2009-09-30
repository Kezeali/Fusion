class Test : ScriptEntity
{
	uint health;

	bool input_forward;
	bool input_left;
	bool input_right;

	string message;

	TestGUI@ gui_entity;

	Renderable@ sprite;
	SoundSample@ movesound;
	SoundSample@ bgm;

	Vector position;

	Test()
	{
		BindSimpleCommands();
		position = Vector(150.0, 50.0);
		runningtime = 0;
	}
	~Test()
	{
	}

	void Spawn()
	{
		//CircleShape @shape = @CircleShape(physBody, 0.0f, radius);
		//physBody.attach_shape(shape);

		@gui_entity = cast<TestGUI>( entity_manager.instance("TestGUI", "test_gui") );

		console.println("Press [Debug] to print info");

		first = true;
	}

	void Draw()
	{
		//Vector p; physBody.get_position(p);
		//imgBody.draw(p.x, p.y, physBody.get_angle());
	}

	void DebugOutput()
	{
		Vector p = GetPosition();
		Vector v = GetVelocity();
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: " + v.x + ", " + v.y);
		console.println("r: " + GetAngle());
		//console.println("w: " + getAngularVelocity());
	}

	uint runningtime;
	bool first;
	void Update(float dt)
	{
		if (first)
		{
			seed_rand(dt);

			bgm.play();

			first = false;
		}

		if (InputIsActive("quit"))
			system.quit();

		bool forward = InputIsActive("thrust");
		bool left = InputIsActive("left");
		bool right = InputIsActive("right");

		if (forward && !input_forward)
		{
			ApplyForce(Vector(0,5));
			movesound.play();
		}

		// TODO:
		//if (left && !input_left)
		//{
		//	ApplyTorque(-0.6);
		//}

		//if (right && !input_right)
		//{
		//	ApplyTorque(0.6);
		//}

		input_forward = forward;
		input_left = left;
		input_right = right;
	}
};
