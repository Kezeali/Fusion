class ConsoleElement : ScriptElement
{
	bool dirty;
	bool autoScroll;
	bool slowScroll;

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
			//slowScroll = true;
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
				//text_area.SetCursorIndex(current_text.Length(), true);
				// Send ctrl-end key-press event
				e_Dictionary parameters;
				parameters.Set(e_String("ctrl_key"), int(1));
				parameters.Set(e_String("key_identifier"), int(GUIKey::KI_END));
				text_area.DispatchEvent(e_String("keydown"), parameters);
				text_area.ShowCursor(false);
				autoScroll = false;
			}
			//text_area.SetReadOnly(true);
			dirty = false;
		}

		//if (slowScroll)
		//{
		//}
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

// TODO: merge this with ConsoleElement (perhaps make ConsoleElement create an instance and hold it as a member, if they have to be seperate for some reason)
class ConsoleRouter : IConsoleListener
{
	//ConsoleElement@ consoleElm;
	AutocompleteListener autocompleteListener;

	ConsoleRouter(Element@ element)
	{
		//ConsoleElement@ consoleElm = cast<ConsoleElement>( unwrap(gui.getContext().GetDocument(e_String("console_doc")).GetElementById(e_String("console_element"))) );
		@consoleElm = cast<ConsoleElement>(unwrap(element));
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

	~ConsoleRouter()
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

EventConnection@ consoleClickConnection;
void OnConsoleEnterButtonClick(Event& ev)
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

void InitialiseConsole()
{
	RegisterElementType(e_String("console"), e_String("ConsoleElement"));
}

void OnConsoleOpened(Event @ev)
{
}

void OnConsoleClosed(Event @ev)
{
}

//void CreateConsole()
//{
//	Context@ context = gui.getContext();
//	@document = context.LoadDocument(e_String("core/gui/console.rml"));
//	if (document !is null)
//	{
//		document.SetId(e_String("console_doc"));
//
//		document.Show();
//
//		//Element@ consoleElement = document.GetElementById(e_String("console_element"));
//		@congui = ConsoleRouter(/*consoleElement*/);
//		@listenerConnection = console.connectListener(congui);
//
//		Element@ enterElement = document.GetElementById(e_String("enter_command"));
//		@consoleClickConnection = enterElement.AddEventListener(e_String("click"), e_String("void OnConsoleEnterClick(Event& ev)"));
//	}
//}
