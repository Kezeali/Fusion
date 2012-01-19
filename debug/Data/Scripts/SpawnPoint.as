#uses ITransform
#uses IRigidBody
#uses ISprite
#uses TestB script_b
#uses PseudoI notai

class SpawnPoint : ScriptComponent
{
	SpawnPoint()
	{
		playerSprite = "/Entities/Test/Gfx/spaceshoot_body_moving1.png";
		spawnJunk = true;
	}

	uint frames;
	float runtime;
	
	bool spawnJunk;
	
	string playerSprite;

	void onSpawn()
	{
	}
	
	EntityWrapper@ createPlayerEntity(Vector &in pos, PlayerID owner)
	{
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = instantiator.instantiate("b2Dynamic", true, pos, 0.f, owner);
		if (!newEnt.isNull())
		{
		instantiator.addComponent(newEnt, "b2Circle", "");
		instantiator.addComponent(newEnt, "CLSprite", "sprite_main");
		instantiator.addComponent(newEnt, "CLSprite", "sprite_shadow");
		instantiator.addComponent(newEnt, "TestB", "script_b");
		instantiator.addComponent(newEnt, "StreamingCamera", "");
		ISprite@ sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_main").get());
		if (sprite is null)
		{
			console.println("sprite cast failed");
			return EntityWrapper();
		}
		//console.println(sprite.getType());
		sprite.ImagePath = playerSprite;
		sprite.BaseAngle = 1.57f;
		
		ISprite@ shadow = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_shadow").get());
		shadow.ImagePath = "/Entities/Test/Gfx/spaceshoot_body_shadow.png";
		shadow.BaseAngle = 1.57f;
		//shadow.Scale << Vector(1.02f, 1.02f);
		shadow.Alpha = 0.75f;
		shadow.LocalDepth = -1;
		//shadow.Offset << Vector(0.0f, 0.25f);
		
		//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
		
		cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 0;
		
		ICamera@ cam = cast<ICamera>(newEnt.getComponent("ICamera").get());
		cam.AngleEnabled = false;
		if (owner == 1)
			cam.ViewportRect << Rect(0, 0, 0.5f - 0.01f, 1);
		if (owner == 2)
			cam.ViewportRect << Rect(0.5f + 0.01f, 0, 1, 1);
		
		return EntityWrapper(newEnt);
		}
		return null;
	}
	
	EntityWrapper@ createBeachBall(Vector &in pos)
	{
		console.println("Ball spawned at " + pos.x + "," + pos.y);
		Entity newEnt = instantiator.instantiate("b2Dynamic", true, pos, 0.f, 0);
		if (!newEnt.isNull())
		{
		instantiator.addComponent(newEnt, "b2Circle", "");
		instantiator.addComponent(newEnt, "CLSprite", "");
		ISprite@ sprite = cast<ISprite>(newEnt.getComponent("ISprite").get());
		if (sprite is null)
		{
			return EntityWrapper();
		}
		//console.println(sprite.getType());
		sprite.ImagePath = "Entities/Test/Gfx/spaceshoot_body_moving1.png";
		sprite.BaseAngle = 1.57f;
		
		//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
		cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).LinearDamping = 1.f;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
		
		cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 0;
		
		return EntityWrapper(newEnt);
		}
		return null;
	}
	
	EntityWrapper@ entityA;
	
	void update()
	{	
		++frames;
		if (frames == 1)
		{
			seed_rand(1234);
			
			@entityA = createPlayerEntity(itransform.Position, 1);
			EntityWrapper@ entityB = createPlayerEntity(itransform.Position.value + Vector(0.5f, 0.f), 2);
			
			Vector pos = itransform.Position;
			for (uint i = 0; i < 50; ++i)
			{
				pos.x = pos.x + 0.5f;
				if ((i % 7) == 0)
				{
					pos.x = itransform.Position.value.x;
					pos.y = pos.y + 0.5f;
				}
				createBeachBall(pos);
			}
			
			if (entityA !is null)
			{
				entityA.script_b.speed = 3.0f;
			}
			
			if (entityB !is null)
			{
				entityB.script_b.speed = 3.0f;
			}
			
			isprite.ImagePath = "";
		}

		if (entityA !is null)
		{
			if (frames % 60 == 0)
				console.println("Player pos: " + entityA.itransform.Position.value.x + ", " + entityA.itransform.Position.value.y);
		}
	}
}


void mandle(uint ImageHeight, uint ImageWidth)
{
double MinRe = -2.0;
double MaxRe = 1.0;
double MinIm = -1.2;
double MaxIm = MinIm+(MaxRe-MinRe)*ImageHeight/ImageWidth;
double Re_factor = (MaxRe-MinRe)/(ImageWidth-1);
double Im_factor = (MaxIm-MinIm)/(ImageHeight-1);
uint MaxIterations = 30;

for(uint y=0; y<ImageHeight; ++y)
{
    double c_im = MaxIm - y*Im_factor;
    for(uint x=0; x<ImageWidth; ++x)
    {
        double c_re = MinRe + x*Re_factor;

        double Z_re = c_re, Z_im = c_im;
        bool isInside = true;
        for(uint n=0; n<MaxIterations; ++n)
        {
            double Z_re2 = Z_re*Z_re, Z_im2 = Z_im*Z_im;
            if(Z_re2 + Z_im2 > 4)
            {
                isInside = false;
                break;
            }
            Z_im = 2*Z_re*Z_im + c_im;
            Z_re = Z_re2 - Z_im2 + c_re;
        }
        if(isInside)
		{}
    }
}
}