class ScriptEntity : IEntity
{
	Entity@ __appObject;
	void _setAppObject(Entity@ obj)
	{
		@__appObject = @obj;
	}
	Entity@ _getAppObject()
	{
		return __appObject;
	}

	uint16 GetOwnerID() const
	{
		return __appObject.getOwnerID();
	}

	bool InputIsActive(const string@ input)
	{
		return __appObject.inputIsActive(input);
	}
	float GetInputPosition(const string@ input)
	{
		return __appObject.getInputPosition(input);
	}

	void Spawn() {}
	void Update() {}
	void Draw() {}

	void ApplyForce(const Vector@ point, const Vector@ force)
	{
		__appObject.applyForce(point, force);
	}

	void ApplyForce(const Vector@ force)
	{
		__appObject.applyForce(force);
	}

	void ApplyTorque(float torque)
	{
		__appObject.applyTorque(torque);
	}


	void SetPosition(const Vector@ position)
	{
		__appObject.setPosition(position);
	}

	const Vector@ GetPosition() const
	{
		return __appObject.getPosition();
	}

	const Vector@ GetVelocity() const
	{
		return __appObject.getVelocity();
	}

	float GetAngle() const
	{
		return __appObject.getAngle();
	}

	void SetAngularVelocity(float ang_vel)
	{
		__appObject.setAngularVelocity(ang_vel);
	}

	float GetAngularVelocity() const
	{
		return __appObject.getAngularVelocity();
	}

}
