#uses ITransform
#uses IRigidBody

class PseudoI : ScriptComponent
{
	PseudoI()
	{
		//console.println("--TestC--");
		
		frames = 0;
		contact_frames = 0;
	}

	private uint frames;
	
	private Camera cam;
	
	Entity target;
	private EntityWrapper@ wtarget;
	
	private uint contact_frames;
	
	void update()
	{
		++frames;
		
		if (frames == 1)
		{
			cam = Camera(itransform.Position);
			streaming.addCamera(cam);
			//renderer.addViewport(cam);
			
			@setcampos_con = itransform.Position.connect("void setCameraPosition(const Vector &in)");
			
			//entity.irigidbody.AngularVelocity = 0.9f;
			entity.irigidbody.Velocity = Vector(cos(itransform.Angle.value), sin(itransform.Angle.value));
			@velchange_con = entity.irigidbody.Velocity.connect("void onVelocityChanged(const Vector &in)");
		}
		//if (frames % 2 == 0)
		//	entity.irigidbody.Velocity = Vector(cos(itransform.Angle.value), sin(itransform.Angle.value));
		//if (frames % 100 == 0)
			//entity.irigidbody.AngularVelocity = (rand() + 0.1) * 6.0 - 3.0;
		
		if (frames > 10 && wtarget is null)
			@wtarget = EntityWrapper(target);
		
		if (frames % 2 == 0 && wtarget !is null)
		{
			Vector myPos = entity.itransform.Position;
			Vector targetPos = wtarget.itransform.Position;
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
		
		float angleDiff = wrap(target_angle - entity.itransform.Angle.value, 0.0f, 2 * 3.14);
		if (angleDiff >= 3.14)
			angleDiff -= 6.28;
		const float normAngle = angleDiff / 2 * 3.14;
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
	
	private SignalConnection@ velchange_con;
	void onVelocityChanged(const Vector &in pos)
	{
		Vector myVel = entity.irigidbody.Velocity;
		if (myVel.length() > 0.f)
		{
			target_angle = atan2(myVel.y, myVel.x);// - (3.14/2);
		}
	}
	
	private SignalConnection@ setcampos_con;
	void setCameraPosition(const Vector &in pos)
	{
		cam.setPosition(pos);
	}
}
