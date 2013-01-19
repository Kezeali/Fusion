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

service Editor {
	list<EntityData> getSelectedEntities();
	oneway void selectEntity(1: i32 id);
	oneway void stop();
}