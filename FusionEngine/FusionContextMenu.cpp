/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
		
		
	File Author(s):

		Elliot Hayward

*/

#include "Common.h"

#include "FusionContextMenu.h"

//#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	void MenuItem::init()
	{
		m_HideTimer.func_expired().set(this, &MenuItem::onHideTimeout);
	}

	MenuItem::MenuItem()
		: m_Element(NULL),
		m_Index(-1),
		m_Document(NULL),
		m_Context(NULL)
	{
		init();
	}

	MenuItem::MenuItem(const std::string &title, const std::string &value)
		: m_Title(title),
		m_Value(value),
		m_Element(NULL),
		m_Index(-1),
		m_Document(NULL),
		m_Context(NULL)
	{
		init();
	}

	MenuItem::~MenuItem()
	{
		RemoveAllChildren();

		if (m_Document != NULL)
			m_Document->Close();
		if (m_Element != NULL)
		{
			// Remove the event listeners, in case the reference removed below isn't the last
			m_Element->RemoveEventListener("click", this);
			// Remove these event listeners too, just in case
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);

			m_Element->RemoveReference();
		}
	}

	void MenuItem::Show()
	{
		if (m_Document != NULL)
		{
			int x = (int)std::ceil(m_Element->GetAbsoluteLeft() + m_Element->GetClientWidth() - 4);
			int y = (int)std::ceil(m_Element->GetAbsoluteTop());

			
			m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
			m_Document->SetProperty("top",  Rocket::Core::Property(y, Rocket::Core::Property::PX));
			m_Document->Show();
		}
	}

	void MenuItem::Hide()
	{
		if (m_Document != NULL)
		{
			m_Document->Hide();

			for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
			{
				MenuItem *child = *it;
				child->Hide();
			}
		}
	}

	int MenuItem::AddChild(MenuItem *item)
	{
		// Sub Menu:
		if (m_Element != NULL)
		{
			initSubmenu();

			Rocket::Core::Element *childElement = m_Document->CreateElement("menuitem");
			m_Document->AppendChild(childElement);

			item->addedToMenu(this, childElement);
		}
		// Root menu:
		else if (m_Document != NULL)
		{
			Rocket::Core::Element *childElement = m_Document->CreateElement("menuitem");
			m_Document->AppendChild(childElement);

			item->addedToMenu(this, childElement);
		}

		item->m_Index = m_Children.size();

		item->addRef();
		m_Children.push_back(item);
		return m_Children.size() - 1;
	}

	void MenuItem::RemoveChild(int index)
	{
		// Find the item in question
		MenuItem *child = m_Children[index];
		// Remove the GUI element that represents the item from the document
		m_Document->RemoveChild(child->m_Element);
		// Erase the item
		child->release();
		m_Children.erase(m_Children.begin() + index);

		if (m_Children.empty() && m_Element != NULL) // Remove sub-menu stuff if this item has no more child-items
		{
			m_Element->SetPseudoClass("submenu", false);
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);
		}
	}

	void MenuItem::RemoveAllChildren()
	{
		for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			MenuItem *child = *it;
			m_Document->RemoveChild(child->m_Element);
			child->release();
		}

		m_Children.clear();

		if (m_Element != NULL) // This element no longer has a sub-menu
		{
			m_Element->SetPseudoClass("submenu", false);
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);
		}
	}

	void MenuItem::SelectChild(int id)
	{
		// Validate the selected ID
		if (id < 1)
			id = 0;
		else if (id >= (signed)m_Children.size())
			id = m_Children.size()-1;
		m_SelectedItem = id;

		for (int i = 0, size = (signed)m_Children.size(); i < size; ++i)
		{
			m_Children[i]->m_Element->SetPseudoClass("selected", i == id);
		}
	}

	void MenuItem::SelectChildRelative(int distance)
	{
		SelectChild(m_SelectedItem + distance);
	}

	void MenuItem::ProcessEvent(Rocket::Core::Event& ev)
	{
		if (m_Document != NULL)
		{
			if (ev.GetType() == "mouseover" || ev.GetType() == "click")
			{
				m_HideTimer.stop();
				Show();
			}
			else if (ev.GetType() == "mouseout")
			{
				m_HideTimer.stop();
				m_HideTimer.start(500, false);
			}
		}
		else
		{
			if (ev.GetType() == "click")
			{
				MenuItemEvent ev;
				ev.title = m_Title;
				ev.value = NULL;
				SignalClicked(ev);
			}
		}
	}

	void MenuItem::EnumReferences(asIScriptEngine *engine)
	{
		if (m_Parent != NULL)
			engine->GCEnumCallback((void*)m_Parent);

		for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			engine->GCEnumCallback((void*)*it);
		}
	}

	void MenuItem::ReleaseAllReferences(asIScriptEngine *engine)
	{
		if (m_Parent != NULL)
			m_Parent->release();

		for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			MenuItem *item = *it;
			item->release();
		}
	}

	void MenuItem::onHideTimeout()
	{
		if (!isPseudoClassSetOnAnyChild("hover"))
			Hide();
	}

	bool MenuItem::isPseudoClassSetOnAnyChild(const EMP::Core::String &pseudo_class)
	{
		if (m_Element != NULL && m_Element->IsPseudoClassSet(pseudo_class))
			return true;

		if (m_Document != NULL)
		{
			if (m_Document->IsPseudoClassSet(pseudo_class))
				return true;

			for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
			{
				MenuItem *child = *it;
				if (child->isPseudoClassSetOnAnyChild(pseudo_class))
					return true;
			}
		}

		return false;
	}

	void MenuItem::initSubmenu()
	{
		if (m_Document == NULL)
		{
			m_Document = m_Context->LoadDocument("core/gui/context_menu.rml");
			m_Document->AddEventListener("mouseout", this);
		}

		m_Element->SetPseudoClass("submenu", true);
		// Sub-menus need to open on mouse over / close on mouse leave
		m_Element->AddEventListener("mouseover", this);
		m_Element->AddEventListener("mouseout", this);
	}

	void MenuItem::addedToMenu(MenuItem *parent, Rocket::Core::Element *element)
	{
		// note that the parent's ref-count isn't incremented: the parent can't be deleted
		//  without deleting the child so that would be an unnecessary circular reference
		m_Parent = parent; 
		m_Element = element;

		m_Context = m_Element->GetContext();

		m_Element->SetProperty("display", "block");
		m_Element->SetProperty("clip", "auto");
		m_Element->SetAttribute("menu_index", m_Index);
		m_Element->SetId("contextmenuitem-" + EMP::Core::String(m_Value.c_str(), m_Value.c_str()+m_Value.length()));
		m_Element->SetInnerRML(m_Title.c_str());

		m_Element->AddEventListener("click", this);

		// Do a late initialisation of this menu-item's children, if necessary
		if (m_Document == NULL && !m_Children.empty())
		{
			initSubmenu();

			for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
			{
				Rocket::Core::Element *childElement = m_Document->CreateElement("menuitem");
				m_Document->AppendChild(childElement);

				MenuItem *child = *it;
				child->addedToMenu(this, childElement);
			}
		}
	}

	ContextMenu::ContextMenu(Rocket::Core::Context *context, bool auto_hide)
		: m_AutoHide(auto_hide)
	{
		m_Context = context;
		m_Document = m_Context->LoadDocument("core/gui/context_menu.rml");

		InputManager *manager = InputManager::getSingletonPtr();
		if (manager != NULL)
		{
			m_RawInputConnection = manager->SignalRawInput.connect( boost::bind(&ContextMenu::OnRawInput, this, _1) );
		}
	}

	ContextMenu::ContextMenu(Rocket::Core::Context *context, InputManager *input, bool auto_hide)
		: m_AutoHide(auto_hide)
	{
		m_Context = context;
		m_Document = m_Context->LoadDocument("core/gui/context_menu.rml");

		m_RawInputConnection = input->SignalRawInput.connect( boost::bind(&ContextMenu::OnRawInput, this, _1) );
	}

	ContextMenu::~ContextMenu()
	{
		m_RawInputConnection.disconnect();
	}

	void ContextMenu::SetPosition(int x, int y)
	{
		m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
		m_Document->SetProperty("top", Rocket::Core::Property(y, Rocket::Core::Property::PX));
	}

	void ContextMenu::Show()
	{
		m_Document->Show();
	}

	void ContextMenu::Show(int x, int y)
	{
		m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
		m_Document->SetProperty("top", Rocket::Core::Property(y, Rocket::Core::Property::PX));
		m_Document->Show();
	}

	void ContextMenu::OnRawInput(const RawInput &input)
	{
		if (m_AutoHide &&
			input.ButtonPressed && input.InputType == RawInput::Button && input.Code == CL_MOUSE_LEFT &&
			!isPseudoClassSetOnAnyChild("hover"))
		{
			Hide();
		}
	}

	void ContextMenu::ProcessEvent(Rocket::Core::Event &ev)
	{
		if (ev.GetType() == "click" && ev.GetTargetElement() == m_Context->GetRootElement())
		{
			Hide();
		}
	}

	void ContextMenu::Register(asIScriptEngine *engine)
	{
	}

}
