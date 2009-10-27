class TestScenery : ScriptEntity
{
	Renderable@ sprite;

	TestScenery()
	{
		console.println("'TestScenery' entity created");
	}
	~TestScenery()
	{
		console.println("'TestScenery' entity deleted");
	}

	void Spawn()
	{
		console.println("'TestScenery' (" + GetName() + " entity Spawned");
	}

	void Update(float dt)
	{
	}
}
