#uses ITransform
#uses IRigidBody
//#uses cam

class PseudoI : ScriptComponent
{
	PseudoI()
	{
		//console.println("--TestC--");
		
		frames = 0;
		contact_frames = 0;
	}

	private uint frames;
	
	//private Camera cam;
	
	//Entity target;
	EntityWrapper@ target;
	
	private uint contact_frames;
	
	void update()
	{
		++frames;
		
		if (frames == 1)
		{			
			//entity.irigidbody.AngularVelocity = 0.9f;
			entity.irigidbody.Velocity = Vector(cos(itransform.Angle), sin(itransform.Angle));
			bindMethod("void onVelocityChanged(const Vector &in)", to_placeholder(@entity.irigidbody.VelocityProp));
		}
		//if (frames % 2 == 0)
		//	entity.irigidbody.Velocity = Vector(cos(itransform.Angle), sin(itransform.Angle));
		//if (frames % 100 == 0)
			//entity.irigidbody.AngularVelocity = (rand() + 0.1) * 6.0 - 3.0;
		
		if ((frames % 2) == 0 && target !is null)
		{
			Vector myPos = entity.itransform.Position;
			Vector targetPos = target.itransform.Position;
			Vector posDiff = targetPos - myPos;
			if (posDiff.length() < 0.6f)
				contact_frames = 10;
			else if (contact_frames > 0)
				--contact_frames;
			if (contact_frames == 0)
				entity.irigidbody.Velocity = posDiff.normalised();
			else if (contact_frames == 10)
				entity.irigidbody.Velocity = Vector();
			
			//Vector myVel = entity.irigidbody.Velocity;
			//entity.itransform.Angle = atan2(myVel.x, myVel.y);
		}
		
		float angleDiff = wrap(target_angle - entity.itransform.Angle, 0.0f, 2 * 3.14f);
		if (angleDiff >= 3.14f)
			angleDiff -= 6.28f;
		const float normAngle = angleDiff / 2 * 3.14f;
		if (angleDiff < -0.01f)
		{
			entity.irigidbody.AngularVelocity = normAngle * 2.0f;
		}
		else if (angleDiff > 0.01f)
		{
			entity.irigidbody.AngularVelocity = normAngle * 2.0f;
		}
		else
			entity.irigidbody.AngularVelocity = 0.0f;
	}
	
	float wrap(float value, float lower, float upper)
	{ 
		float distance = upper - lower; 
		float times = floor((value - lower) / distance); 
 
		return value - (times * distance);
	} 
	
	private float target_angle;
	
	//private SignalConnection@ velchange_con;
	void onVelocityChanged(const Vector &in pos)
	{
		Vector myVel = entity.irigidbody.Velocity;
		if (myVel.length() > 0.f)
		{
			target_angle = atan2(myVel.y, myVel.x);// - (3.14/2);
		}
	}
}
