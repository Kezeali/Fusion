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
	console.println(command);
	console.interpret(command);
	input.SetValue(e_String(""));
}

int correctNumber = 0;

string CC_startGuess(const StringArray &in args)
{
	console.println("Guess a number between 1 and 100");
	correctNumber = rand() * 100 + 1;
	return "";
}

string CC_takeGuess(const StringArray &in args)
{
	if (args.size() >= 2)
	{
		int guess;
		bool validGuess = args[1].parseInt(guess);
		if (validGuess)
		{
			if (guess == correctNumber)
				return "You Win!";
			else if (guess < correctNumber)
				return "Try higher";
			else if (guess > correctNumber)
				return "Try lower";
		}
	}

	return "Guess a number";
}

void anotherFunc(Vector value)
{
	string hi = "A divide-by-zero exception should occor here";
	float error = value.length() / value.get_y();
}

bool anotherFunc(int number, int zero)
{
	Vector@ vec = @Vector(number, zero);
	anotherFunc(vec);
	console.println("Shouldn't get here");
	return true;
}

bool anotherFunc(string arg)
{
	return anotherFunc(arg.length(), 0);
}

string CC_test(const StringArray &in args)
{
	console.println("test: ");
	if (args.size() == 2)
		args[1].toInt();
	if (args.size() >= 3)
		anotherFunc(args[2]);
	return "hi";
}

string CC_echo(const StringArray &in args)
{
	if (args.size() >= 2)
		return args[1];

	return "";
}

string CC_longName(const StringArray &in args)
{
	console.println("long name: " + args[0]);
	return "";
}

// Binds a command with callback and Help text (H)
void CCBind_H(const string &in command, const string &in callback, const string &in help_text)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, help_text);
}

// Binds a command with callback and Argument names (Args)
void CCBind_Args(const string &in command, const string &in callback, const string &in argument_names)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, "", argument_names);
}

// Binds a command with callback, Help text and Argument names (HA)
void CCBind_HA(const string &in command, const string &in callback, const string &in help_text, const string &in argument_names)
{
	console.bindCommand(command, callback);
	console.setCommandHelpText(command, help_text, argument_names);
}

// Binds a command with callback, Auto-correct callback, Help text and Argument names (AHA)
void CCBind_AHA(const string &in command, const string &in callback, const string &in autocorrect, const string &in help_text, const string &in argument_names)
{
	console.bindCommand(command, callback, autocorrect);
	console.setCommandHelpText(command, help_text, argument_names);
}

// Binds a command with callback, Auto-correct callback and Argument names (AA)
void CCBind_AA(const string &in command, const string &in callback, const string &in autocorrect, const string &in argument_names)
{
	console.bindCommand(command, callback, autocorrect);
	console.setCommandHelpText(command, "", argument_names);
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
			@enterElement = document.GetElementById(e_String("textbox_element"));
			enterElement.AddEventListener(e_String("submit"), e_String("void OnConsoleEnterClick(Event& ev)"));

			console.bindCommand("test", "string CC_test(const StringArray &in args)");
			console.bindCommand("my_cool_commandle", "string CC_longName(const StringArray &in args)");
			console.bindCommand("my_cool_commandlay", "string CC_longName(const StringArray &in args)");
			console.bindCommand("doolp_compila_fizzle_topla_bick", "string CC_longName(const StringArray &in args)");
			console.bindCommand("doSomethingPlz", "string CC_longName(const StringArray &in args)");

			CCBind_HA("echo", "CC_echo",
				"Writes whatever you type after it to the console", "[text] [...]");

			CCBind_H("play", "string CC_startGuess(const StringArray &in args)", "Play a guessing game");
			CCBind_HA("guess", "string CC_takeGuess(const StringArray &in args)",
				"Take a guess at the guessing game. Enter the command 'play' first.", "number");
			//console.bindCommand("play", "string CC_startGuess(const StringArray &in args)");
			//console.bindCommand("guess", "string CC_takeGuess(const StringArray &in args)");
		}
		context.LoadMouseCursor(e_String("gui/cursor.rml"));

		first = true;
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
	bool first;
	void Simulate(uint dt)
	{
		if(first)
		{
			seed_rand(dt);
			first = false;
		}
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
