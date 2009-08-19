class ScriptEntity : IEntity
{
	Entity@ __appObject;
	void _setAppObject(Entity@ obj)
	{
		@__appObject = @obj;
	}
	Entity@ _getAppObject()
	{
		return __appObject;
	}

	uint16 GetOwnerID() const
	{
		return 0;//return __appObject.getOwnerID();
	}

	bool InputIsActive(const string@ input)
	{
		return __appObject.inputIsActive(input);
	}
	float GetInputPosition(const string@ input)
	{
		return __appObject.getInputPosition(input);
	}

	void Spawn() {}
	void Update() {}
	void Draw() {}
}

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
		def.setRadius(30);

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

		bool forward = InputIsActive("thrust");
		bool left = InputIsActive("left");
		bool right = InputIsActive("right");

		if (forward && !input_forward)
		{
			physBody.applyForce(Vector(0.003,0));
			movesound.play();
		}

		if (left && !input_left)
		{
			physBody.applyTorque(-0.001);
		}

		if (right && !input_right)
		{
			physBody.applyTorque(0.001);
		}

		input_forward = forward;
		input_left = left;
		input_right = right;

		//if (input.is_active(player, "thrust"))
		//{
			//physBody.apply_thrust(engineForce);

			//// Cycle through sounds at an interval
			//soundTimer -= dt;
			//if (soundTimer <= 0)
			//{
			//	soundTimer = 2000;
			//	currentSound.stop();
			//	currentSound = engineSounds[nextSound].prepare(true);
			//	currentSound.play();
			//	nextSound = (nextSound + 1) % engineSounds.length();
			//}
			//// If thrust has been re-activated before soundTimer = 0, this condition will be true
			//else if (!currentSound.is_playing())
			//	currentSound.play();
		//}
		//else if (commandThrustChanged)
		//{
		//	//currentSound.stop();
		//}

		//commandThrustChanged = false;
	}
};
