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

#include "FusionStableHeaders.h"

#include "FusionEditorUndo.h"


namespace FusionEngine
{

	UndoableActionManager::UndoableActionManager(unsigned int cap)
		: m_CurrentAction(-1)
	{
		FSN_ASSERT(cap > 0);
		m_Actions.set_capacity(cap);
	}

	void UndoableActionManager::SetMaxActions(unsigned int capacity)
	{
		FSN_ASSERT(capacity > 0);

		// Update the current action index if actions are going to be removed
		if (m_Actions.size() > capacity)
			m_CurrentAction -= m_Actions.capacity() - capacity;

		m_Actions.rset_capacity(capacity);
	}

	void UndoableActionManager::Add(const UndoableActionPtr &action)
	{
		// Erase undone actions (m_CurrentAction is the most recent action that hasn't been undone)
		if (!m_Actions.empty() && m_CurrentAction < (signed)m_Actions.size()-1)
		{
			m_Actions.erase(m_Actions.begin()+(m_CurrentAction+1), m_Actions.end());
		}
		// Remove all redo items
		invokeActionRemoveAll(m_RedoListeners);

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
			invokeActionRemoveAll(m_RedoListeners);
		}
		else
			invokeActionRemove(m_RedoListeners, action);

		m_Actions.erase(m_Actions.begin()+action, m_Actions.end());

		//if (action > 0)
			m_CurrentAction = action-1;
		//else
		//	m_CurrentAction = 0;
	}

	void UndoableActionManager::Clear()
	{
		invokeActionRemoveAll(m_UndoListeners);
		invokeActionRemoveAll(m_RedoListeners);

		m_Actions.clear();
		m_CurrentAction = -1;
	}

	void UndoableActionManager::Undo(unsigned int action_index)
	{
		FSN_ASSERT_MSG(action_index < m_Actions.size(), "Given index does not exist");

		if (m_CurrentAction < 0)
			return; // nothing to undo

		// The given action onward are no longer in the undo list
		invokeActionRemove(m_UndoListeners, action_index);

		// Call undo on each action taken since the given one, and add the actions to the redo list
		for (UndoableActionBuffer::size_type i = m_CurrentAction+1; i-- > action_index;)
		{
			UndoableActionPtr &action = m_Actions[i];
			action->Undo();
			invokeActionAdd(m_RedoListeners, action);
		}

		// Update the current action index
		//m_CurrentAction = (action_index > 0 ? (action_index-1) : 0);
		m_CurrentAction = action_index-1;
	}

	void UndoableActionManager::Undo()
	{
		if (m_Actions.empty() || m_CurrentAction < 0)
			return;
		// Remove the current action from the undo list
		invokeActionRemove(m_UndoListeners, m_CurrentAction);

		// Call undo, and add the action to the redo list
		UndoableActionPtr &action = m_Actions[m_CurrentAction];
		action->Undo();
		invokeActionAdd(m_RedoListeners, action);

		// Update the current action index
		//if (m_CurrentAction != 0)
			--m_CurrentAction;
		//else
		//	m_CurrentAction = 0;
	}

	void UndoableActionManager::Redo(unsigned int redo_index)
	{
		// Translate the redo-index to the actual index within the unified buffer
		unsigned int action_index = redo_index + m_CurrentAction+1;

		FSN_ASSERT_MSG(action_index < m_Actions.size(), "Given index does not exist");

		// The given action and earlier are no longer in the redo list
		invokeActionRemoveRawIndex(m_RedoListeners, redo_index, UndoListener::REVERSE);

		// Call redo on each undone action leading up to the given one, and add the actions to the redo list
		for (UndoableActionBuffer::size_type i = m_CurrentAction+1; i <= action_index; ++i)
		{
			UndoableActionPtr &action = m_Actions[i];
			action->Redo();
			invokeActionAdd(m_UndoListeners, action);
		}

		// Update the current action index
		m_CurrentAction = action_index;
	}

	void UndoableActionManager::Redo()
	{
		if (m_Actions.empty())
			return;

		if (m_CurrentAction == m_Actions.size()-1)
			return; // there are no undone actions

		// The given action and earlier are no longer in the redo list
		invokeActionRemove(m_RedoListeners, m_CurrentAction+1, UndoListener::REVERSE);

		UndoableActionPtr &action = m_Actions[m_CurrentAction+1];
		action->Redo();
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

	void UndoableActionManager::invokeActionAdd(const UndoableActionManager::UndoListenerList &list, const UndoableActionPtr &action)
	{
		for (UndoListenerList::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
		{
			UndoListener *listener = *it;
			listener->OnActionAdd(action, list == m_UndoListeners); // Tells undo-listeners to add to the back, redo-listeners to add to the front
		}
	}

	void UndoableActionManager::invokeActionRemove(const UndoableActionManager::UndoListenerList &list, unsigned int first, UndoListener::Direction direction)
	{
		if (list == m_RedoListeners)
		{
			if ((signed)first <= m_CurrentAction)
				return;
			first -= m_CurrentAction+1;
		}
		invokeActionRemoveRawIndex(list, first, direction);
	}

	void UndoableActionManager::invokeActionRemoveRawIndex(const UndoableActionManager::UndoListenerList &list, unsigned int first, UndoListener::Direction direction)
	{
		for (UndoListenerList::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
		{
			UndoListener *listener = *it;
			listener->OnActionRemove(first, direction);
		}
	}

	void UndoableActionManager::invokeActionRemoveAll(const UndoableActionManager::UndoListenerList &list)
	{
		for (UndoListenerList::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
		{
			UndoListener *listener = *it;
			listener->OnActionRemove(0, UndoListener::FORWARD);
		}
	}

}
