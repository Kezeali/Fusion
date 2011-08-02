#uses ITransform
#uses IRigidBody
#uses TestB script_b

class Test : ScriptComponent
{
	Test()
	{
		//console.println("--Test--");

		frames = 0;
		runtime = 0.0;
		lastDamping = 0;
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

	void update(float delta)
	{
		++frames;
		if (frames == 1)
		{
			//console.println("itransform implemented by: " + itransform.getType());
			//console.println("isprite implemented by: " + isprite.getType());
			//console.println("icircleshape implemented by: " + icircleshape.getType());
			//console.println("iscript implemented by: " + script_a.getType());
			//console.println(itransform.getType());

			itransform.Angle.value = 0.7f;
			//itransform.Angle << 0.7;
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
		
		if (frames == 3)
		{
			//console.println("testb.frames: " + script_b.frames);
			script_b.foo = 99;
			console.println("testb.foo: " + script_b.foo);
		}
		
		if (frames == 4)
		{
			console.println("testb.foo: " + script_b.foo);
		}

		//if (irigidbody.AngularDamping.value != lastDamping)
		//{
		//	lastDamping = irigidbody.AngularDamping.value;
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