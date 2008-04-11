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
		XmlDocument entityDoc = file.GetXML("myship.xml");

		console.println("Loading: " + entityDoc.find("/entity/name/text()"));

		string body_file = entityDoc.xpath_string("/entity/images/body/text()");
		string sound1_file = entityDoc.xpath_string("/entity/sounds/engine1/text()");
		string sound2_file = entityDoc.xpath_string("/entity/sounds/engine2/text()");

		if (!entityDoc.xpath("/entity/mass/text()", mass))
		{
			console.println("Couldn't load mass :(");
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

		if (!body_file.empty())
			imgBody = file.GetImage(body_file);
		else
			console.println("Preload failed: image tag incomplete");

		if (!sound1_file.empty() && !sound2_file.empty())
		{
			engineSounds.resize(2);
			engineSounds[0] = file.GetSound(sound1_file); // short name for the res_man object ('file')
			engineSounds[1] = resource_manager.GetSound(sound2_file); // full name for the res_man object
		}
		else
			console.println("Preload failed: one or more sound tags incomplete");

		soundTimer = 0;
		nextSound = 1;

		// Physical properties
		@physBody = @Body();
		world.attach_body(physBody);

		physBody.set_position(50, 120);
		physBody.set_mass(mass);

		CircleShape @shape = @CircleShape(physBody, 0.0f, radius);
		physBody.attach_shape(shape);

		console.println("Press [Primary Fire] to print info");
	}
	void DebugOutput()
	{
		Vector p = physBody.get_position();
		Vector v = physBody.get_velocity();
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: " + v.x + ", " + v.y);
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

	void SetCommand(int tick, int player)
	{
		// The new method:
		if (input.is_active(player, tick, "thrust"))
			physBody.apply_thrust(engineForce);
		// ... this way 'input' (InputHandler) can store, serialize / deserialize
		//  command objects any way it likes and thus we have a central location for
		//  the back buffers, packet data generation, and using input (as above).


		//commandThrustChanged = 
		//	(currentCommand.thrust != cmd.thrust);

		//if (cmd.thrust)
		//{
		//	physBody.apply_thrust(engineForce);
		//}

		//if (cmd.left)
		//{
		//	physBody.set_rotational_velocity(-rotVelocity);
		//}
		//else if (cmd.right)
		//{
		//	physBody.set_rotational_velocity(rotVelocity);
		//}
		//else
		//{
		//	//console.println("Setting omega to 0");
		//	physBody.set_rotational_velocity(0);
		//	//console.println("Omega: " + physBody.get_rotational_velocity());
		//}

		//currentCommand = cmd;

		//// Print debug info
		//if (cmd.primary)
		//	DebugOutput();
	}
};
