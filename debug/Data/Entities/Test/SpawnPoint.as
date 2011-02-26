class SpawnPoint : ScriptEntity
{
	uint playerToSpawn;

	SpawnPoint()
	{
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
