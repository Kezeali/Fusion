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

#include "Common.h"

#include "FusionEditorUndo.h"


namespace FusionEngine
{

	void UndoableActionManager::SetMaxActions(unsigned int capacity)
	{
		FSN_ASSERT(capacity > 0);

		// Update the current action index if actions are going to be removed
		if (m_Actions.size() > capacity)
			m_CurrentAction -= m_Actions.capacity() - capacity;

		m_Actions.rset_capacity(capacity);

		// inform listeners
		//for (UndoListenerList::const_iterator it = m_UndoListeners.begin(), end = m_UndoListeners.end(); it != end; ++it)
		//{
		//	const UndoListener *listener = *it;
		//	listener->OnSetMaxActions(capacity);
		//}
		//for (UndoListenerList::const_iterator it = m_RedoListeners.begin(), end = m_RedoListeners.end(); it != end; ++it)
		//{
		//	const UndoListener *listener = *it;
		//	listener->OnSetMaxActions(capacity);
		//}
	}

	void UndoableActionManager::Add(const UndoableActionPtr &action)
	{
		// Erase undone actions (m_CurrentAction is the most recent action that hasn't been undone)
		if (m_CurrentAction != m_Actions.size()-1)
		{
			m_Actions.erase(m_Actions.begin()+m_CurrentAction+1);
		}
		// Remove all redo items
		invokeActionRemove(m_RedoListeners, 0);

		// Remove the oldest undo action from the menu if it will be dropped out of the buffer
		if (m_Actions.size() == m_Actions.capacity())
			invokeActionRemove(m_UndoListeners, 0, UndoListener::NONE);

		m_Actions.push_back(action);
		m_CurrentAction = m_Actions.size()-1;

		// Add the new action to the undo menu
		invokeActionAdd(m_UndoListeners, action);
	}

	void UndoableActionManager::Remove(unsigned int action)
	{
		FSN_ASSERT_MSG(action < m_Actions.size(), "Given index does not exist");

		if (!m_Actions[action]->IsUndone())
		{
			invokeActionRemove(m_UndoListeners, action);
			invokeActionRemove(m_RedoListeners, 0);
		}
		else
			invokeActionRemove(m_RedoListeners, action);

		m_Actions.erase(m_Actions.begin()+action, m_Actions.end());

		if (action > 0)
			m_CurrentAction = action-1;
		else
			m_CurrentAction = 0;
	}

	void UndoableActionManager::Clear()
	{
		invokeActionRemove(m_UndoListeners, 0);
		invokeActionRemove(m_RedoListeners, 0);

		m_Actions.clear();
		m_CurrentAction = 0;
	}

	void UndoableActionManager::Undo(unsigned int action)
	{
		FSN_ASSERT_MSG(action < m_Actions.size(), "Given index does not exist");

		// The given action onward are no longer in the undo list
		invokeActionRemove(m_UndoListeners, action);

		// Call undo, and add the actions to the redo list
		for (UndoableActionBuffer::size_type i = m_Actions.size()-1; i >= action; --i)
		{
			UndoableActionPtr &action = m_Actions[i];
			action->Undo();
			invokeActionAdd(m_RedoListeners, action);
		}

		// Update the current action index
		m_CurrentAction = (action > 0 ? (action-1) : 0);
	}

	void UndoableActionManager::Undo()
	{
		FSN_ASSERT_MSG(!m_Actions.empty(), "Given index does not exist");
		// Remove the current action from the undo list
		invokeActionRemove(m_UndoListeners, m_CurrentAction);

		// Call undo, and add the action to the redo list
		UndoableActionPtr &action = m_Actions[m_CurrentAction];
		action->Undo();
		invokeActionAdd(m_RedoListeners, action);

		// Update the current action index
		if (m_CurrentAction != 0)
			--m_CurrentAction;
		else
			m_CurrentAction = 0;
	}

	void UndoableActionManager::Redo(unsigned int action)
	{
		FSN_ASSERT_MSG(action < m_Actions.size(), "Given index does not exist");

		if (action <= m_CurrentAction)
			return; // the action hasn't been undone

		// The given action and earlier are no longer in the redo list
		invokeActionRemove(m_RedoListeners, action, UndoListener::REVERSE);

		// Call redo, and add the actions to the redo list
		for (UndoableActionBuffer::size_type i = m_Actions.size()-1; i >= action; --i)
		{
			UndoableActionPtr &action = m_Actions[i];
			action->Undo();
			invokeActionAdd(m_RedoListeners, action);
		}

		// Update the current action index
		m_CurrentAction = (action < m_Actions.size()-1 ? (action+1) : m_Actions.size()-1);
	}

	void UndoableActionManager::Redo()
	{
		FSN_ASSERT_MSG(m_Actions.empty(), "Given index does not exist");

		if (m_CurrentAction == m_Actions.size()-1)
			return; // the action hasn't been undone

		// The given action and earlier are no longer in the redo list
		invokeActionRemove(m_RedoListeners, m_CurrentAction, UndoListener::REVERSE);

		UndoableActionPtr &action = m_Actions[m_CurrentAction];
		action->Undo();
		invokeActionAdd(m_RedoListeners, action);

		// Update the current action index
		++m_CurrentAction;
	}

	void UndoableActionManager::AttachListener(FusionEngine::UndoListener *listener, bool undo)
	{
		if (undo)
			m_UndoListeners.push_back(listener);
		else
			m_RedoListeners.push_back(listener);

		//listener->OnSetMaxActions(m_Actions.capacity())
	}

	void UndoableActionManager::DetachListener(FusionEngine::UndoListener *listener, bool undo)
	{
		if (undo)
		{
			for (UndoListenerList::iterator it = m_UndoListeners.begin(), end = m_UndoListeners.end(); it != end; ++it)
			{
				if (*it == listener)
				{
					m_UndoListeners.erase(it);
					break;
				}
			}
		}

		else
		{
			for (UndoListenerList::iterator it = m_RedoListeners.begin(), end = m_RedoListeners.end(); it != end; ++it)
			{
				if (*it == listener)
				{
					m_RedoListeners.erase(it);
					break;
				}
			}
		}
	}

	void UndoableActionManager::DetachAllListeners()
	{
		//for (UndoListenerList::iterator it = m_UndoListeners.begin(), end = m_UndoListeners.end(); it != end; ++it)
		//{
		//	UndoListener *listener = *it;
		//	listener->OnDetach();
		//}
		//for (UndoListenerList::iterator it = m_RedoListeners.begin(), end = m_RedoListeners.end(); it != end; ++it)
		//{
		//	UndoListener *listener = *it;
		//	listener->OnDetach();
		//}
		m_UndoListeners.clear();
		m_RedoListeners.clear();
	}

	void UndoableActionManager::invokeActionAdd(const FusionEngine::UndoableActionManager::UndoListenerList &list, const UndoableActionPtr &action)
	{
		for (UndoListenerList::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
		{
			UndoListener *listener = *it;
			listener->OnActionAdd(action);
		}
	}

	void UndoableActionManager::invokeActionRemove(const FusionEngine::UndoableActionManager::UndoListenerList &list, unsigned int first, UndoListener::Direction direction)
	{
		if (list == m_RedoListeners)
		{
			FSN_ASSERT(first > m_CurrentAction);
			first -= m_CurrentAction+1;
		}

		for (UndoListenerList::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
		{
			UndoListener *listener = *it;
			listener->OnActionRemove(first, direction);
		}
	}

}
