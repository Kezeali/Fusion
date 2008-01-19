class ship
{
	Image imgBody;
	Sound[] engineSounds;
	SoundSession currentSound;

	int soundTimer;
	int nextSound;

	double mass;
	double engineForce;
	double rotVelocity;

	Command currentCommand;
	bool commandThrustChanged;

	Body @physBody;

	void Preload()
	{
		imgBody = file.GetImage("body.png");
		engineSounds.resize(2);
		engineSounds[0] = file.GetSound("engine1.wav"); // short name
		engineSounds[1] = resource_manager.GetSound("engine2.wav"); // full name

		soundTimer = 0;
		nextSound = 1;

		XmlDocument entityDoc = file.GetXML("myship.xml");

		console.println("loading: " + entityDoc.find("/entity/name/text()"));

		if (!entityDoc.xpath("/entity/mass/text()", mass))
		{
			console.println("Couldnt load mass :(");
			mass = 1.0;
		}

		if (!entityDoc.xpath("/entity/engineForce/text()", engineForce))
		{
			console.println("Couldnt load speed :(");
			engineForce = 0.1;
		}
		if (!entityDoc.xpath("/entity/rotVelocity/text()", rotVelocity))
		{
			console.println("Couldnt load rotSpeed :(");
			rotVelocity = 0.05;
		}
		float radius;
		if (!entityDoc.xpath("/entity/radius/text()", radius))
		{
			console.println("Couldnt load radius :(");
			radius = 32.0;
		}

		// Display the loaded properties
		console.println("Mass: " + mass);
		console.println("Engine Force: " + engineForce);
		console.println("Rotation speed: " + rotVelocity);
		console.println("Radius: " + radius);

		// Physical properties
		console.println("Creating body");
		@physBody = @Body();
		console.println("Body created.");

		console.println("Adding body to world");
		world.attach_body(physBody);

		console.println("Setting position");
		physBody.set_position(50, 120);

		console.println("Setting mass");
		physBody.set_mass(mass);
		console.println("Success!");

		CircleShape @shape = @CircleShape(physBody, 0.0f, radius);
		console.println("Attaching shape");
		physBody.attach_shape(shape);

		console.println("Press [Primary Fire] to display print info");
	}
	void DebugOutput()
	{
		Vector p; physBody.get_position(p);
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: NOT IMPLEMENTED");
		console.println("r: " + physBody.get_angle());
		console.println("w: " + physBody.get_rotational_velocity());
	}
	void Draw()
	{
		Vector p; physBody.get_position(p);
		imgBody.draw(p.x, p.y, physBody.get_angle());
	}
	void Simulate(uint dt)
	{
		if (currentCommand.thrust)
		{
			// Cycle through sounds at an interval
			soundTimer -= dt;
			if (soundTimer <= 0)
			{
				soundTimer = 2000;
				currentSound.stop();
				currentSound = engineSounds[nextSound].prepare(true);
				currentSound.play();
				nextSound = (nextSound + 1) % engineSounds.length();
			}
			// If thrust has been re-activated before soundTimer = 0, this condition will be true
			else if (!currentSound.is_playing())
				currentSound.play();
		}
		else if (commandThrustChanged)
		{
			currentSound.stop();
		}

		commandThrustChanged = false;
	}

	void SetCommand(Command cmd)
	{
		commandThrustChanged = 
			(currentCommand.thrust != cmd.thrust);

		if (cmd.thrust)
		{
			physBody.apply_thrust(engineForce);

			//if (currentCommand.thrust != cmd.thrust)
			//	currentCommand.button_delta = currentCommand.button_delta | 1;
			//else
			//	currentCommand.button_delta &= ~1;
		}

		if (cmd.left)
		{
			physBody.set_rotational_velocity(-rotVelocity);
		}
		else if (cmd.right)
		{
			physBody.set_rotational_velocity(rotVelocity);
		}
		else
		{
			//console.println("Setting omega to 0");
			physBody.set_rotational_velocity(0);
			//console.println("Omega: " + physBody.get_rotational_velocity());
		}

		currentCommand = cmd;

		// Print debug info
		if (cmd.primary_fire)
			DebugOutput();
	}
};
