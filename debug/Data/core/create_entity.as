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
	}
	if (type == 2)
	{
		Entity e = editor.CreateEntity("b2Dynamic", pos, angle, true, true);
	}
	if (type == 3)
	{
		Entity e = editor.CreateEntity("b2Static", pos, angle, false, true);
	}
	if (type == 4)
	{
		Entity e = editor.CreateEntity("StaticTransform", pos, angle, true, false);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		instantiator.addComponent(e, "SpawnPoint", "");
		
		sprite.ImagePath << "/Data/Entities/Test/Gfx/spaceshoot_body_moving1.png";
	}
	if (type == 5)
	{
		Entity e = editor.CreateEntity("b2Static", pos, angle, false, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		EntityComponent@ b2Circle = instantiator.addComponent(e, "b2Circle", "");
		IFixture@ fixture = cast<IFixture>(b2Circle);
		ICircleShape@ shape = cast<ICircleShape>(b2Circle);
		
		sprite.ImagePath << "/Data/Entities/grass_shadowed.png";
		shape.Radius << 0.20f;
		fixture.Sensor << true;
	}
	if (type == 6)
	{
		Entity e = editor.CreateEntity("b2Dynamic", pos, angle, true, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "body_sprite"));
		ISprite@ shadow = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "shadow_sprite"));
		ICircleShape@ shape = cast<ICircleShape>(instantiator.addComponent(e, "b2Circle", ""));
		
		sprite.ImagePath << "/Data/Entities/character/walk_cycle.png";
		sprite.AnimationPath << "/Data/Entities/character/walk_cycle2.yaml:left";
		sprite.Offset << Vector(0.f, -20.f);
		
		shadow.ImagePath << "/Data/Entities/character/shadow.png";
		shadow.Offset << Vector(0.f, 11.f);
		shadow.Alpha << 0.75f;
		shadow.LocalDepth << -1;
		
		shape.Radius << 0.176f;
	}
	if (type == 7)
	{
		Entity e = editor.CreateEntity("b2Dynamic", pos, angle, true, true);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "body_sprite"));
		ISprite@ hair = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "hair_sprite"));
		ISprite@ shirt = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "shirt_sprite"));
		ISprite@ shadow = cast<ISprite>(instantiator.addComponent(e, "CLSprite", "shadow_sprite"));
		ICircleShape@ shape = cast<ICircleShape>(instantiator.addComponent(e, "b2Circle", ""));
		
		sprite.ImagePath << "/Data/Entities/char1/char1.png";
		sprite.AnimationPath << "/Data/Entities/char1/walk_cycle.yaml";
		sprite.Offset << Vector(0.f, -10.f);
		hair.ImagePath << "/Data/Entities/char1/char1_hair.png";
		hair.AnimationPath << "/Data/Entities/char1/walk_cycle.yaml";
		hair.Offset << Vector(0.f, -10.f);
		hair.LocalDepth << 2;
		shirt.ImagePath << "/Data/Entities/char1/char1_shirt.png";
		shirt.AnimationPath << "/Data/Entities/char1/walk_cycle.yaml";
		shirt.Offset << Vector(0.f, -10.f);
		shirt.LocalDepth << 1;
		
		sprite.Scale << Vector(2, 2);
		hair.Scale << Vector(2, 2);
		shirt.Scale << Vector(2, 2);
		
		shadow.ImagePath << "/Data/Entities/character/shadow.png";
		shadow.Offset << Vector(0.f, 6.f);
		shadow.Alpha << 0.75f;
		shadow.LocalDepth << -1;
		
		shape.Radius << 0.176f;
	}
	if (type == 8)
	{
		Entity e = editor.CreateEntity("StaticTransform", pos, angle, true, false);
		ISprite@ sprite = cast<ISprite>(instantiator.addComponent(e, "CLSprite", ""));
		instantiator.addComponent(e, "TestB", "");
		
		sprite.ImagePath << "/Data/Entities/Test/Gfx/spaceshoot_body_moving1.png";
	}
}