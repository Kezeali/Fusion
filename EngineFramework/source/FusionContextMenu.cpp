/*
*  Copyright (c) 2009-2012 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionContextMenu.h"

#include "FusionGUI.h"
//#include <boost/lexical_cast.hpp>
#include <ClanLib/Display/Window/keys.h>
//#include <ScriptUtils/Inheritance/RegisterConversion.h>
#include "FusionScriptedSlots.h"
#include "scriptstdstring.h"

using namespace std::placeholders;

namespace FusionEngine
{

	void MenuItem::init()
	{
		m_HideTimer.func_expired().set(this, &MenuItem::onHideTimeout);
	}

	MenuItem::MenuItem()
		: m_Element(nullptr),
		m_Index(-1),
		m_Document(nullptr),
		m_Context(nullptr),
		m_Parent(nullptr),
		m_SelectedItem(-1)
	{
		init();
	}

	MenuItem::MenuItem(const std::string &title, const std::string &value)
		: m_Title(title),
		m_Value(value),
		m_Element(nullptr),
		m_Index(-1),
		m_Document(nullptr),
		m_Context(nullptr),
		m_Parent(nullptr),
		m_SelectedItem(-1)
	{
		init();
	}

	MenuItem::~MenuItem()
	{
		RemoveAllChildren();

		if (m_Document != nullptr)
		{
			if (m_Element != nullptr) // If this is a submenu
				m_Document->RemoveEventListener("mouseout", this); // Remove the mouse-out listener (used for auto-close)
			m_Document->Close();
			m_Document->RemoveReference();
		}

		if (m_Element != nullptr)
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
		if (m_Document != nullptr)
		{
			int x = (int)std::ceil(m_Element->GetAbsoluteLeft() + m_Element->GetClientWidth() - 4);
			int y = (int)std::ceil(m_Element->GetAbsoluteTop());

			const Rocket::Core::Vector2i &dimensions = m_Context->GetDimensions();
			if (x + m_Document->GetOffsetWidth() > dimensions.x)
				x = (int)std::ceil(m_Element->GetAbsoluteLeft() - m_Document->GetClientWidth() + 5);
			if (y + m_Document->GetOffsetHeight() > dimensions.y)
				y -= (int)(y + m_Document->GetOffsetHeight() - dimensions.y);
			
			m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
			m_Document->SetProperty("top",  Rocket::Core::Property(y, Rocket::Core::Property::PX));
			m_Document->Show();
		}
	}

	void MenuItem::Hide()
	{
		if (m_Document != nullptr)
		{
			m_Document->Hide();

			for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
			{
				MenuItem *child = *it;
				child->Hide();
			}
		}
	}

	bool MenuItem::IsOpen() const
	{
		return m_Document != nullptr && m_Document->IsVisible();
	}

	int MenuItem::AddChild(MenuItem *item)
	{
		// Sub Menu:
		if (m_Element != nullptr)
		{
			initSubmenu();

			Rocket::Core::Element *childElement = m_Document->CreateElement("menuitem");
			m_Document->AppendChild(childElement);

			Rocket::Core::Element *iconElement = Rocket::Core::Factory::InstanceElement(childElement, "icon", "icon", Rocket::Core::XMLAttributes());
			childElement->AppendChild(iconElement);
			iconElement->RemoveReference();

			item->addedToMenu(this, childElement);
		}
		// Root menu:
		else if (m_Document != nullptr)
		{
			Rocket::Core::Element *childElement = m_Document->CreateElement("menuitem");
			m_Document->AppendChild(childElement);

			Rocket::Core::Element *iconElement = Rocket::Core::Factory::InstanceElement(childElement, "icon", "icon", Rocket::Core::XMLAttributes());
			childElement->AppendChild(iconElement);
			iconElement->RemoveReference();

			item->addedToMenu(this, childElement);
		}

		item->m_Index = m_Children.size();

		item->addRef();
		m_Children.push_back(item);
		return m_Children.size() - 1;
	}

	void MenuItem::RemoveChild(MenuItem *item)
	{
		MenuItem *child = nullptr;
		// Find the item in question
		for (auto it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			if (*it == item)
			{
				m_Children.erase(it);
				break;
			}
		}
		// Remove the GUI element that represents the item from the document
		m_Document->RemoveChild(item->m_Element);
		// Erase the item
		item->release();

		if (m_Children.empty() && m_Element != nullptr) // Remove sub-menu stuff if this item has no more child-items
		{
			m_Element->SetPseudoClass("submenu", false);
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);
		}
	}

	void MenuItem::RemoveChild(int index)
	{
		// Set the selection to 'none' if the removed item was selected
		if (index == m_SelectedItem)
			m_SelectedItem = -1;

		// Find the item in question
		MenuItem *child = m_Children[index];
		// Remove the GUI element that represents the item from the document
		m_Document->RemoveChild(child->m_Element);
		// Erase the item
		child->release();
		m_Children.erase(m_Children.begin() + index);

		if (m_Children.empty() && m_Element != nullptr) // Remove sub-menu stuff if this item has no more child-items
		{
			m_Element->SetPseudoClass("submenu", false);
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);
		}
	}

	void MenuItem::RemoveAllChildren()
	{
		m_SelectedItem = -1;

		for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			MenuItem *child = *it;
			m_Document->RemoveChild(child->m_Element);
			child->release();
		}

		m_Children.clear();

		if (m_Element != nullptr)
		{
			// This element no longer has a sub-menu
			m_Element->SetPseudoClass("submenu", false);
			m_Element->RemoveEventListener("mouseover", this);
			m_Element->RemoveEventListener("mouseout", this);
		}
	}

	MenuItem *MenuItem::GetChild(int index) const
	{
		if (index >= 0 && (unsigned)index < m_Children.size())
			return m_Children[(unsigned)index];
		else
			return nullptr;
	}

	int MenuItem::GetNumChildren() const
	{
		return m_Children.size();
	}

	bool MenuItem::Empty() const
	{
		return m_Children.empty();
	}

	void MenuItem::SelectChild(int id)
	{
		// Select none if an out-of-range ID is given
		if (id < 0)
			id = -1;
		else if (id >= (signed)m_Children.size())
			id = -1;
		m_SelectedItem = id;

		for (int i = 0, size = (signed)m_Children.size(); i < size; ++i)
		{
			m_Children[i]->m_Element->SetPseudoClass("selected", i == id);
		}
	}

	void MenuItem::SelectChildRelative(int distance)
	{
		int id = m_SelectedItem + distance;
		// Wrap around
		if (id < 0)
			id = m_Children.size()-1;
		else if (id >= (signed)m_Children.size())
			id = 0;
		SelectChild(id);
	}

	int MenuItem::GetSelectedIndex() const
	{
		return m_SelectedItem;
	}

	MenuItem *MenuItem::GetSelectedItem()
	{
		return GetChild(m_SelectedItem);
	}

	void MenuItem::Click()
	{
		m_Element->Click();
	}

	bool MenuItem::IsSubmenu() const
	{
		return m_Document != nullptr && m_Element != nullptr;
	}

	bool MenuItem::IsTopMenu() const
	{
		return m_Document != nullptr && m_Element == nullptr;
	}

	bool MenuItem::IsMenu() const
	{
		return m_Document != nullptr;
	}

	void MenuItem::SetBGColour(const CL_Color& colour)
	{
		std::stringstream colourStr;
		colourStr << "rgba(" << colour.get_red() << "," << colour.get_green() << "," << colour.get_blue() << "," << colour.get_alpha() << ")";
		Rocket::Core::String colourAttr(("border-color: " + colourStr.str() + "; border-width: 1px;").c_str());
		m_Element->SetAttribute("style", colourAttr);
	}

	void MenuItem::ProcessEvent(Rocket::Core::Event& ev)
	{
		if (m_Document != nullptr)
		{
			if (ev.GetType() == "click")
			{
				m_HideTimer.stop();
				Show();
				std::for_each(m_Parent->m_Children.begin(), m_Parent->m_Children.end(),
					[this](MenuItem *child)
				{
					if (child != this) child->Hide();
				});
			}
			else if (ev.GetType() == "mouseover")
			{
				m_HideTimer.stop();
				// This can only auto-open (on mouseover) if no other items in the same menu are open,
				//  hence:
				if (!std::any_of(m_Parent->m_Children.begin(), m_Parent->m_Children.end(), [](MenuItem *child)->bool { return child->IsOpen(); }))
					Show();
			}
			else if (ev.GetType() == "mouseout")
			{
				m_HideTimer.stop();
				m_HideTimer.start(500, false);
			}
		}
		if (ev.GetTargetElement() == m_Element || ev.GetTargetElement() == m_Document)
		{
			if (ev.GetType() == "click" && ev.GetTargetElement()->GetOwnerDocument()->IsVisible())
			{
				MenuItemEvent item_ev;
				item_ev.title = m_Title;
				item_ev.value = m_Value;
				SignalClicked(item_ev);
				if (m_Parent)
					m_Parent->onChildClicked(this, item_ev);
			}
		}
	}

	void MenuItem::EnumReferences(asIScriptEngine *engine)
	{
		if (m_Parent != nullptr)
			engine->GCEnumCallback((void*)m_Parent);

		for (MenuItemArray::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
		{
			engine->GCEnumCallback((void*)*it);
		}
	}

	void MenuItem::ReleaseAllReferences(asIScriptEngine *engine)
	{
		if (m_Parent != nullptr)
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

		for (auto it = m_Parent->m_Children.begin(), end = m_Parent->m_Children.end(); it != end; ++it)
		{
			MenuItem *child = *it;
			if (child->m_Element->IsPseudoClassSet("hover"))
			{
				child->Show();
				break;
			}
		}
	}

	bool MenuItem::isPseudoClassSetOnAnyChild(const Rocket::Core::String &pseudo_class)
	{
		if (m_Element != nullptr && m_Element->IsPseudoClassSet(pseudo_class))
			return true;

		if (m_Document != nullptr)
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
		if (m_Document == nullptr)
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
		m_Element->SetId("contextmenuitem-" + Rocket::Core::String(m_Value.c_str(), m_Value.c_str()+m_Value.length()));
		Rocket::Core::Factory::InstanceElementText(m_Element, m_Title.c_str());
		//m_Element->SetInnerRML(m_Title.c_str());

		m_Element->AddEventListener("click", this);

		// Do a late initialisation of this menu-item's children, if necessary
		if (m_Document == nullptr && !m_Children.empty())
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

	void MenuItem::onChildClicked(MenuItem *clicked_child, const MenuItemEvent &ev)
	{
		SignalChildClicked(ev);
	}

	ContextMenu::ContextMenu(Rocket::Core::Context *context, bool auto_hide)
		: m_AutoHide(auto_hide)
	{
		//context->AddReference();
		m_Context = context;
		m_Document = m_Context->LoadDocument("core/gui/context_menu.rml");

		InputManager *manager = InputManager::getSingletonPtr();
		if (manager != nullptr)
		{
			m_RawInputConnection = manager->SignalRawInput.connect( std::bind(&ContextMenu::OnRawInput, this, _1) );
		}
	}

	ContextMenu::ContextMenu(Rocket::Core::Context *context, InputManager *input, bool auto_hide)
		: m_AutoHide(auto_hide)
	{
		//context->AddReference();
		m_Context = context;
		m_Document = m_Context->LoadDocument("core/gui/context_menu.rml");

		m_RawInputConnection = input->SignalRawInput.connect( std::bind(&ContextMenu::OnRawInput, this, _1) );
	}

	ContextMenu::~ContextMenu()
	{
		//if (m_Context != nullptr)
		//	m_Context->RemoveReference();
		m_RawInputConnection.disconnect();
	}

	void ContextMenu::SetPosition(int x, int y)
	{
		m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
		m_Document->SetProperty("top", Rocket::Core::Property(y, Rocket::Core::Property::PX));
	}

	void ContextMenu::Show()
	{
		m_Document->Show(Rocket::Core::ElementDocument::NONE);
	}

	void ContextMenu::Show(int x, int y, bool fit_within_context)
	{
		if (fit_within_context)
		{
			const Rocket::Core::Vector2i &dimensions = m_Context->GetDimensions();
			if (x + m_Document->GetOffsetWidth() > dimensions.x)
				if (m_Element != nullptr)
					x = (int)std::ceil(m_Element->GetAbsoluteLeft() - m_Document->GetClientWidth() + 5);
				else
					x = dimensions.x - (int)std::ceil(m_Document->GetClientWidth());
			if (y + m_Document->GetOffsetHeight() > dimensions.y)
				y -= (int)std::ceil(y + m_Document->GetOffsetHeight() - dimensions.y);
		}

		m_Document->SetProperty("left", Rocket::Core::Property(x, Rocket::Core::Property::PX));
		m_Document->SetProperty("top", Rocket::Core::Property(y, Rocket::Core::Property::PX));
		//Rocket::Core::Element *focus = m_Context->GetFocusElement();
		m_Document->Show(Rocket::Core::ElementDocument::NONE);
		//focus->Focus();
		m_Document->PullToFront();
		//m_Document->Blur();
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

	const std::string& MenuItemEvent_get_title(MenuItemEvent *obj)
	{
		return obj->title;
	}

	const std::string& MenuItemEvent_get_value(MenuItemEvent *obj)
	{
		return obj->value;
	}

	MenuItem *MenuItem_Factory(const std::string &title, const std::string &value)
	{
		return new MenuItem(title, value);
	}

	MenuItem *MenuItem_FactoryNoValue(const std::string &title)
	{
		return new MenuItem(title, title);
	}

	ScriptedSlotWrapper *MenuItem_ConnectToClick(const std::string &decl, MenuItem *obj)
	{
		ScriptedSlotWrapper *slot = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), decl);
		if (slot != nullptr)
		{
			boost::signals2::connection c = obj->SignalClicked.connect( std::bind(&ScriptedSlotWrapper::Callback<const MenuItemEvent &>, slot, _1) );
			slot->HoldConnection(c);
		}
		return slot;
	}

	ScriptedSlotWrapper *MenuItem_ConnectToChildClick(const std::string &decl, MenuItem *obj)
	{
		ScriptedSlotWrapper *slot = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), decl);
		if (slot != nullptr)
		{
			boost::signals2::connection c = obj->SignalChildClicked.connect( std::bind(&ScriptedSlotWrapper::Callback<const MenuItemEvent &>, slot, _1) );
			slot->HoldConnection(c);
		}
		return slot;
	}

	void MenuItem::Register(asIScriptEngine *engine)
	{
		MenuItem::RegisterType<MenuItem>(engine, "MenuItem");
		engine->RegisterObjectBehaviour("MenuItem", asBEHAVE_FACTORY, "MenuItem@ f(const string &in, const string &in)", asFUNCTION(MenuItem_Factory), asCALL_CDECL);
		engine->RegisterObjectBehaviour("MenuItem", asBEHAVE_FACTORY, "MenuItem@ f(const string &in)", asFUNCTION(MenuItem_FactoryNoValue), asCALL_CDECL);

		engine->RegisterObjectType("MenuItemEvent", sizeof(MenuItemEvent), asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectMethod("MenuItemEvent", "const string& get_title() const", asFUNCTION(MenuItemEvent_get_title), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("MenuItemEvent", "const string& get_value() const", asFUNCTION(MenuItemEvent_get_value), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectMethod("MenuItem", "void click()", asMETHOD(MenuItem, Click), asCALL_THISCALL);

		// This is a virtual method that is overloaded by ContextMenu, so it isn't registered in the RegisterMethods method (it is registered in ContextMenu::Register)
		engine->RegisterObjectMethod("MenuItem", "void show()", asMETHODPR(MenuItem, Show, (void), void), asCALL_THISCALL);
		RegisterMethods(engine, "MenuItem");
	}

	int MenuItem_AddChild(MenuItem* item, MenuItem *obj)
	{
		int index = obj->AddChild(item);
		item->release();
		return index;
	}

	MenuItem *MenuItem_GetChild(int index, MenuItem *obj)
	{
		MenuItem *child = obj->GetChild(index);
		if (child != nullptr)
			child->addRef();
		return child;
	}

	MenuItem *MenuItem_GetSelectedItem(MenuItem *obj)
	{
		MenuItem *child = obj->GetSelectedItem();
		if (child != nullptr)
			child->addRef();
		return child;
	}

	void MenuItem::RegisterMethods(asIScriptEngine *engine, const std::string &type)
	{
		engine->RegisterObjectMethod(type.c_str(), "void hide()", asMETHODPR(MenuItem, Hide, (void), void), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "int addChild(MenuItem@)", asFUNCTION(MenuItem_AddChild), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod(type.c_str(), "void removeChild(MenuItem@)", asMETHODPR(MenuItem, RemoveChild, (MenuItem*), void), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "void removeChild(int)", asMETHODPR(MenuItem, RemoveChild, (int), void), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "void removeAllChildren()", asMETHOD(MenuItem, RemoveAllChildren), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "MenuItem@ getChild(int) const", asFUNCTION(MenuItem_GetChild), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod(type.c_str(), "int getNumChildren() const", asMETHOD(MenuItem, GetNumChildren), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "bool empty() const", asMETHOD(MenuItem, Empty), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "void select(int)", asMETHOD(MenuItem, SelectChild), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "void selectRelative(int)", asMETHOD(MenuItem, SelectChildRelative), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "int getSelectedIndex() const", asMETHOD(MenuItem, GetSelectedIndex), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "MenuItem@ getSelectedItem()", asFUNCTION(MenuItem_GetSelectedItem), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod(type.c_str(), "bool isSubmenu() const", asMETHOD(MenuItem, IsSubmenu), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "bool isTopMenu() const", asMETHOD(MenuItem, IsTopMenu), asCALL_THISCALL);
		engine->RegisterObjectMethod(type.c_str(), "bool isMenu() const", asMETHOD(MenuItem, IsMenu), asCALL_THISCALL);

		engine->RegisterObjectMethod(type.c_str(), "SignalConnection@ connectToClick(const string &in)", asFUNCTION(MenuItem_ConnectToClick), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod(type.c_str(), "SignalConnection@ connectToChildClick(const string &in)", asFUNCTION(MenuItem_ConnectToChildClick), asCALL_CDECL_OBJLAST);
	}

	ContextMenu *ContextMenu_FactoryDefault()
	{
		return new ContextMenu(GUI::getSingleton().GetContext(), true);
	}

	void ContextMenu_FactoryGeneric(asIScriptGeneric *gen)
	{
		ContextMenu *contextMenu = new ContextMenu(GUI::getSingleton().GetContext(), true);
		gen->SetReturnAddress((void*)contextMenu);
	}

	ContextMenu *ContextMenu_Factory(Rocket::Core::Context *context, bool auto_hide)
	{
		// This method is fully commented for later reference (about AngelScript's behaviour wrt. passing references)

		// Construct the context menu
		ContextMenu* contextMenu = new ContextMenu(context, auto_hide);
		// Passing a handle to a method increases the ref. count automatically,
		//  so it is the method's responsibility to free it:
		context->RemoveReference();
		// Now that everything is tidy, return the pointer to the constructed obj.
		return contextMenu;
	}

	void ContextMenu_ShowWithinCtx(int x, int y, ContextMenu *obj)
	{
		obj->Show(x, y);
	}

	void ContextMenu::Register(asIScriptEngine *engine)
	{
		MenuItem::Register(engine);

		ContextMenu::RegisterType<ContextMenu>(engine, "ContextMenu");

		engine->RegisterObjectBehaviour("ContextMenu", asBEHAVE_FACTORY, "ContextMenu@ f()", asFUNCTION(ContextMenu_FactoryGeneric), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("ContextMenu", asBEHAVE_FACTORY, "ContextMenu@ f(Context@, bool)", asFUNCTION(ContextMenu_Factory), asCALL_CDECL);
		
		engine->RegisterObjectMethod("ContextMenu", "void show()", asMETHODPR(ContextMenu, Show, (void), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("ContextMenu", "void show(int, int, bool)", asMETHODPR(ContextMenu, Show, (int, int, bool), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("ContextMenu", "void show(int, int)", asFUNCTION(ContextMenu_ShowWithinCtx), asCALL_CDECL_OBJLAST);

		// Register MenuItem inheritance
		RegisterBaseOf<MenuItem, ContextMenu>(engine, "MenuItem", "ContextMenu");
		MenuItem::RegisterMethods(engine, "ContextMenu");
	}

}
