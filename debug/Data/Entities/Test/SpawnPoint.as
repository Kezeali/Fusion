class SpawnPoint : ScriptEntity
{
	uint playerToSpawn;

	Camera@ primaryCamera;
	Viewport@ p1Viewport;

	SpawnPoint()
	{
		playerToSpawn = 0;
	}
	~SpawnPoint()
	{
	}

	void OnPlayerAdded(uint local_player, uint8 net_id)
	{
		if (local_player == playerToSpawn)
		{
			system.requestInstance(true, "Test", "test_player", net_id);
		}
	}

	void OnInstanceRequestFulfilled(Entity@ player)
	{
		@primaryCamera = @Camera();
		@p1Viewport = @Viewport(0, 0, 1, 1);
		p1Viewport.setCamera(primaryCamera);

		system.addViewport(p1Viewport);

		primaryCamera.setFollowEntity(unwrap_Test(player));
		primaryCamera.setFollowMode(CamFollowMode::FollowInstant);

		streamer.addCamera(primaryCamera);

		@primaryCamera = null;
	}

	void OnSpawn()
	{
	}

	void Draw()
	{
	}

	void Update(float dt)
	{
	}
}
