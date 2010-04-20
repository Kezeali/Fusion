class TestStartup : ScriptEntity
{
	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	uint playerIndex;

	Test@ testEntity;
	TestScenery@[] scenery;
	
	bool firstUpdate;
	uint updateNum;

	TestStartup()
	{
		firstUpdate = true;
		updateNum = 0;
		//BindSimpleCommands();

		gui.getContext().SetMouseCursor(e_String("Arrow"));
		console.println("'TestStartup' entity created");
	}
	~TestStartup()
	{
		console.println("'TestStartup' entity deleted");
	}

	void OnAddPlayer(uint16 localPlayer, uint16 player)
	{
		//@testEntity = cast<Test>( entity_manager.instance("Test", "test", player) );
		testEntity.Spawn();
		testEntity.SetPosition(GetPosition());

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

		//system.addViewport(p1Viewport);

		//system.enablePhysicsDebugDraw(p1Viewport);

		@p1Viewport = null;

		//scenery.resize(2);
		//@scenery[0] = cast<TestScenery>( entity_manager.instance("TestScenery", "scenery") );
		//scenery[0].Spawn();
		//scenery[0].SetPosition(Vector(100, 20));
		//@scenery[1] = cast<TestScenery>( entity_manager.instance("TestScenery", "scenery2") );
		//scenery[1].Spawn();
		//scenery[1].SetPosition(Vector(100, 90));
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
