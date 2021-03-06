#uses ITransform
#uses IRigidBody
#uses ISprite
#uses TestB script_b
#uses PseudoI notai

class Test : ScriptComponent
{
	Test()
	{
		console.println("--Test--");

		frames = 0;
		runtime = 0.0;
		lastDamping = 0;
		
		spawnerY = 0;
		
		dirtSize = 32;
	}

	uint frames;
	float runtime;

	void coroutine()
	{
		const float frame = 1;
		//console.println("1st yield at " + frame);
		yield();
		//console.println("2nd yield at " + frame);
		yield();
		//console.println("3rd yield at " + frame);
		yield();
		console.println("coroutine done");
	}

	float lastDamping;
	
	void onSpawn()
	{
	}
	
	void createEntity(Vector &in pos)
	{
		//console.println("Entity spawned at " + pos.x + "," + pos.y);
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = instantiator.instantiate("b2Dynamic", true, pos, 0.f, 0);
		instantiator.addComponent(newEnt, "b2Circle", "");
		instantiator.addComponent(newEnt, "CLSprite", "");
		bool script = false;
		//if (rand() < 0.2f)
		//{
			//instantiator.addComponent(newEnt, "TestC", "script_c");
			//script = true;
		//}
		EntityComponent@ com = newEnt.getComponent("ISprite");
		ISprite@ sprite = cast<ISprite>(com);
		if (sprite is null)
		{
			console.println("sprite cast failed");
			return;
		}
		//console.println(sprite.getType());
		sprite.ImagePath = "Entities/Test/Gfx/spaceshoot_body_moving1.png";
		sprite.BaseAngle = 1.57f;
		
		cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).LinearDamping = 1.f;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
		
		if (script && newEnt.getComponent("IScript").get() is null)
			console.println("No Script");
		else
			@entityB = EntityWrapper(newEnt);
	}
	
	void createPI(Vector &in pos, EntityWrapper@ target)
	{
		//console.println("Entity spawned at " + pos.x + "," + pos.y);
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = instantiator.instantiate("b2Dynamic", true, pos, 0.f, 0);
		instantiator.addComponent(newEnt, "b2Circle", "");
		instantiator.addComponent(newEnt, "CLSprite", "");
		instantiator.addComponent(newEnt, "PseudoI", "notai");
		EntityComponent@ com = newEnt.getComponent("ISprite");
		ISprite@ sprite = cast<ISprite>(com);
		sprite.ImagePath = "Entities/Test/Gfx/spaceshoot_body_moving1.png";
		sprite.BaseAngle = 1.57f;
		
		EntityWrapper@ wr = EntityWrapper(newEnt);
		console.println("before setting target");
		@wr.notai.target = target;
		console.println("after setting target");
		
		//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).LinearDamping = 1.f;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
	}

	float rand()
	{
		return 0.5f;
	}
	
	void createDirt(Vector &in pos)
	{
		//console.println("Entity spawned at " + pos.x + "," + pos.y);
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = instantiator.instantiate("StaticTransform", false, pos, rand() * 3.14f, 0);
		instantiator.addComponent(newEnt, "CLSprite", "");
		EntityComponent@ com = newEnt.getComponent("ISprite");
		ISprite@ sprite = cast<ISprite>(com);
		sprite.ImagePath = "Entities/Dirt.png";
		
		cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = -1;
	}
	
	EntityWrapper@ entityA;
	private EntityWrapper@ entityB;
	
	EntityWrapper@ createPlayerEntity(Vector &in pos)
	{
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = instantiator.instantiate("b2Dynamic", true, pos, 0.f, 1);
		instantiator.addComponent(newEnt, "b2Circle", "");
		instantiator.addComponent(newEnt, "CLSprite", "");
		instantiator.addComponent(newEnt, "TestB", "script_b");
		ISprite@ sprite = cast<ISprite>(newEnt.getComponent("ISprite").get());
		if (sprite is null)
		{
			console.println("sprite cast failed");
			return EntityWrapper();
		}
		//console.println(sprite.getType());
		sprite.ImagePath = "Entities/Test/Gfx/spaceshoot_body_moving1.png";
		sprite.BaseAngle = 1.57f;
		
		//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
		
		cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 1;
		
		return EntityWrapper(newEnt);
	}
	
	private uint dirtSize;
	
	private uint spawnerY;
	void dirtSpawner()
	{
		uint y = spawnerY;
		++spawnerY;
		console.println("starting " + y);
		uint hDirtSize = (dirtSize / 2);
		for (uint x = 0; x < dirtSize; ++x)
			createDirt(Vector(x * 1.25f - hDirtSize * 1.25f, y * 1.25f - hDirtSize * 1.25f));
		console.println("finished " + y);
	}

	void update()
	{
		++frames;
		if (frames == 1)
		{
			//seed_rand(1234);
			
			@entityA = createPlayerEntity(Vector(-0.1f, 0.0f));
			createPI(Vector(0.3f, 0.3f), entityA);
			
			entityA.script_b.speed = 3.0f;

			for (uint i = 0; i < dirtSize; ++i)
			{
				createCoroutine("dirtSpawner", i * 0.5f);
				//for (uint x = 0; x < 128; ++x)
				//	createDirt(Vector(x * 1.25 - 64 * 1.25, y * 1.25 - 64 * 1.25));
				//console.println("y: " + y);
				//yield();
			}
			
			//console.println("itransform implemented by: " + itransform.getType());
			//console.println("isprite implemented by: " + isprite.getType());
			//console.println("icircleshape implemented by: " + icircleshape.getType());
			//console.println("iscript implemented by: " + script_a.getType());
			//console.println(itransform.getType());

			//itransform.Angle = 0.7f;
			//itransform.Angle << 0.7f;
			//float angle = itransform.Angle;
			//console.println("Angle: " + angle);
			//Vector pos = itransform.Position;
			//console.println("Position: " + pos.x + ", " + pos.y);
			//console.println("Depth: " + itransform.Depth);

			//irigidbody.AngularDamping.bindProperty(itransform.Angle);

			//coroutine_t @fn = @coroutine;
			//createCoroutine("coroutine");
		}
		//itransform.Depth = (rand() * 20.0 - 10.0);
		
		if (frames > 1 && frames < 256)
		{
			//console.println("frame " + frames);
			uint xframes = frames % 50;
			uint yframes = frames / 50;
			float x = (xframes * 1.8f) - 9.f;
			float y = (yframes * 1.6f) - 6.f;
			createEntity(Vector(x, y));
		}
		
		if (entityA !is null)
		{
			//if (frames % 60 == 0)
			//console.println("Angle: " + double(entityA.itransform.Angle));
		}
		
		if (frames == 10)
		{
			//console.println("testb.frames: " + script_b.frames);
			//script_b.foo = 99;
			//console.println("testb.foo: " + script_b.foo);
			//console.println("entityB.script_b.speed: " + entityB.script_b.speed);
			//entityB.script_b.speed = 2.0;
		}
		
		if (frames == 11)
		{
			//console.println("testb.foo: " + script_b.foo);
			//console.println("entityB.script_b.speed: " + entityB.script_b.speed);
		}

		//if (irigidbody.AngularDamping != lastDamping)
		//{
		//	lastDamping = irigidbody.AngularDamping;
		//	console.println("Damping: " + lastDamping);
		//}

		//if(frames <= 2)
		//{
		//	console.println("updating " + frames);
		//}

		//if (frames % 30 == 0)
		//	console.println("update(" + delta + ") - frame: " + frames + " - runtime: " + runtime + " (seconds)");
		//++frames;
		//runtime += delta;
		//if (frames >= 150)
		//{
		//	frames = 0;
		//	console.println("update" + runtime);
		//}
		//int m = 2;
		//int v = m * m;

		//mandle(5,5);
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