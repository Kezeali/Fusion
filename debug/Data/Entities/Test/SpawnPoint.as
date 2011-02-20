class SpawnPoint : ScriptEntity
{
	uint playerToSpawn;

	SpawnPoint()
	{
	}
	~SpawnPoint()
	{
	}

	void OnAddPlayer(uint local_player, uint8 net_id)
	{
		if (local_player == playerToSpawn)
		{
			Entity@ playerEntity = entity_factory.instance("Test", "test_player");
			playerEntity.setOwnerID(net_id);
			entity_manager.add(playerEntity); // TODO: make EntityManager::update call Spawn the first time an entity is updated (or something)
		}
	}

	void Spawn()
	{
	}

	void Draw()
	{
	}

	void Update(float dt)
	{
	}
}
