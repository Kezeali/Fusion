class TestStartup : ScriptEntity
{
	TestStartup()
	{
		//BindSimpleCommands();

		//gui.getContext().SetMouseCursor(rString("Arrow"));
	}
	~TestStartup()
	{
	}

	void OnPlayerAdded(uint localPlayer, uint8 player)
	{
	}

	void OnSpawn()
	{
		system.addPlayer();
	}

	void Update(float)
	{
	}

	void Draw()
	{}
}
