/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#ifndef Header_FusionEngine_ContextMenu
#define Header_FusionEngine_ContextMenu

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
		void *value;
	};

	class MenuItem : public Rocket::Core::EventListener, public RefCounted
	{
	public:
		MenuItem();
		MenuItem(const std::string &title, const std::string &value);
		virtual ~MenuItem();

		virtual void Show();
		void Hide();

		int AddChild(MenuItem *item);

		void RemoveChild(MenuItem *item);
		void RemoveChild(int index);

		void RemoveAllChildren();

		void GetChild(int index);

		void SelectChild(int index);
		void SelectChildRelative(int distance);

		int GetSelectedIndex() const;
		MenuItem *GetSelectedItem();

		virtual void ProcessEvent(Rocket::Core::Event& ev);

		virtual void EnumReferences(asIScriptEngine *engine);
		virtual void ReleaseAllReferences(asIScriptEngine *engine);

		void onHideTimeout();

		boost::signals2::signal<void (const MenuItemEvent&)> SignalClicked;

	protected:
		int m_Index;
		std::string m_Title;
		std::string m_Value;

		// The button for this menu item
		Rocket::Core::Element *m_Element;

		MenuItem *m_Parent;
		typedef EMP::Core::STL::vector<MenuItem*> MenuItemArray;
		MenuItemArray m_Children;

		int m_SelectedItem;

		CL_Timer m_HideTimer;

		Rocket::Core::ElementDocument *m_Document;
		Rocket::Core::Context *m_Context;

		bool isPseudoClassSetOnAnyChild(const EMP::Core::String &pseudo_class);

		void init();
		void initSubmenu();
		void addedToMenu(MenuItem *parent, Rocket::Core::Element *element);
	};

	//! Spawns a context-menu GUI document
	class ContextMenu : public MenuItem
	{
	public:
		ContextMenu(Rocket::Core::Context *context,  bool auto_hide = false);
		ContextMenu(Rocket::Core::Context *context, InputManager *input, bool auto_hide = true);
		virtual ~ContextMenu();

		void SetPosition(int x, int y);

		virtual void Show();
		void Show(int x, int y, bool fit_within_context = true);

		void OnRawInput(const RawInput &data);
		virtual void ProcessEvent(Rocket::Core::Event& ev);

		static void Register(asIScriptEngine *engine);

	protected:
		bool m_AutoHide;
		boost::signals2::connection m_RawInputConnection;
	};

}

#endif