/*
*  Copyright (c) 2009-2010 Fusion Project Team
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

#ifndef H_FusionEngine_ContextMenu
#define H_FusionEngine_ContextMenu

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionRefCounted.h"

#include "FusionInputHandler.h"

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Core/EventListener.h>

#include "FusionBoostSignals2.h"


namespace FusionEngine
{

	class MenuItem;
	typedef boost::intrusive_ptr<MenuItem> MenuItemPtr;

	struct MenuItemEvent
	{
		std::string title;
		std::string value;
	};

	//! MenuItem class
	/*!
	* \todo TODO: Rename MenuItem#Show() -> Open() and MenuItem#Hide() -> Close()
	* \todo A a hide-timeout property (currently it is a static value)
	*
	* \see ContextMenu
	*/
	// NOTE that RefCounted needs to be first here, or an exception occors
	//  whenever an instance of this class is created in a script
	class MenuItem : public RefCounted, public noncopyable, public Rocket::Core::EventListener
	{
	public:
		//! Ctor
		MenuItem();
		//! Ctor
		MenuItem(const std::string &title, const std::string &value);
		//! Dtor
		virtual ~MenuItem();

		//! Opens the submenu attached to this item
		virtual void Show();
		//! Closes the submenu attached to this item
		void Hide();
		//! Returns true if this item's submenu is open
		bool IsOpen() const;

		//! Adds a child element to this item's sub-menu
		int AddChild(MenuItem *item);

		//! Removes the given menu item from this item's sub-menu
		void RemoveChild(MenuItem *item);
		//! Removes the submenu item at the given index
		void RemoveChild(int index);

		//! Removes all menu item's from this item's attached submenu
		void RemoveAllChildren();

		//! Returns the menu-item from this item's sub-menu
		MenuItem *GetChild(int index) const;

		int GetNumChildren() const;
		bool Empty() const;

		//! Selects the given sub-menu item
		void SelectChild(int index);
		//! Moves the selection
		void SelectChildRelative(int distance);

		//! Returns the selected index
		int GetSelectedIndex() const;
		//! Returns the selected menu item
		MenuItem *GetSelectedItem();

		void Click();

		//! Returns true if this object represents a menu-item that opens a sub-menu
		bool IsSubmenu() const;
		//! Returns true if this is the top-level menu of the context menu
		bool IsTopMenu() const;
		//! Returns true if this is either an item that opens a sub-menu OR the top-level menu
		bool IsMenu() const;

		virtual void ProcessEvent(Rocket::Core::Event& ev);

		virtual void EnumReferences(asIScriptEngine *engine);
		virtual void ReleaseAllReferences(asIScriptEngine *engine);

		void onHideTimeout();

		boost::signals2::signal<void (const MenuItemEvent&)> SignalClicked;
		
		//! Registers the script interface to this class (used internally - call ContextMenu#Register() instead)
		static void Register(asIScriptEngine *engine);
		//! Registers non-virtual methods of this class (used internally)
		static void RegisterMethods(asIScriptEngine *engine, const std::string &type);

	protected:
		int m_Index;
		std::string m_Title;
		std::string m_Value;

		// The button for this menu item
		Rocket::Core::Element *m_Element;

		MenuItem *m_Parent;
		typedef std::vector<MenuItem*> MenuItemArray;
		MenuItemArray m_Children;

		int m_SelectedItem;

		CL_Timer m_HideTimer;

		Rocket::Core::ElementDocument *m_Document;
		Rocket::Core::Context *m_Context;

		bool isPseudoClassSetOnAnyChild(const Rocket::Core::String &pseudo_class);

		void init();
		void initSubmenu();
		void addedToMenu(MenuItem *parent, Rocket::Core::Element *element);

		void onChildClicked(MenuItem *clicked_child, const MenuItemEvent &ev);
	};

	//! Spawns a context-menu GUI document
	class ContextMenu : public MenuItem
	{
	public:
		//! Ctor
		ContextMenu(Rocket::Core::Context *context,  bool auto_hide = false);
		//! Ctor
		ContextMenu(Rocket::Core::Context *context, InputManager *input, bool auto_hide = true);
		//! Dtor
		virtual ~ContextMenu();

		//! Sets the position of the ContextMenu
		void SetPosition(int x, int y);

		//! Shows the context menu
		virtual void Show();
		//! Sets the position of the context menu and shows it
		void Show(int x, int y, bool fit_within_context = true);

		//! Gets raw input, for additional functionality
		void OnRawInput(const RawInput &data);
		//! Processes messages from Rocket
		virtual void ProcessEvent(Rocket::Core::Event& ev);

		//! Registers the script interface to this class and the MenuItem class
		static void Register(asIScriptEngine *engine);

	protected:
		bool m_AutoHide;
		boost::signals2::connection m_RawInputConnection;
	};

}

#endif
