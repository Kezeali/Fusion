class ConsoleElement : ScriptElement
{
	bool dirty;
	bool autoScroll;

	ConsoleElement(Element@ appElement)
	{
		super(appElement);

		dirty = false;
		current_text = "";
		length = 0;
	}

	~ConsoleElement()
	{
		console.println("ConsoleElement Deleted");
		@listenerConnection = null;
		@__inner = null;
	}

	uint length;
	e_String current_text;

	void AddText(const string &in text)
	{
		if (text_area.GetCursorIndex() >= current_text.Length()-1)
		{
			autoScroll = true;
		}

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

		dirty = true;
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
			//text_area.SetReadOnly(false);
			text_area.SetValue( current_text );
			if (autoScroll)
			{
				//text_area.SetCursorIndex(current_text.Length());
				e_Dictionary parameters;
				parameters.Set(e_String("key_identifier"), int(GUIKey::KI_END));
				parameters.Set(e_String("ctrl_key"), int(1));
				text_area.DispatchEvent(e_String("keydown"), parameters);
				autoScroll = false;
			}
			//text_area.SetReadOnly(true);
			dirty = false;
		}
	}
}

ConsoleConnection@ listenerConnection;
ContextMenu@ autocomplete_menu;

class AutocompleteListener : IContextMenuListener
{
	void OnContextMenuClick(const MenuItem&in item)
	{
		Document@ doc = gui.getContext().GetDocument(e_String("console_doc"));
		FormControlInput@ input = cast<FormControlInput>( doc.GetElementById(e_String("command_element")) );
		input.SetValue( e_String(item.name) );
		input.Focus();

		e_Dictionary parameters;
		parameters.Set(e_String("key_identifier"), int(GUIKey::KI_END));
		input.DispatchEvent(e_String("keydown"), parameters);
	}
}

class ConsoleGui : IConsoleListener
{
	//ConsoleElement@ consoleElm;
	AutocompleteListener autocompleteListener;

	ConsoleGui(/*Element@ element*/)
	{
		ConsoleElement@ consoleElm = cast<ConsoleElement>( unwrap(gui.getContext().GetDocument(e_String("console_doc")).GetElementById(e_String("console_element"))) );
		//@consoleElm = cast<ConsoleElement>(unwrap(element));
		if (consoleElm !is null)
		{
			Element@ text_element = consoleElm.GetElementById(e_String("text_element"));
			@consoleElm.text_area = cast<FormControlTextArea>(text_element);
			if (consoleElm.text_area is null)
				console.println("Couldn't find textarea element");

			Element@ entry_element = consoleElm.GetElementById(e_String("command_element"));
			@autocomplete_menu = @ContextMenu( e_String("autocomplete_menu"), e_Vector2i(entry_element.GetAbsoluteLeft(), entry_element.GetAbsoluteTop()) );
			autocomplete_menu.SetListener(@autocompleteListener);
		}
		else
			console.println("Couldn't find console element"); 
	}

	~ConsoleGui()
	{
		console.println("ConsoleGUI Deleted");
		//consoleElm._SetAppObject(null);
		//@consoleElm = null;

		if (listenerConnection !is null)
			listenerConnection.disconnect();

		console.println("autocomplete_menu SetListener(null)"); 
		autocomplete_menu.SetListener(null);
		console.println("autocomplete_menu Clear()"); 
		autocomplete_menu.Clear();
		console.println("autocomplete_menu Close()"); 
		autocomplete_menu.m_MenuDoc.Close();
		console.println("autocomplete_menu MenuDoc(null)"); 
		@autocomplete_menu.m_MenuDoc = null;

		@autocomplete_menu = null;
	}

	void OnNewData(const string&in data)
	{
		ConsoleElement@ consoleElm = cast<ConsoleElement>( unwrap(gui.getContext().GetDocument(e_String("console_doc")).GetElementById(e_String("console_element"))) );
		if (consoleElm !is null)
			consoleElm.AddText(data);
	}

	void OnClear()
	{
		ConsoleElement@ consoleElm = cast<ConsoleElement>( unwrap(gui.getContext().GetDocument(e_String("console_doc")).GetElementById(e_String("console_element"))) );
		if (consoleElm !is null)
			consoleElm.Clear();
	}
}

//class ConsoleAutocomplete
//{
//	void OnEntryType(Event &ev)
//	{
//		console.listPrefixedCommands(ev.GetParameter_char("data"));
//	}
//}
e_String lastvalue = "";
bool commandDone = false;
uint commandLength = 0;
StringArray possibleCommands;
void OnConsoleEntryChanged(Event& ev)
{
	e_String value("");
	value = ev.GetParameter(e_String("value"), value);

	if (!commandDone && value != lastvalue)
	{
		lastvalue = value;
		if (!value.Empty())
		{
			if (value[value.Length()-1] == e_String(" "))
			{
				commandDone = true;
				commandLength = value.Length()-1;
				autocomplete_menu.Hide();
				return;
			}

			if (autocomplete_menu !is null)
			{
				console.listPrefixedCommands(string(value), possibleCommands);
				autocomplete_menu.Clear();
				for (uint i = 0; i < possibleCommands.size(); i++)
					autocomplete_menu.AddItem(possibleCommands[i]);

				if (possibleCommands.size() > 0)
				{
					Element@ target = ev.GetTargetElement();
					autocomplete_menu.SetPosition(target.GetAbsoluteLeft(), target.GetAbsoluteTop() + target.GetClientHeight());
					autocomplete_menu.Show();
				}
			}
		}
		else // 'value' is empty
		{
			autocomplete_menu.Hide();
			autocomplete_menu.Clear();
		}
	}
	else if (value.Length() <= commandLength)
	{
		commandDone = false;
		commandLength = 0;
		autocomplete_menu.Show();
	}
}

void OnConsoleEntryEnter(Event& ev)
{
	if (autocomplete_menu.GetCurrentSelection() != -1)
		autocomplete_menu.ClickSelected();
	else
		OnConsoleEnterClick(ev);
	autocomplete_menu.Hide();
	autocomplete_menu.Clear();
}

void OnConsoleEntryKeyUp(Event& ev)
{
	if (ev.GetType() == e_String("keyup"))
	{
		int key_identifier = ev.GetParameter(e_String("key_identifier"), int(0));
		if (key_identifier == GUIKey::KI_UP)
		{
			autocomplete_menu.SelectRelative(-1);
		}
		if (key_identifier == GUIKey::KI_DOWN)
		{
			autocomplete_menu.SelectRelative(1);
		}
	}
	else if (ev.GetType() == e_String("keydown"))
	{
		int key_identifier = ev.GetParameter(e_String("key_identifier"), int(0));
		if (key_identifier == GUIKey::KI_UP || key_identifier == GUIKey::KI_DOWN)
		{
			ev.StopPropagation();
		}
	}
}

void OnAutocompleteChanged(Event& ev)
{
	Document@ doc = gui.getContext().GetDocument(e_String("console_doc"));
	FormControlInput@ input = cast<FormControlInput>( doc.GetElementById(e_String("command_element")) );
	input.SetValue( ev.GetParameter(e_String("value"), input.GetValue()) );
}

interface IContextMenuListener
{
	void OnContextMenuClick(const MenuItem&in);
}

class MenuItem
{
	uint index;
	string name;
	Element@ element;

	MenuItem()
	{
		index = 0;
		name = "";
		@element = null;
	}

	MenuItem(uint i, const string&in n, Element@ e)
	{
		index = i;
		name = n;
		@element = @e;
	}

	~MenuItem()
	{
		console.println("MenuItem " + name + " deleted");
		@element = null;
	}
}

class ContextMenu/* : IEventListener*/
{
	//IEventListener@ clickListener;
	IContextMenuListener@ m_Listener;

	MenuItem[] m_Items;
	uint m_ItemCount;

	int m_CurrentSelection;

	Document@ m_MenuDoc;
	e_String m_Id;
	e_Vector2i m_Position;

	ContextMenu(const e_String&in id, const e_Vector2i&in position)
	{
		m_Id = id;
		m_Position = position;

		@m_MenuDoc = gui.getContext().LoadDocument(e_String("gui/context_menu.rml"));
		if (m_MenuDoc !is null)
		{
			m_MenuDoc.SetId(id);
			m_MenuDoc.SetProperty(e_String("left"), e_String(position.x + "px"));
			m_MenuDoc.SetProperty(e_String("top"), e_String(position.y + "px"));

			m_Items.resize(5);
			m_ItemCount = 0;
			m_CurrentSelection = -1; // nothing selected
		}
	}

	~ContextMenu()
	{
		Clear();
		if (m_MenuDoc !is null) 
			m_MenuDoc.Close();
		@m_MenuDoc = null;
		m_Items.resize(0);
	}

	bool Show()
	{
		if (m_MenuDoc is null)
			return false;

		m_MenuDoc.Show(DocumentFocusFlags::NONE);
		m_MenuDoc.PullToFront();
		return true;
	}

	void Hide()
	{
		if (m_MenuDoc !is null)
			m_MenuDoc.Hide();
	}

	void SetPosition(int x, int y)
	{
		m_Position.x = x;
		m_Position.y = y;
		m_MenuDoc.SetProperty(e_String("left"), e_String(x + "px"));
		m_MenuDoc.SetProperty(e_String("top"), e_String(y + "px"));
	}

	void SetListener(IContextMenuListener@ listener)
	{
		@m_Listener = @listener;
	}

	bool AddItem(const string&in name/*const e_String&in name*/)
	{
		// Add the GUI element to the context menu document
		Element@ element = @m_MenuDoc.CreateElement(e_String("button"));
		element.SetProperty(e_String("display"), e_String("block"));
		element.SetProperty(e_String("clip"), e_String("auto"));
		element.SetAttribute(e_String("menu_index"), m_ItemCount);
		element.SetId(e_String("contextmenuitem-"+name+m_ItemCount));
		element.SetInnerRML(e_String(name));
		m_MenuDoc.AppendChild(element);

		element.AddEventListener(e_String("click"), e_String("void OnContextMenuInput(Event@ ev)"));

		// Add the MenuItem object
		if (m_ItemCount == m_Items.length())
			m_Items.resize(m_ItemCount+5);
		m_Items[m_ItemCount] = MenuItem(m_ItemCount, name, element);
		m_ItemCount++;

		return true;
	}

	void RemoveItem(uint index)
	{
		m_MenuDoc.RemoveChild(m_Items[index].element);
		@m_Items[index].element = null;

		if (m_CurrentSelection >= 0 && index == int(m_CurrentSelection))
			m_CurrentSelection = -1;

		m_ItemCount--;
		for (uint i = index; i < m_ItemCount; i++)
		{
			m_Items[i] = m_Items[i+1];
		}
	}

	void Clear()
	{
		for (uint i = 0; i < m_ItemCount; i++)
		{
			if (!m_MenuDoc.RemoveChild(m_Items[i].element))
				console.println("Error: Failed to remove MenuItem element " + m_Items[i].name);
			@m_Items[i].element = null;
		}
		//m_Items.resize(0);
		m_ItemCount = 0;
		m_CurrentSelection = -1;
	}

	void Select(int index)
	{
		if (index >= 0 && index < m_ItemCount)
		{
			m_CurrentSelection = index;
			// false: don't scroll to top if if not neccessary to get the element into view
			m_Items[index].element.ScrollIntoView(false);
			gui.setMouseCursorPosition(m_Items[index].element.GetAbsoluteLeft()+1, m_Items[index].element.GetAbsoluteTop()+1);
		}
	}

	void SelectRelative(int distance)
	{
		int index = m_CurrentSelection + distance;
		if (index < 0)
			index = 0;
		if (index >= m_ItemCount)
			index = m_ItemCount-1;

		Select(index);
	}

	int GetCurrentSelection()
	{
		return m_CurrentSelection;
	}

	void ClickItem(int index)
	{
		if (index >= 0 && index < m_ItemCount)
		{
			m_Items[index].element.DispatchEvent(e_String("click"));
		}
	}

	void ClickSelected()
	{
		ClickItem(m_CurrentSelection);
	}

	void OnAttach(Element@) {}
	void OnDetach(Element@) {}

	//void ProcessEvent(Event@ ev)
	//{
	//	if (ev.GetType() == e_String("click"))
	//	{
	//		//console.println("item clicked...");
	//		Element@ clicked = ev.GetTargetElement();
	//		uint index = clicked.GetAttribute(e_String("menu_index"), m_ItemCount);
	//		if (index < m_ItemCount)
	//			m_Listener.OnContextMenuClick(m_Items[index]);
	//	}
	//}
}

void OnContextMenuInput(Event@ ev)
{
	if (ev.GetType() == e_String("click"))
	{
		//console.println("item clicked...");
		Element@ clicked = ev.GetTargetElement();
		uint index = clicked.GetAttribute(e_String("menu_index"), autocomplete_menu.m_ItemCount);
		if (index < autocomplete_menu.m_ItemCount)
			autocomplete_menu.m_Listener.OnContextMenuClick(autocomplete_menu.m_Items[index]);

		autocomplete_menu.Hide();
	}
}

class AutocompleteDS : IDataSource
{
	AutocompleteDS()
	{
	}

	void GetRow(StringList&out row, const e_String&in table, int row_index, const StringList&in columns)
	{
		if (row_index >= 0 && uint(row_index) >= possibleCommands.size())
			return;

		if (table == e_String("commands"))
		{
			for (uint i = 0; i < columns.size(); i++)
			{
				if (columns[i] == e_String("name"))
				{
					row.push_back( e_String(possibleCommands[row_index]) );
				}
			}
		}
	}

	int GetNumRows(const e_String&in table)
	{
		if (table == e_String("commands"))
		{
			return possibleCommands.size();
		}

		return 0;
	}
}

void OnConsoleEnterClick(Event& ev)
{
	Document@ doc = gui.getContext().GetDocument(e_String("console_doc"));
	if (doc is null)
		return;
	FormControlInput@ input = cast<FormControlInput>( doc.GetElementById(e_String("command_element")) );

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
		DeleteScriptElements();
		if (document !is null)
		{
			console.println("~ship: closing console document");
			//document.Close();
			@document = null;
			console.println("~ship: console document de-referenced");
		}
	}

	void DeleteScriptElements()
	{
		@congui = null;
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

			//Element@ consoleElement = document.GetElementById(e_String("console_element"));
			@congui = ConsoleGui(/*consoleElement*/);
			@listenerConnection = console.connectListener(congui);

			Element@ enterElement = document.GetElementById(e_String("enter_command"));
			enterElement.AddEventListener(e_String("click"), e_String("void OnConsoleEnterClick(Event& ev)"));
			//@enterElement = document.GetElementById(e_String("textbox_element"));
			//enterElement.AddEventListener(e_String("change"), e_String("void OnConsoleEntryChanged(Event& ev)"));

			AddDataSource(e_String("autocomplete"), AutocompleteDS());

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
