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

	void ApplyForce(const Vector@ force, const Vector@ point)
	{
		__appObject.applyForce(force, point);
	}

	void ApplyForce(const Vector@ force)
	{
		__appObject.applyForce(force);
	}

	void ApplyTorque(float torque)
	{
		__appObject.applyTorque(torque);
	}


	Vector@ GetWorldVector(const Vector@ vector)
	{
		return __appObject.getWorldVector(vector);
	}

	Vector@ GetWorldPoint(const Vector@ point)
	{
		return __appObject.getWorldPoint(point);
	}

	Vector@ GetWorldVector(float x, float y)
	{
		return __appObject.getWorldVector( Vector(x, y) );
	}

	Vector@ GetWorldPoint(float x, float y)
	{
		return __appObject.getWorldPoint( Vector(x, y) );
	}


	void SetPosition(const Vector@ position)
	{
		__appObject.setPosition(position);
	}

	const Vector@ GetPosition() const
	{
		return __appObject.getPosition();
	}

	void SetVelocity(const Vector@ velocity)
	{
		__appObject.setVelocity(velocity);
	}

	const Vector@ GetVelocity() const
	{
		return __appObject.getVelocity();
	}

	void SetAngle(float angle)
	{
		__appObject.setAngle(angle);
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
