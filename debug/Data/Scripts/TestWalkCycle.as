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
	}

	void update()
	{
		++frames;
		
		Vector currentVelocity;
		if (entity.input.getButton("up"))
		{
			currentVelocity.y -= speed;
		}
		if (entity.input.getButton("down"))
		{
			//console.println("at " + speed);
				currentVelocity.y += speed;
		}
		if (entity.input.getButton("left"))
		{
				currentVelocity.x -= speed;
		}
		if (entity.input.getButton("right"))
		{
				currentVelocity.x += speed;
		}
		irigidbody.Velocity = currentVelocity;
		
		sprite_left.Alpha = 0.0f;
		sprite_up.Alpha = 0.0f;
		sprite_right.Alpha = 0.0f;
		sprite_down.Alpha = 0.0f;
		sprite_idle.Alpha = 1.0f;
		
		if (irigidbody.Velocity.value.x < -0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:left";
			sprite_left.Playing = true;
			sprite_up.Alpha = 1.0f;
			sprite_right.Alpha = 1.0f;
			sprite_down.Alpha = 1.0f;
		}
		else if (irigidbody.Velocity.value.x > 0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:right";
			sprite_right.Playing = true;
			sprite_up.Alpha = 1.0f;
			sprite_left.Alpha = 1.0f;
			sprite_down.Alpha = 1.0f;
		}
		if (irigidbody.Velocity.value.y < -0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:up";
			sprite_up.Playing = true;
			sprite_left.Alpha = 1.0f;
			sprite_right.Alpha = 1.0f;
			sprite_down.Alpha = 1.0f;
		}
		else if (irigidbody.Velocity.value.y > 0.01f)
		{
			//isprite.AnimationPath << "/Entities/character/walk_cycle2.yaml:down";
			sprite_down.Playing = true;
			sprite_up.Alpha = 1.0f;
			sprite_right.Alpha = 1.0f;
			sprite_left.Alpha = 1.0f;
		}
		if (irigidbody.Velocity.value.x < 0.01f && irigidbody.Velocity.value.x > -0.01f &&
			irigidbody.Velocity.value.y < 0.01f && irigidbody.Velocity.value.y > -0.01f)
		{
			sprite_down.Playing = false;
			sprite_idle.Alpha = 1.0f;
		}
	}
}
