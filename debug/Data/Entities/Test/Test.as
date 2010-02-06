class Test : ScriptEntity
{
	uint health;

	bool input_forward;
	bool input_left;
	bool input_right;

	uint16 localPlayerNumber;

	//TestGUI@ gui_entity;

	Renderable@ sprite;
	SoundSample@ movesound;
	SoundSample@ bgm;

	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	Test()
	{
		console.println("'Test' entity created");
		runningtime = 0;

		health = 100;
		input_forward = true;
		input_left = false;

		@primaryCamera = @Camera();
		@p1Viewport = @Viewport(0, 0, 1, 1);
		p1Viewport.setCamera(primaryCamera);

		system.addViewport(p1Viewport);

		//primaryCamera.setFollowEntity(testEntity);
		primaryCamera.setFollowMode(CamFollowMode::FollowInstant);

		streamer.setPlayerCamera(0, primaryCamera);

		@primaryCamera = null;
	}
	~Test()
	{
		console.println("'Test' entity deleted");
	}

	void OnAddPlayer(uint16 localPlayer, uint16 player)
	{
		//!!! note that we dont neccesarily need an equivilant OnRemovePlayer method to destroy player ents
		//  because entities owned by players are removed automatically when players leave
	}

	void Spawn()
	{
		console.println("'Test' entity Spawned");

		//system.setAddPlayerCallback(this, "OnAddPlayer", localPlayerNumber);

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
	bool specialPressed; bool savePressed; bool loadPressed;
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

		bool save = InputIsActive("primary");
		if (save && !savePressed)
		{
			system.save("test.save");
		}
		savePressed = save;

		bool load = InputIsActive("secondary");
		if (load && !loadPressed)
		{
			system.load("test.save");
		}
		loadPressed = load;

		bool special = InputIsActive("special");
		if (special && !specialPressed)
		{
			system.save("test.save");
			editor.startEditor();
		}
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
			SetAngularVelocity(-1.6f);
		}
		else if (!left && input_left)
			ApplyTorque(0.54f);

		if (right)
		{
			SetAngularVelocity(1.6f);
		}
		else if (!right && input_right)
			ApplyTorque(-0.54f);

		input_forward = forward;
		input_left = left;
		input_right = right;
	}
}
