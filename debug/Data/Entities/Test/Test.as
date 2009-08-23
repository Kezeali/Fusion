class Test : ScriptEntity
{
	Body @physBody;

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
		// Physical properties
		@physBody = @Body();
		world.add_body(physBody);

		CircleFixtureDef def;
		//def.setFriction(0.1);
		//def.setRestitution(1);
		def.setDensity(30);
		//def.setLocalPosition(Vector(0,0));
		def.setRadius(25);

		physBody.setPosition(50, 120);
		physBody.createFixture(@def);
		physBody.setMass(0);

		//CircleShape @shape = @CircleShape(physBody, 0.0f, radius);
		//physBody.attach_shape(shape);

		@gui_entity = cast<TestGUI>( entity_manager.instance("TestGUI", "test_gui") );

		console.println("Press [Debug] to print info");

		first = true;
	}

	const Vector@ GetPosition()
	{
		if (physBody !is null)
			return physBody.getPosition();
		else
			return position;
	}

	float GetAngle()
	{
		if (physBody !is null)
			return physBody.getAngle();
		else
			return 0.0;
	}

	void Draw()
	{
		//Vector p; physBody.get_position(p);
		//imgBody.draw(p.x, p.y, physBody.get_angle());
	}

	void DebugOutput()
	{
		Vector p = physBody.getPosition();
		Vector v = physBody.getVelocity();
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: " + v.x + ", " + v.y);
		console.println("r: " + physBody.getAngle());
		//console.println("w: " + physBody.get_rotational_velocity());
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
			physBody.applyForce(Vector(0,5));
			movesound.play();
		}

		if (left && !input_left)
		{
			physBody.applyTorque(-0.6);
		}

		if (right && !input_right)
		{
			physBody.applyTorque(0.6);
		}

		input_forward = forward;
		input_left = left;
		input_right = right;
	}
};
