class TestStartup : ScriptEntity
{
	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	uint playerIndex;

	Test@ testEntity;

	TestStartup()
	{
	}

	void OnAddPlayer(uint16 player)
	{
		@testEntity = cast<Test>( entity_manager.instance("Test", "test", 1) );

		primaryCamera.setFollowEntity(testEntity);
		primaryCamera.setFollowMode(CamFollowMode::FollowInstant);
	}

	void Spawn()
	{
		//playerIndex = system.addPlayer("void OnAddPlayer(uint16 player)");

		@primaryCamera = @Camera();
		@p1Viewport = @Viewport(0, 0, 1, 1);
		p1Viewport.setCamera(primaryCamera);

		system.addViewport(p1Viewport);
	}

	void Update()
	{}

	void Draw()
	{}
}
