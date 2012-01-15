#uses ITransform
#uses IRigidBody
#uses ISprite
#uses SpawnPoint

void createEntity(int type, const Vector &in pos, float angle)
{
	console.println("Creating Entity - Type: " + type + "; Pos: " + pos.x + "," + pos.y + "; Angle: " + angle);
	if (type == 1)
	{
		Entity e = editor.CreateEntity("StaticTransform", pos, angle, false, true);
		ITransform@ transform = cast<ITransform>(e.getComponent("StaticTransform", "").get());
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		
		transform.Depth = -1;
		sprite.ImagePath << "/Entities/Dirt.png";
	}
	if (type == 2)
	{
		Entity e = editor.CreateEntity("StaticTransform", pos, angle, true, false);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		instantiator.addComponent(e, "SpawnPoint", "");
		
		sprite.ImagePath << "/Entities/Test/Gfx/spaceshoot_body_moving1.png";
	}
	if (type == 3)
	{
		Entity e = editor.CreateEntity("b2RigidBody", pos, angle, true, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		ICircleShape@ shape = cast<ICircleShape>(instantiator.addComponent(e, "b2Circle", ""));
		
		sprite.ImagePath << "/Entities/Test/Gfx/spaceshoot_body_moving1.png";
		shape.Radius << 0.25f;
	}
	if (type == 4)
	{
		Entity e = editor.CreateEntity("b2Static", pos, angle, false, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		IComponent@ b2Circle = instantiator.addComponent(e, "b2Circle", "");
		IFixture@ fixture = cast<IFixture>(b2Circle);
		ICircleShape@ shape = cast<ICircleShape>(b2Circle);
		
		sprite.ImagePath << "/Entities/shrub_shadowed.png";
		shape.Radius << 0.18f;
		fixture.Sensor << true;
	}
	if (type == 5)
	{
		Entity e = editor.CreateEntity("b2Static", pos, angle, false, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		IComponent@ b2Circle = instantiator.addComponent(e, "b2Circle", "");
		IFixture@ fixture = cast<IFixture>(b2Circle);
		ICircleShape@ shape = cast<ICircleShape>(b2Circle);
		
		sprite.ImagePath << "/Entities/grass_shadowed.png";
		shape.Radius << 0.20f;
		fixture.Sensor << true;
	}
}