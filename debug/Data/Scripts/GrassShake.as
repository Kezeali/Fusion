#uses ITransform
#uses IRigidBody
#uses ISprite
#uses TestWalkCycle walkcycle

class GrassShake : ScriptComponent
{
	GrassShake()
	{
		foo = 1;
	}
	
	~GrassShake()
	{
	}

	uint foo;
	
	void onSensorEnter(CollisionEvent@ ev)
	{
		//~ ITestWalkCycle@ wc = ev.entity.walkcycle;
		//~ if (wc !is null)
		//~ {
			//~ Vector pos = ev.entity.itransform.Position;
			//~ pos.y = pos.y + 0.1f;
			//~ ev.entity.itransform.Position << pos;
		//~ }
		//isprite.AnimationPath << "/Entities/grass_anim.yaml";
		isprite.AnimationFrame << 1;
		//isprite.Looping << true;
		console.println("sensed");
	}
	
	void onSensorExit(CollisionEvent@ ev)
	{
		//~ ITestWalkCycle@ wc = ev.entity.walkcycle;
		//~ if (wc !is null)
		//~ {
			//~ Vector pos = ev.entity.itransform.Position;
			//~ pos.y = pos.y + 0.1f;
			//~ ev.entity.itransform.Position << pos;
		//~ }
		//isprite.AnimationPath << "/Entities/grass_anim.yaml";
		isprite.AnimationFrame << 0;
		//isprite.Looping << false;
	}

	void update()
	{
	}
}
