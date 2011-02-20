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

	void OnAddPlayer(uint localPlayer, uint8 player)
	{
	}

	void Spawn()
	{
		system.addPlayer();
	}

	void Update(float)
	{
	}

	void Draw()
	{}
}
