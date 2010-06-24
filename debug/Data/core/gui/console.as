void InitialiseConsole()
{
	RegisterElementType(e_String("console"), e_String("ConsoleElement"));
}

ContextMenu@ autocomplete_menu;

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

	ElementFormControlTextArea@ text_area;
	SignalConnection@ onDataConnection;
	SignalConnection@ onClearConnection;
	SignalConnection@ autocompleteCon;

	string@ text_block;

	string[] lines;
	uint lines_end;
	uint size;

	ConsoleElement(Element@ appElement)
	{
		super(appElement);

		dirty = false;
		current_text = "";
		length = 0;

		@text_block = string();

		lines.resize(128);
		lines_end = 0;
		size = 0;

		//@consoleConnection = console.connectListener(this);
		@onDataConnection = console.connectToNewLine("void OnNewLine(const string &in)");//console.connectToNewData("void OnNewData(const string &in)");
		@onClearConnection = console.connectToClear("void OnClear()");

		//console.println("Creating autocomplete_menu (context menu)");
		@autocomplete_menu = @ContextMenu(gui.getContext(), false);
		//console.println("Connecting to Click.");
		@autocompleteCon = autocomplete_menu.connectToClick("void OnAutocompleteClick(const MenuItemEvent &in)"); // This still doesn't get garbage collected
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

	uint length;
	e_String current_text;

	void AddText(const string &in text)
	{
		if (text_area is null)
		{
			//console.println("Storing reference to console text_element");
			Element@ text_element = GetElementById(e_String("text_element"));
			if (text_element !is null)
				@text_area = cast<ElementFormControlTextArea>(text_element);
			if (text_area is null)
				return;
		}

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

	uint diff(uint front, uint back, uint buffer_length)
	{
		if (front >= back)
			return front - back;
		else
			return front + (buffer_length-back);
	}

	void AddLine(const string &in text)
	{
		if (text_area.GetCursorIndex() >= text_block.length()-1)
		{
			autoScroll = true;
			//slowScroll = true;
		}

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
		//text_block.clear();
		//current_text = "";
		dirty = true;
	}

	void OnLayout()
	{
		if (text_area is null)
		{
			Element@ text_element = GetElementById(e_String("text_element"));
			if (text_element !is null)
				@text_area = cast<ElementFormControlTextArea>(text_element);
			if (text_area is null)
			{
				console.println("Couldn't find text_element.");
			}
		}
	}

	void OnRender()
	{
		if (dirty && text_area !is null)
		{
			string@ linesText = @string();
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
			text_area.SetValue( e_String(linesText) );
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

	void OnNewData(const string &in data)
	{
		AddText(data);
	}

	void OnClear()
	{
		Clear();
	}
	
	void OnAutocompleteClick(const MenuItemEvent &in ev)
	{
		ElementFormControlInput@ input = cast<ElementFormControlInput>( GetElementById(e_String("command_element")) );
		input.SetValue( e_String(ev.title) );
		input.Focus();

		e_Dictionary parameters;
		parameters.Set(e_String("key_identifier"), int(GUIKey::KI_END));
		input.DispatchEvent(e_String("keydown"), parameters);
	}

}

bool acConnection = false;
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
				autocomplete_menu.hide();
				return;
			}

			if (autocomplete_menu !is null)
			{
				console.listPrefixedCommands(string(value), possibleCommands);
				autocomplete_menu.removeAllChildren();
				for (uint i = 0; i < possibleCommands.size(); i++)
				{
					MenuItem @newItem = @MenuItem(possibleCommands[i], possibleCommands[i]);
					autocomplete_menu.addChild(newItem);
				}

				if (possibleCommands.size() > 0)
				{
					Element@ target = ev.GetTargetElement();
					autocomplete_menu.show(target.GetAbsoluteLeft(), target.GetAbsoluteTop() + target.GetClientHeight() + 4);
				}
				else
					autocomplete_menu.hide();
			}
		}
		else // 'value' is empty
		{
			autocomplete_menu.hide();
			autocomplete_menu.removeAllChildren();
		}
	}
	else if (value.Length() <= commandLength)
	{
		commandDone = false;
		commandLength = 0;
		autocomplete_menu.show();
	}
}

EventConnection@ consoleClickConnection;
void OnConsoleEnterClick(Event& ev)
{
	autocomplete_menu.hide();
	autocomplete_menu.removeAllChildren();

	Element@ consoleElem = ev.GetTargetElement().GetParentNode();
	ElementFormControlInput@ input = cast<ElementFormControlInput>( consoleElem.GetElementById(e_String("command_element")) );

	string command = input.GetValue();
	console.println("> " + command);
	console.interpret(command);
	input.SetValue(e_String(""));
}

void OnConsoleEntryEnter(Event& ev)
{
	if (autocomplete_menu.getSelectedIndex() != -1)
		autocomplete_menu.getSelectedItem().click();
	else
		OnConsoleEnterClick(ev);
	autocomplete_menu.hide();
	autocomplete_menu.removeAllChildren();
}

void OnConsoleEntryKeyUp(Event& ev)
{
	if (ev.GetType() == e_String("keyup"))
	{
		int key_identifier = ev.GetParameter(e_String("key_identifier"), int(0));
		if (key_identifier == GUIKey::KI_UP)
		{
			autocomplete_menu.selectRelative(-1);
		}
		if (key_identifier == GUIKey::KI_DOWN)
		{
			autocomplete_menu.selectRelative(1);
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

void OnConsoleOpened(Event &ev)
{
	OnWindowLoad(ev);
}

void OnConsoleClosed()
{
	gui.hideDebugger();
}

void OnConsoleShow(Event &ev)
{
	Element@ input = ev.GetCurrentElement().GetElementById(e_String("command_element"));
	input.Focus();

	gui.enableDebugger();
	gui.showDebugger();
}

void OnConsoleHide()
{
	gui.hideDebugger();
}