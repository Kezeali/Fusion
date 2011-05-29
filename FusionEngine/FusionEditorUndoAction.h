/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionEditorUndoAction
#define H_FusionEditorUndoAction

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	class UndoableAction;
	typedef std::tr1::shared_ptr<UndoableAction> UndoableActionPtr;

	class UndoableActionManager;

	//! Item in the undo list
	/*!
	* Implemented as a simple linked list, altho each item in the undo menu holds
	* a pointer to an action so the list only needs to be traversed when actually
	* undoing / redoing actions.
	*/
	class UndoableAction
	{
		friend class UndoableActionManager;
	public:
		UndoableAction();

		virtual const std::string &GetTitle() const = 0;

		//! Calls the implementation's undoAction() and marks this action as undone
		void Undo();
		//! Calls the implementation's redoAction() and removes the UNDONE flag
		void Redo();

		bool IsUndone() const;

		// Linked list stuff:
		//! Removes this and all following items from the list
		//void Remove();

		//static void Add(UndoableActionQueue &queue, const UndoableActionPtr &action);
		//static void Remove(UndoableActionQueue &queue, UndoableActionQueue::size_type index);

	protected:
		//! Implentation should undo the action that this object represents
		/*!
		* The implementation can assume that the current state is
		* the same as it was just after this object was created
		* (the UndoableAction#Undo() method should ensure this.)
		*/
		virtual void undoAction() =0;
		//! Implentation should undo the action that this object represents
		/*!
		* The implementation can assume that the current state is
		* the same as it was just before this object was created
		*/
		virtual void redoAction() =0;

		//static void eraseUndoneActions();

		// Didn't use pseudo-hungarian notation for this (m_...) because I think it reads better for linked list vars
		//  In fact, phung (as I call it among friends) doesn't read very well anywhere, I just like knowing when
		//  vars are local / member on sight and this is how I've gotten used to doing it - i.e.
		//  camelCase, with first char lowercase, for local; underscore_spaces for function parameters; and m_PToTheH for 
		//  member vars.
		//  Thank you for your time.
		// Yours,
		// Elliot Hayward
		UndoableActionPtr next;
		// Set to true when Undo() is called on this object
		bool m_Undone;
	};

}

#endif