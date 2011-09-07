#uses ITransform
#uses IRigidBody
#uses ISprite
#uses TestB script_b
#uses PseudoI notai

class SpawnPoint : ScriptComponent
{
	SpawnPoint()
	{
	}

	uint frames;
	float runtime;

	void onSpawn()
	{
	}
	
	EntityWrapper@ createPlayerEntity(Vector &in pos)
	{
		// One possibility is to remove the addComponent method and just have an instantiate method
		//  where you can pass some sort of collection
		Entity newEnt = ontology.instantiate("b2Dynamic", true, pos, 0.f, 1);
		ontology.addComponent(newEnt, "b2Circle", "");
		ontology.addComponent(newEnt, "CLSprite", "");
		ontology.addComponent(newEnt, "TestB", "script_b");
		ISprite@ sprite = cast<ISprite>(newEnt.getComponent("ISprite").get());
		if (sprite is null)
		{
			console.println("sprite cast failed");
			return EntityWrapper();
		}
		//console.println(sprite.getType());
		sprite.ImagePath.value = "Entities/Test/Gfx/spaceshoot_body_moving1.png";
		sprite.BaseAngle = 1.57;
		
		//cast<IRigidBody>(newEnt.getComponent("IRigidBody").get()).AngularVelocity = 1;
		
		cast<ICircleShape>(newEnt.getComponent("ICircleShape").get()).Radius = 0.25f;
		
		cast<ITransform>(newEnt.getComponent("ITransform").get()).Depth = 1;
		
		return EntityWrapper(newEnt);
	}
	
	EntityWrapper@ entityA;
	
	void update()
	{
		++frames;
		if (frames == 1)
		{
			seed_rand(1234);
			
			@entityA = createPlayerEntity(itransform.Position);
			
			entityA.script_b.speed = 3.0f;
			
			isprite.ImagePath = "";
		}

		if (entityA !is null)
		{
			if (frames % 60 == 0)
			console.println("Player Position: " + entityA.itransform.Position.value.x + ", " + entityA.itransform.Position.value.y);
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