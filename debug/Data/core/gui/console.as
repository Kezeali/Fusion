//#include "gui_base.as"

void InitialiseConsole()
{
	Rocket::RegisterElementType(Rocket::String("console"), Rocket::String("ConsoleElement"));
}

ContextMenu@ autocomplete_menu;
bool firstArg = true;
uint firstArgLength = 0;

class ConsoleLine
{
	uint first;
	uint last;
};

class ConsoleElement : ScriptElement
{
	bool dirty;
	bool autoScroll;
	bool slowScroll;

	Rocket::ElementFormControlTextArea@ text_area;
	SignalConnection@ onDataConnection;
	SignalConnection@ onClearConnection;
	SignalConnection@ autocompleteCon;

	string[] lines;
	uint lines_end;
	uint size;

	uint num_chars;

	ConsoleElement(Rocket::Element@ appElement)
	{
		super(appElement);

		dirty = false;

		lines.resize(128);
		lines_end = 0;
		size = 0;

		num_chars = 0;

		@onDataConnection = console.connectToNewLine("void OnNewLine(const string &in)");
		@onClearConnection = console.connectToClear("void OnClear()");

		@autocomplete_menu = @ContextMenu(gui.getContext(), false);
		@autocompleteCon = autocomplete_menu.connectToChildClick("void OnAutocompleteClick(const MenuItemEvent &in)");
	}

	~ConsoleElement()
	{
		@__inner = null;

		@onDataConnection = null;
		@onClearConnection = null;

		autocomplete_menu.removeAllChildren();
		@autocompleteCon = null;
		@autocomplete_menu = null;
	}

	uint diff(uint front, uint back, uint buffer_length)
	{
		if (front >= back)
			return front - back;
		else
			return front + (buffer_length-back);
	}

	void AddLine(const string &in text)
	{
		if (text_area.GetCursorIndex() >= int(num_chars))
		{
			autoScroll = true;
			//slowScroll = true;
		}
		num_chars += text.length();

		// Add the line
		lines[lines_end] = text;

		if (++lines_end == lines.length())
			lines_end = 0;

		if (size < lines.length())
			++size;

		dirty = true;
	}

	void Clear()
	{
		lines_end = 0;
		size = 0;
		num_chars = 0;
		dirty = true;
	}

	void OnLayout()
	{
		if (text_area is null)
		{
			Rocket::Element@ text_element = GetElementById(Rocket::String("text_element"));
			if (text_element !is null)
				@text_area = cast<Rocket::ElementFormControlTextArea>(text_element);
			if (text_area is null)
			{
				console.println("Couldn't find text_element.");
			}
		}
	}
	
	void OnUpdate()
	{
	}

	void OnRender()
	{
		if (dirty && text_area !is null)
		{
			string linesText;
			{
				uint i = (size < lines.length() ? 0 : lines_end);
				for (uint done = 0; done < size; ++done)
				{
					linesText += lines[i++];
					linesText += '\n';

					if (i == lines.length())
						i = 0;
				}
			}
			text_area.SetValue( Rocket::String(linesText) );
			if (autoScroll)
			{
				//text_area.SetCursorIndex(current_text.Length(), true);
				// Send ctrl-end key-press event
				Rocket::Dictionary parameters;
				parameters.Set(Rocket::String("ctrl_key"), int(1));
				parameters.Set(Rocket::String("key_identifier"), int(Rocket::GUIKey::KI_END));
				text_area.DispatchEvent(Rocket::String("keydown"), parameters);
				text_area.ShowCursor(false, true); // scroll to cursor
				autoScroll = false;
			}
			dirty = false;
		}

		//if (slowScroll)
		//{
		//}
	}

	void OnNewLine(const string &in data)
	{
		AddLine(data);
	}

	void OnClear()
	{
		Clear();
	}
	
	void OnAutocompleteClick(const MenuItemEvent &in ev)
	{
		Rocket::ElementFormControlInput@ input = cast<Rocket::ElementFormControlInput>( GetElementById(Rocket::String("command_element")) );
		if (firstArg)
			input.SetValue( Rocket::String(ev.title) );
		else
		{
			Rocket::String completed = console.autocomplete(string(input.GetValue()), ev.title);
			input.SetValue(completed);
		}
		input.Focus();

		Rocket::Dictionary parameters;
		parameters.Set(Rocket::String("key_identifier"), int(Rocket::GUIKey::KI_END));
		input.DispatchEvent(Rocket::String("keydown"), parameters);
	}

}

bool acConnection = false;
Rocket::String lastvalue = Rocket::String("");
StringArray possibleCompletions;
void OnConsoleEntryChanged(Rocket::Event& ev)
{
	if (autocomplete_menu is null)
		return;

	Rocket::String value("");
	value = ev.GetParameter(Rocket::String("value"), value);

	if (value != lastvalue)
	{
		lastvalue = value;
		if (!value.Empty())
		{
			if (value[value.Length()-1] == Rocket::String(" "))
			{
				if (firstArg)
				{
					firstArg = false;
					firstArgLength = value.Length()-1;
				}
				autocomplete_menu.hide();
				return;
			}
			else
				autocomplete_menu.show();

			if (firstArg)
				console.listPrefixedCommands(string(value), possibleCompletions);
			else
				console.listPossibleCompletions(string(value), possibleCompletions);

			autocomplete_menu.removeAllChildren();
			for (uint i = 0; i < possibleCompletions.size(); i++)
			{
				MenuItem @newItem = @MenuItem(possibleCompletions[i], possibleCompletions[i]);
				autocomplete_menu.addChild(newItem);
			}
			if (possibleCompletions.size() > 0)
			{
				Rocket::Element@ target = ev.GetTargetElement();
				autocomplete_menu.show(target.GetAbsoluteLeft(), target.GetAbsoluteTop() + target.GetClientHeight() + 4);
			}
			else
				autocomplete_menu.hide();
		}
		else // 'value' is empty
		{
			autocomplete_menu.hide();
			autocomplete_menu.removeAllChildren();
		}

		// If first arg has been partially or completely deleted
		if (value.Length() <= firstArgLength)
		{
			firstArg = true;
			firstArgLength = 0;
		}
	}
}

//EventConnection@ consoleClickConnection;
void OnConsoleEnterClick(Rocket::Event& ev)
{
	autocomplete_menu.hide();
	autocomplete_menu.removeAllChildren();

	Rocket::Element@ consoleElem = ev.GetTargetElement().GetParentNode();
	Rocket::ElementFormControlInput@ input = cast<Rocket::ElementFormControlInput>( consoleElem.GetElementById(Rocket::String("command_element")) );

	string command = input.GetValue();
	console.println("> " + command);
	console.interpret(command);
	input.SetValue(Rocket::String(""));
}

void OnConsoleEntryEnter(Rocket::Event& ev)
{
	if (autocomplete_menu.getSelectedIndex() != -1)
	{
		autocomplete_menu.getSelectedItem().click();
		autocomplete_menu.hide();
		autocomplete_menu.removeAllChildren();
	}
	else
		OnConsoleEnterClick(ev);
}

void OnConsoleEntryKeyUp(Rocket::Event& ev)
{
	if (ev.GetType() == Rocket::String("keyup"))
	{
		int key_identifier = ev.GetParameter(Rocket::String("key_identifier"), int(0));
		if (key_identifier == Rocket::GUIKey::KI_UP)
		{
			autocomplete_menu.selectRelative(-1);
		}
		if (key_identifier == Rocket::GUIKey::KI_DOWN)
		{
			autocomplete_menu.selectRelative(1);
		}
	}
	else if (ev.GetType() == Rocket::String("keydown"))
	{
		int key_identifier = ev.GetParameter(Rocket::String("key_identifier"), int(0));
		if (key_identifier == Rocket::GUIKey::KI_UP || key_identifier == Rocket::GUIKey::KI_DOWN)
		{
			ev.StopPropagation();
		}
	}
}

void OnConsoleOpened(Rocket::Event &ev)
{
	//OnWindowLoad(ev);
}

void OnConsoleClosed()
{
	gui.hideDebugger();
}

void OnConsoleShow(Rocket::Event &ev)
{
	Rocket::Element@ input = ev.GetCurrentElement().GetElementById(Rocket::String("command_element"));
	input.Focus();

	//gui.enableDebugger();
	//gui.showDebugger();
}

void OnConsoleHide()
{
	//gui.hideDebugger();
}