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
	1: string path,
	2: string type,
	3: bool directory
}

struct ConsoleCommandHelpData {
	1: string helpText;
	2: optional list<string> argumentNames;
}

enum DragDropAction {
	Copy = 1,
	Move = 2
}

struct DragDropData {
	1: string path;
	4: DragDropAction attemptedAction;
}

enum DialogType {
	Open = 1,
	Save = 2
}

struct DialogRequest {
	1: DialogType type;
	2: i32 id;
	3: optional string title;
	4: optional string path;
}

service Editor {
	void SaveMap(1: string name);
	void LoadMap(1: string name);
	void CompileMap(1: string name);
	DialogRequest PopDialogRequest();
	void CompleteDialogRequest(1: DialogRequest request, 2: bool success);
	string GetUserDataDirectory();
	string GetDataDirectory();
	list<ResourceFile> GetResources(1: string path);
	list<ResourceFile> GetResourcesRecursive(1: string path);
	string GetResourceType(1: string path);
	oneway void RefreshResources();
	oneway void CopyResource(1: string source, 2: string destination);
	oneway void MoveResource(1: string source, 2: string destination);
	oneway void DeleteResource(1: string path);
	DragDropAction SetDragDropData(1: DragDropData data);
	bool DragDrop(1: DragDropData data);
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