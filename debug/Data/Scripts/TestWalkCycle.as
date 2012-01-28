#uses ITransform
#uses IRigidBody
#uses ISprite sprite_left
#uses ISprite sprite_up
#uses ISprite sprite_right
#uses ISprite sprite_down
#uses ISprite sprite_idle


class TestWalkCycle : ScriptComponent
{
	TestWalkCycle()
	{
		console.println("--TestB--");

		frames = 0;
		foo = 1;
		speed = 1.0f;
		go = false;
		quicksaveSlot = 0;
	}
	
	~TestWalkCycle()
	{
		console.println("--~TestB--");
	}

	uint frames;
	uint foo;
	private bool go;
	float speed;
	
	uint quicksaveSlot;

	void onInput(InputEvent@ ev)
	{
		if (ev.isDown)
		{
			if (ev.inputName == "quicksave")
			{
				game.save("quicksave", true);
				game.save("quicksave" + quicksaveSlot, true);
				++quicksaveSlot;
				if (quicksaveSlot > 4)
					quicksaveSlot = 0;
			}
			if (ev.inputName == "quickload")
			{
				game.load("quicksave");
			}
		}
	}

	void update()
	{
		++frames;
		
		Vector currentVelocity;
		if (entity.input.getButton("up"))
		{
			currentVelocity.y = currentVelocity.y - speed;
		}
		if (entity.input.getButton("down"))
		{
			//console.println("at " + speed);
				currentVelocity.y = currentVelocity.y + speed;
		}
		if (entity.input.getButton("left"))
		{
				currentVelocity.x = currentVelocity.x - speed;
		}
		if (entity.input.getButton("right"))
		{
				currentVelocity.x = currentVelocity.x + speed;
		}
		irigidbody.Velocity = currentVelocity;
		
		sprite_left.Alpha = 0.0f;
		sprite_up.Alpha = 0.0f;
		sprite_right.Alpha = 0.0f;
		sprite_down.Alpha = 0.0f;
		sprite_idle.Alpha = 0.0f;
		
		if (irigidbody.Velocity.value.x < -0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:left";
			sprite_left.Playing = true;
			sprite_left.Alpha = 1.0f;
		}
		else if (irigidbody.Velocity.value.x > 0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:right";
			sprite_right.Playing = true;
			sprite_right.Alpha = 1.0f;
		}
		float xvel = irigidbody.Velocity.value.x;
		bool movingX = xvel > 0.01f || xvel < -0.01f;
		if (irigidbody.Velocity.value.y < -0.01f && !movingX)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:up";
			sprite_up.Playing = true;
			sprite_up.Alpha = 1.0f;
		}
		else if (irigidbody.Velocity.value.y > 0.01f && !movingX)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:down";
			sprite_down.Playing = true;
			sprite_down.Alpha = 1.0f;
		}
		if (irigidbody.Velocity.value.x < 0.01f && irigidbody.Velocity.value.x > -0.01f &&
			irigidbody.Velocity.value.y < 0.01f && irigidbody.Velocity.value.y > -0.01f)
		{
			sprite_down.Playing = false;
			sprite_idle.Alpha = 1.0f;
		}
	}
}
