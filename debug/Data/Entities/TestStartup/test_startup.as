class TestStartup : ScriptEntity
{
	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	uint playerIndex;

	Test@ testEntity;
	
	bool firstUpdate;

	TestStartup()
	{
		firstUpdate = true;
		//BindSimpleCommands();
	}
	~TestStartup()
	{
		console.println("'TestStartup' entity deleted");
	}

	void OnAddPlayer(uint16 localPlayer, uint16 player)
	{
		@testEntity = cast<Test>( entity_manager.instance("Test", "test", player) );
		testEntity.Spawn();

		console.println("Before camera setup");

		primaryCamera.setFollowEntity(testEntity);
		primaryCamera.setFollowMode(CamFollowMode::FollowInstant);

		streamer.setPlayerCamera(player, primaryCamera);

		@primaryCamera = null;
		@testEntity = null;

		console.println("After camera setup");
	}

	void Spawn()
	{
		@primaryCamera = @Camera();
		@p1Viewport = @Viewport(0, 0, 1, 1);
		p1Viewport.setCamera(primaryCamera);

		system.addViewport(p1Viewport);

		playerIndex = system.addPlayer(this, "OnAddPlayer");

		@p1Viewport = null;
	}

	void Update(float)
	{
		//if (firstUpdate)
		//{
		//	firstUpdate = false;
		//	entity_manager.remove(this);
		//}
	}

	void Draw()
	{}
}
