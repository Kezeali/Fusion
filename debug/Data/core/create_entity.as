#uses ITransform
#uses IRigidBody
#uses ISprite
#uses SpawnPoint

void createEntity(int type, const Vector &in pos, float angle)
{
	console.println("Type: " + type);
	console.println("Pos: " + pos.x + "," + pos.y);
	console.println("Angle: " + angle);
	if (type == 1)
	{
		Entity e = editor.CreateEntity("StaticTransform", pos, angle, false, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		sprite.ImagePath << "/Entities/Dirt.png";
	}
	if (type == 2)
	{
		EntityRapper@ e = EntityRapper(editor.CreateEntity("StaticTransform", pos, angle, false, true));
		e.addComponent("CLSprite", "");
		e.addComponent("SpawnPoint", "");
		e.isprite.ImagePath << "Entities/Test/Gfx/spaceshoot_body_moving1.png";
	}
	//if (type == 3)
	//{
	//	EntityRapper@ e = EntityRapper(editor.CreateEntity("b2RigidBody", pos, angle, false, true));
	//	e.addComponent("CLSprite", "");
	//	e.addComponent("ICircleShape", "");
	//	e.isprite.ImagePath << "Entities/Test/Gfx/spaceshoot_body_moving1.png";
	//}
}