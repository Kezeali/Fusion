class ConsoleElement : ScriptElement
{
	bool dirty;

	ConsoleElement(Element@ appElement)
	{
		super(appElement);

		dirty = false;
		current_text = "";
		length = 0;
	}

	~ConsoleElement()
	{
		@listenerConnection = null;
	}

	uint length;
	e_String current_text;

	void AddText(const string &in text)
	{
		dirty = true;
		if (length < 5120)
		{
			current_text += text;
			length += text.length();
		}
		else
		{
			current_text = text;
			length = 0;
		}
	}

	void Clear()
	{
		current_text = "";
		dirty = true;
	}

	FormControlTextArea@ text_area;

	void OnRender()
	{
		if (dirty)
		{
			text_area.SetValue( current_text );
			dirty = false;
		}
	}
}

ConsoleConnection@ listenerConnection;

class ConsoleGui : IConsoleListener
{
	ConsoleElement@ consoleElm;

	ConsoleGui(Element@ element)
	{
		@consoleElm = cast<ConsoleElement>(unwrap(element));
		if (consoleElm !is null)
		{
			Element@ text_element = element.GetElementById(e_String("text_element"));
			@consoleElm.text_area = cast<FormControlTextArea>(text_element);
			if (consoleElm.text_area is null)
				console.println("Couldn't find textarea element");
		}
		else
			console.println("Couldn't find console element"); 
	}

	~ConsoleGui()
	{
		console.println("ConsoleGUI Deleted"); 
		@consoleElm = null;
		if (listenerConnection !is null)
			listenerConnection.disconnect();
	}

	void OnNewData(const string&in data)
	{
		if (consoleElm !is null)
			consoleElm.AddText(data);
	}

	void OnClear()
	{
		if (consoleElm !is null)
			consoleElm.Clear();
	}
}

void OnConsoleEnterClick(Event& ev)
{
	Document@ doc = gui.getContext().GetDocument(e_String("console_doc"));
	FormControlInput@ input = cast<FormControlInput>( doc.GetElementById(e_String("textbox_element")) );
	string command = input.GetValue();
	console.interpret(command);
	input.SetValue(e_String(""));
}

string CC_test()
{
	console.println("test: "/* + args[0]*/);
	return "hi";
}

class ship
{
	//Image imgBody;
	//Sound[] engineSounds;
	//SoundSession currentSound;

	int soundTimer;
	int nextSound;

	double mass;
	double engineForce;
	double rotVelocity;

	bool commandThrustChanged;

	Body @physBody;

	ConsoleGui@ congui;

	Document@ document;

	~ship()
	{
		console.println("ship Deleted");
		@congui = null;
		if (document !is null)
			document.Close();
	}
	void Preload()
	{
		/*XmlDocument entityDoc = OpenXml("myship.xml");

		console.println("Loading: " + entityDoc.find("/entity/name/text()"));

		string body_file = entityDoc.xpath_string("/entity/images/body/text()");
		string sound1_file = entityDoc.xpath_string("/entity/sounds/engine1/text()");
		string sound2_file = entityDoc.xpath_string("/entity/sounds/engine2/text()");

		if (!entityDoc.xpath("/entity/mass/text()", mass))
		{
			console.println("Couldn't load mass :(");
			mass = 1.0;
		}
		if (!entityDoc.xpath("/entity/engineForce/text()", engineForce))
		{
			console.println("Couldnt load speed :(");
			engineForce = 0.1;
		}
		if (!entityDoc.xpath("/entity/rotVelocity/text()", rotVelocity))
		{
			console.println("Couldnt load rotSpeed :(");
			rotVelocity = 0.05;
		}
		float radius;
		if (!entityDoc.xpath("/entity/radius/text()", radius))
		{
			console.println("Couldnt load radius :(");
			radius = 32.0;
		}*/

		// Display the loaded properties
		console.println("Mass: " + mass);
		console.println("Engine Force: " + engineForce);
		console.println("Rotation speed: " + rotVelocity);
		//console.println("Radius: " + radius);

		//if (!body_file.empty())
		//	imgBody = file.GetImage(body_file);
		//else
		//	console.println("Preload failed: image tag incomplete");

		//if (!sound1_file.empty() && !sound2_file.empty())
		//{
		//	engineSounds.resize(2);
		//	engineSounds[0] = file.GetSound(sound1_file);
		//	engineSounds[1] = file.GetSound(sound2_file);
		//}
		//else
		//	console.println("Preload failed: one or more sound tags incomplete");

		soundTimer = 0;
		nextSound = 1;

		runningtime = 0;

		// Physical properties
		@physBody = @Body();
		world.add_body(physBody);

		physBody.set_mass(10);
		physBody.set_position(50, 120);
		physBody.create_circle_fixture(Vector(0,0), 16);

		//CircleShape @shape = @CircleShape(physBody, 0.0f, radius);
		//physBody.attach_shape(shape);

		console.println("Press [Debug] to print info");

		gui.enableDebugger();
		scriptManager.disableDebugOutput();

		RegisterElementType(e_String("console"), e_String("ConsoleElement"));

		Context@ context = gui.getContext();//GetContext(e_String("default"));
		@document = context.LoadDocument(e_String("gui/console.rml"));
		if (document !is null)
		{
			document.SetId(e_String("console_doc"));

			document.Show();

			Element@ consoleElement = document.GetElementById(e_String("console_element"));
			@congui = @ConsoleGui(consoleElement);
			@listenerConnection = console.connectListener(congui);

			Element@ enterElement = document.GetElementById(e_String("enter_command"));
			enterElement.AddEventListener(e_String("click"), e_String("void OnConsoleEnterClick(Event& ev)"));

			console.bindCommand("test", "string CC_test()");
		}
		context.LoadMouseCursor(e_String("gui/cursor.rml"));
	}
	void DebugOutput()
	{
		Vector p = physBody.get_position();
		Vector v = physBody.get_velocity();
		console.println("p: " + p.x + ", " + p.y);
		console.println("v: " + v.x + ", " + v.y);
		console.println("r: " + physBody.get_angle());
		//console.println("w: " + physBody.get_rotational_velocity());
	}
	void Draw()
	{
		//Vector p; physBody.get_position(p);
		//imgBody.draw(p.x, p.y, physBody.get_angle());
	}
	uint runningtime;
	void Simulate(uint dt)
	{
		if (runningtime < 2000)
		{
			runningtime+=dt;

			physBody.apply_force(Vector(10,0));
			physBody.apply_torque(1);
		}

		//if (input.is_active(player, "thrust"))
		//{
			//physBody.apply_thrust(engineForce);

			//// Cycle through sounds at an interval
			//soundTimer -= dt;
			//if (soundTimer <= 0)
			//{
			//	soundTimer = 2000;
			//	currentSound.stop();
			//	currentSound = engineSounds[nextSound].prepare(true);
			//	currentSound.play();
			//	nextSound = (nextSound + 1) % engineSounds.length();
			//}
			//// If thrust has been re-activated before soundTimer = 0, this condition will be true
			//else if (!currentSound.is_playing())
			//	currentSound.play();
		//}
		//else if (commandThrustChanged)
		//{
		//	//currentSound.stop();
		//}

		//commandThrustChanged = false;
	}
};
