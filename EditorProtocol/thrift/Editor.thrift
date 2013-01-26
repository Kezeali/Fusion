namespace * FusionEngine.Interprocess

struct EntityComponentData {
	2: string type,
	4: string name,
	9: binary state
}

struct EntityData {
	1: i32 id,
	2: byte owner,
	4: string name,
	9: list<EntityComponentData> components
}

struct ResourceFile {
	1: string filename,
	2: string type,
	3: bool directory
}

struct ConsoleCommandHelpData {
	1: string helpText;
	2: optional list<string> argumentNames;
}

service Editor {
	string GetUserDataDirectory();
	string GetDataDirectory();
	list<ResourceFile> GetResources(1: string path);
	list<ResourceFile> GetResourcesRecursive(1: string path);
	string GetResourceType(1: string path);
	oneway void MoveResource(1: string source, 2: string destination);
	oneway void InterpretConsoleCommand(1: string command);
	list<string> FindConsoleCommandSuggestions(1: string command);
	string CompleteCommand(1: string command, 2: string completion);
	ConsoleCommandHelpData GetConsoleCommandHelp(1: string command);
	list<EntityData> GetSelectedEntities();
	oneway void SelectEntity(1: i32 id);
	oneway void FocusOnEntity(1: i32 id);
	bool CreateEntity(1: string transformType, 4: bool synced, 5: bool streamed);
	oneway void Stop();
}