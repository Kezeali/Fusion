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
		return 0;//return __appObject.getOwnerID();
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
}

void OnAddPlayer(uint16 player)
{
}

class TestStartup : ScriptEntity
{
	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	uint playerIndex;

	Test@ testEntity;

	TestStartup()
	{
	}

	void Spawn()
	{
		//playerIndex = system.addPlayer("void OnAddPlayer(uint16 player)");

		@testEntity = cast<Test>( entity_manager.instance("Test", "test", 1) );

		@primaryCamera = @Camera();
		primaryCamera.setFollowEntity(testEntity);
		primaryCamera.setFollowMode(CamFollowMode::FollowInstant);
		@p1Viewport = @Viewport(0, 0, 1, 1);
		p1Viewport.setCamera(primaryCamera);

		system.addViewport(p1Viewport);
	}

	void Update()
	{}

	void Draw()
	{}
}
