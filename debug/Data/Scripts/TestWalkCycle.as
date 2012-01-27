#uses ITransform
#uses IRigidBody
#uses ISprite

class TestWalkCycle : ScriptComponent
{
	TestWalkCycle()
	{
		console.println("--TestB--");

		frames = 0;
		foo = 1;
		speed = 1.0f;
		go = false;
	}
	
	~TestWalkCycle()
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
		
		Vector currentVelocity = irigidbody.Velocity.value;
		if (ev.inputName == "up")
		{
			//console.println("at " + speed);
			if (ev.isDown)
			{
				currentVelocity.y = -speed;
				isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:up";
				isprite.Looping = true;
			}
			else
				currentVelocity.y = 0;
			go = ev.isDown;
		}
		if (ev.inputName == "down")
		{
			//console.println("at " + speed);
			if (ev.isDown)
			{
				currentVelocity.y = speed;
				isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:down";
				isprite.Looping = true;
			}
			else
				currentVelocity.y = 0;
			go = ev.isDown;
		}
		if (ev.inputName == "left")
		{
			if (ev.isDown)
			{
				currentVelocity.x = -speed;
				isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:left";
				isprite.Looping = true;
			}
			else
				currentVelocity.x = 0;
		}
		if (ev.inputName == "right")
		{
			if (ev.isDown)
			{
				currentVelocity.x = speed;
				isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:right";
				isprite.Looping = true;
			}
			else
				currentVelocity.x = 0;
		}
		irigidbody.Velocity = currentVelocity;
		//if (ev.inputName == "special")
		//{
		//	if (ev.isDown)
		//		irigidbody.Interpolate = !irigidbody.Interpolate;
		//}
	}

	void update()
	{
		++frames;
		
		Vector currentVelocity;
		if (entity.input.getButton("up"))
		{
			console.println("at " + speed);
			currentVelocity.y = -speed;
		}
		if (entity.input.getButton("down"))
		{
			//console.println("at " + speed);
				currentVelocity.y = speed;
		}
		if (entity.input.getButton("left"))
		{
				currentVelocity.x = -speed;
		}
		if (entity.input.getButton("right"))
		{
				currentVelocity.x = speed;
		}
		irigidbody.Velocity = currentVelocity;
		
		//~ if (irigidbody.Velocity.value.x < -0.01f)
		//~ {
			//~ isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:left";
		//~ }
		//~ else if (irigidbody.Velocity.value.x > 0.01f)
		//~ {
			//~ isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:right";
		//~ }
		//~ if (irigidbody.Velocity.value.y < -0.01f)
		//~ {
			//~ isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:up";
		//~ }
		//~ else if (irigidbody.Velocity.value.y > 0.01f)
		//~ {
			//~ isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:down";
		//~ }
		if (!isprite.AnimationFinished.value &&
			(irigidbody.Velocity.value.x < 0.01f && irigidbody.Velocity.value.x > -0.01f &&
			irigidbody.Velocity.value.y < 0.01f && irigidbody.Velocity.value.y > -0.01f))
		{
			isprite.Looping = false;
		}
	}
}
