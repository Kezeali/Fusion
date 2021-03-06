interface IContextMenuListener
{
	void OnContextMenuClick(const MenuItem&in);
}

class MenuItem
{
	uint index;
	string name;
	Rocket::Element@ element;

	MenuItem()
	{
		index = 0;
		name = "";
		@element = null;
	}

	MenuItem(uint i, const string&in n, Rocket::Element@ e)
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

// TODO: just use a dictionary here, rather than this shoddy array
//  (requires new AS array implementation, err, assuming that supports
//  dictionary-style arrays... otherwise use a modified version of the
//  dictionary add-on)
class EventConnectionList
{
	EventConnection@[] connections;
	uint count;
	EventConnectionList()
	{
		count = 0;
		connections.resize(1);
	}
	~EventConnectionList()
	{
		Clear();
	}
	void Add(EventConnection@ connection)
	{
		@connections[count++] = connection;
		if (count >= connections.length())
			connections.resize(count+3);
	}
	void Clear()
	{
		for (uint i = 0; i < count; i++)
		{
			connections[i].RemoveListener();
			@connections[i] = null;
		}
		connections.resize(1);
		count = 0;
	}
}

class ContextMenu : IEventListener
{
	//IEventListener@ clickListener;
	IContextMenuListener@ m_Listener;
	EventConnectionList m_EventConnections;

	MenuItem[] m_Items;
	uint m_ItemCount;

	int m_CurrentSelection;

	Document@ m_MenuDoc;
	Rocket::String m_Id;
	e_Vector2i m_Position;

	ContextMenu(const Rocket::String&in id, const e_Vector2i&in position)
	{
		m_Id = id;
		m_Position = position;

		@m_MenuDoc = gui.getContext().LoadDocument(Rocket::String("Data/core/gui/context_menu.rml"));
		if (m_MenuDoc !is null)
		{
			m_MenuDoc.SetId(id);
			m_MenuDoc.SetProperty(Rocket::String("left"), Rocket::String(position.x + "px"));
			m_MenuDoc.SetProperty(Rocket::String("top"), Rocket::String(position.y + "px"));

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
		m_MenuDoc.SetProperty(Rocket::String("left"), Rocket::String(x + "px"));
		m_MenuDoc.SetProperty(Rocket::String("top"), Rocket::String(y + "px"));
	}

	void SetListener(IContextMenuListener@ listener)
	{
		@m_Listener = @listener;
	}

	bool AddItem(const string&in name)
	{
		// Add the GUI element to the cntext menu document
		Rocket::Element@ element = @m_MenuDoc.CreateElement(Rocket::String("button"));
		element.SetProperty(Rocket::String("display"), Rocket::String("block"));
		element.SetProperty(Rocket::String("clip"), Rocket::String("auto"));
		element.SetAttribute(Rocket::String("menu_index"), m_ItemCount);
		element.SetId(Rocket::String("contextmenuitem-"+name+m_ItemCount));
		element.SetInnerRML(Rocket::String(name));
		m_MenuDoc.AppendChild(element);

		EventConnection@ cnx = element.AddEventListener(Rocket::String("click"), this);
		m_EventConnections.Add(cnx);

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
		m_EventConnections.Clear();
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
			m_Items[index].element.DispatchEvent(Rocket::String("click"));
		}
	}

	void ClickSelected()
	{
		ClickItem(m_CurrentSelection);
	}

	void OnAttach(Rocket::Element@) {}
	void OnDetach(Rocket::Element@) {}

	void ProcessEvent(Rocket::Event@ ev)
	{
		if (ev.GetType() == Rocket::String("click"))
		{
			Rocket::Element@ clicked = ev.GetTargetElement();
			uint index = clicked.GetAttribute(Rocket::String("menu_index"), m_ItemCount);
			if (index < m_ItemCount)
				m_Listener.OnContextMenuClick(m_Items[index]);

			Hide();
		}
	}
}
