/*
  Copyright (c) 2010 Fusion Project Team

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

#ifndef Header_FusionEngine_Undo
#define Header_FusionEngine_Undo

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	class IUndoableAction;
	typedef std::tr1::shared_ptr<IUndoableAction> UndoableActionPtr;

	class UndoableActionQueue;

	//! Item in the undo list
	/*!
	* Implemented as a simple linked list, altho each item in the undo menu holds
	* a pointer to an action so the list only needs to be traversed when actually
	* undoing / redoing actions.
	*/
	class IUndoableAction
	{
		friend class UndoableActionQueue;
	public:
		//! Calls the implementation's undoAction() on this and all subsequent items in the list
		void Undo();
		//! Calls the implementation's redoAction() on this and all subsequent items in the list
		/*!
		* Obvously this can only be called if Undo() has been called on this or a previous action
		*/
		void Redo();

		bool IsUndone() const;

		// Linked list stuff:
		//! Removes this and all following items from the list
		void Remove();

	protected:
		//! Implentation should undo the action that this object represents
		/*!
		* The implementation can assume that the current state is
		* the same as it was just after this object was created
		* (the IUndoableAction#Undo() method should ensure this.)
		*/
		virtual void undoAction() =0;
		//! Implentation should undo the action that this object represents
		/*!
		* The implementation can assume that the current state is
		* the same as it was just before this object was created
		*/
		virtual void redoAction() =0;

		// Didn't use pseudo-hungarian notation for these (m_...) because I think it reads better for linked list vars
		//  In fact, phung (as I call it among friends) doesn't read very well anywhere, I just like knowing when
		//  vars are local / member on sight and this is how I've gotten used to doing it - i.e.
		//  camelCase, with first char lowercase, for local; underscore_spaces for function parameters; and m_PToTheH for 
		//  member vars.
		//  Thank you for your time.
		// Yours,
		// Elliot Hayward
		std::tr1::weak_ptr<IUndoableAction> previous;
		UndoableActionPtr next;
		// Set to true when Undo() is called on this object
		bool m_Undone;
	};

	//! A simple container - list of undoable actions
	class UndoableActionQueue
	{
	public:
		void SetMaxLength(unsigned int length);
		void PushBack(const UndoableActionPtr &action);
		void Clear();

	protected:
		void eraseUndoneActions();

		unsigned int m_MaxLength;
		unsigned int m_Length;

		UndoableActionPtr m_OldestAction;
		UndoableActionPtr m_NewestAction;
	};

}

#endif