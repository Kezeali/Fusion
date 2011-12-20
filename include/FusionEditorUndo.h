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

#ifndef H_FusionEditorUndo
#define H_FusionEditorUndo

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"
#include <boost/circular_buffer.hpp>

#include "FusionEditorUndoAction.h"
#include "FusionEditorUndoListener.h"


namespace FusionEngine
{

	//! Add, undo, redo actions
	class UndoableActionManager
	{
	public:
		//! Constructor
		/*!
		* \param[in] capacity Sets the maximum number of undoable actions to store
		*/
		UndoableActionManager(unsigned int capacity);

		//! Changes the capacity of the container
		void SetMaxActions(unsigned int capacity);
		//! Adds a new action
		/*!
		* All actions subsequent to the most recent action that hasn't been undone
		* (m_CurrentAction) will be dropped.
		*/
		void Add(const UndoableActionPtr &action);
		//! Removes the action at the given index
		void Remove(unsigned int action);
		//! Removes all actions in the container (clears the undo list).
		void Clear();

		//! Calls the undo method of the action at the given index
		/*!
		* Action 0 is the oldest action still on the buffer, higher action indexes are more recent.
		*/
		void Undo(unsigned int action);
		//! Calls Undo(m_CurrentActions) to undo the most recent action
		void Undo();
		//! Calls the redo method of the action at the given index
		/*!
		* Action 0 is the action most recently added to the list, and is the oldest action in the list.
		* In other words, the redoable actions are still indexed by their actual age as actions, not by their
		* age as undone actions.
		*/
		void Redo(unsigned int action);
		//! Calls Redo(m_CurrentAction+1) to redo the last undone action
		void Redo();

		//! Attaches an Undo (or redo) listener (e.g. an undo menu GUI element.)
		void AttachListener(UndoListener *listener, bool undo);
		//! Detach
		void DetachListener(UndoListener *listener, bool undo);

		void DetachAllListeners();

	protected:
		typedef boost::circular_buffer<UndoableActionPtr> UndoableActionBuffer;

		UndoableActionBuffer m_Actions;
		// The current action is the most recent action that hasn't been undone
		/*UndoableActionBuffer::size_type*/int m_CurrentAction;

		typedef std::list<UndoListener*> UndoListenerList;
		UndoListenerList m_UndoListeners;
		UndoListenerList m_RedoListeners;

		//! Adds the given action to the given list of listeners
		void invokeActionAdd(const UndoListenerList &list, const UndoableActionPtr &action);
		//! Removes the indicated indexes from the given list
		/*!
		* If the given list is m_RedoListeners, the 'first' value will be adjusted accordingly (based on m_CurrentAction)
		*/
		void invokeActionRemove(const UndoListenerList &list, unsigned int first, UndoListener::Direction direction = UndoListener::FORWARD);
		void invokeActionRemoveRawIndex(const UndoListenerList &list, unsigned int first, UndoListener::Direction direction = UndoListener::FORWARD);

		//! Clears the given list
		void invokeActionRemoveAll(const UndoListenerList &list);
	};

}

#endif