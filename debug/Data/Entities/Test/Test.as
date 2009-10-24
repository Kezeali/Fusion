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

	Test()
	{
		console.println("'Test' entity created");
		runningtime = 0;
	}
	~Test()
	{
		console.println("'Test' entity deleted");
	}

	void Spawn()
	{
		console.println("'Test' entity Spawned");

		//@gui_entity = cast<TestGUI>( entity_manager.instance("TestGUI", "test_gui", GetOwnerID()) );
		//gui_entity.Spawn();
		//@gui_entity = null;
		
		first = true;
	}

	void Draw()
	{
	}

	void DebugOutput()
	{
		const Vector@ p = GetPosition();
		const Vector@ v = GetVelocity();
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: " + v.x + ", " + v.y);
		console.println("r: " + GetAngle());
		//console.println("w: " + getAngularVelocity());
	}

	uint runningtime;
	bool first;
	bool didOutput;
	bool specialPressed;
	void Update(float dt)
	{
		if (first)
		{
			seed_rand(dt);

			//bgm.play();

			first = false;
		}

		if (InputIsActive("quit"))
			system.quit();

		bool doOutput = InputIsActive("debug");
		if (doOutput && !didOutput)
			DebugOutput();
		didOutput = doOutput;

		bool special = InputIsActive("special");
		if (special && !specialPressed)
			editor.startEditor();
		specialPressed = special;

		bool forward = InputIsActive("thrust");
		bool left = InputIsActive("left");
		bool right = InputIsActive("right");

		if (forward)
		{
			if (!input_forward)
				movesound.play();
			//ApplyForce(GetWorldVector(0, -32));
			SetVelocity(GetWorldVector(0, -80));
		}

		if (left)
		{
			SetAngularVelocity(-1.6);
		}
		else if (!left && input_left)
			ApplyTorque(0.54);

		if (right)
		{
			SetAngularVelocity(1.6);
		}
		else if (!right && input_right)
			ApplyTorque(-0.54);

		input_forward = forward;
		input_left = left;
		input_right = right;
	}
}
