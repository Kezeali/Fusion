#uses ITransform
#uses IRigidBody
#uses ISprite
#uses TestWalkCycle walkcycle
#uses PseudoI notai
#uses ICamera

class SpawnPoint : ScriptComponent
{
	SpawnPoint()
	{
		playerSprite = "/Entities/character/walk_cycle2.png";
		playerAnimationFile = "/Entities/character/walk_cycle2.yaml";
		spawnJunk = true;
		numPlayers = 0;
	}

	uint frames;
	float runtime;
	
	bool spawnJunk;
	
	int numPlayers;
	
	string playerSprite;
	string playerAnimationFile;

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
			instantiator.addComponent(newEnt, "CLSprite", "sprite_idle");
			
			instantiator.addComponent(newEnt, "CLSprite", "sprite_left");
			instantiator.addComponent(newEnt, "CLSprite", "sprite_up");
			instantiator.addComponent(newEnt, "CLSprite", "sprite_right");
			instantiator.addComponent(newEnt, "CLSprite", "sprite_down");
			
			instantiator.addComponent(newEnt, "CLSprite", "sprite_shadow");
			instantiator.addComponent(newEnt, "TestWalkCycle", "walkcycle");
			instantiator.addComponent(newEnt, "StreamingCamera", "");
			ISprite@ sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_idle").get());

			Vector offset = Vector(0, -18.f);
			
			//console.println(sprite.getType());
			sprite.ImagePath = playerSprite;
			sprite.AnimationPath = playerAnimationFile + ":shuffle";
			sprite.Offset << offset;
			//sprite.BaseAngle = 1.57f;
			
			@sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_left").get());
			
			sprite.ImagePath = playerSprite;
			sprite.AnimationPath = playerAnimationFile + ":left";
			sprite.Offset << offset;
			
			@sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_up").get());
			
			sprite.ImagePath = playerSprite;
			sprite.AnimationPath = playerAnimationFile + ":up";
			sprite.Offset << offset;
			
			@sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_right").get());
			
			sprite.ImagePath = playerSprite;
			sprite.AnimationPath = playerAnimationFile + ":right";
			sprite.Offset << offset;
			
			@sprite = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_down").get());
			
			sprite.ImagePath = playerSprite;
			sprite.AnimationPath = playerAnimationFile + ":down";
			sprite.Offset << offset;
			
			ISprite@ shadow = cast<ISprite>(newEnt.getComponent("ISprite", "sprite_shadow").get());
			shadow.ImagePath = "/Entities/character/shadow.png";
			//shadow.BaseAngle = 1.57f;
			//shadow.Scale << Vector(1.02f, 1.02f);
			shadow.Alpha = 0.75f;
			shadow.LocalDepth = -1;
			//shadow.Offset << Vector(0.0f, 0.25f);
			
			//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
			
			cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
			
			cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 0;
			
			/*ICamera@ cam = cast<ICamera>(newEnt.getComponent("ICamera").get());
			cam.AngleEnabled = false;
			
			if (owner == 1)
				cam.ViewportRect << Rect(0, 0, 0.5f - 0.01f, 1);
			if (owner == 2)
				cam.ViewportRect << Rect(0.5f + 0.01f, 0, 1, 1);*/
			
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
			sprite.ImagePath = "Entities/shrub_shadowed.png";
			//sprite.BaseAngle = 1.57f;
			
			//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
			cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).LinearDamping = 1.f;
			
			cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
			
			cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 0;
			
			return EntityWrapper(newEnt);
		}
		return null;
	}
	
	EntityWrapper@ entityA;
	EntityWrapper@ entityB;
	
	private ElementDocument@ doc;
	
	void onPlayerAdded(uint local_num, PlayerID net_num)
	{
		++numPlayers;
		if (local_num == 0 && entityA is null)
		{
			@entityA = createPlayerEntity(itransform.Position, net_num);
			if (entityA !is null)
			{
				entityA.walkcycle.speed = 3.0f;
			}
			
			entityA.icamera.AngleEnabled = false;
		}
		if (local_num == 1 && entityB is null)
		{
			@entityB = createPlayerEntity(itransform.Position.value + Vector(0.5f, 0.f), net_num);
			if (entityB !is null)
			{
				entityB.walkcycle.speed = 3.0f;
			}
			
			entityB.icamera.AngleEnabled = false;
		}
		
		if (numPlayers == 1)
		{
			if (entityA !is null)
				entityA.icamera.ViewportRect << Rect(0, 0, 1, 1);
			if (entityB !is null)
				entityB.icamera.ViewportRect << Rect(0, 0, 1, 1);
		}
		else if (numPlayers == 2)
		{
			if (entityA !is null)
				entityA.icamera.ViewportRect << Rect(0, 0, 0.5f - 0.01f, 1);
			if (entityB !is null)
				entityB.icamera.ViewportRect << Rect(0.5f + 0.01f, 0, 1, 1);
		}
	}
	
	void update()
	{	
		++frames;
		if (frames == 1)
		{
			seed_rand(1234);
			
			numPlayers = 0;
			
			@doc = gui.getContext().LoadDocument("/Entities/gui/add_player.rml");
			doc.Show();
			
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
			
			isprite.ImagePath = "";
		}

		if (entityA !is null)
		{
			if (frames % 120 == 0)
				console.println("Player pos: " + entityA.itransform.Position.value.x + ", " + entityA.itransform.Position.value.y);
		}
	}
}

void OnSelectPlayer(Event@ event)
{
	string numPlayersStr = event.GetParameter(rString("num_players"), rString());
	int numPlayers = parseInt(numPlayersStr);
	for (int i = 0; i < numPlayers; ++i)
		game.requestPlayer();
	
	ElementDocument@ doc = event.GetCurrentElement().GetOwnerDocument();
	doc.Close();
}

void OnSubmitLd(Event@ event)
{
	string saveName = event.GetParameter(rString("filename"), rString());
	
	string numPlayersStr = event.GetParameter(rString("num_players"), rString());
	int numPlayers = parseInt(numPlayersStr);
	for (int i = 0; i < numPlayers; ++i)
		game.requestPlayer();

	game.load(saveName);
	
	ElementDocument@ doc = event.GetCurrentElement().GetOwnerDocument();
	doc.Close();
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