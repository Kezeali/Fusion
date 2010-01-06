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

	void IUndoableAction::Undo()
	{
		if (next)
			next->Undo();
		undoAction();
		m_Undone = true;
	}

	void IUndoableAction::Redo()
	{
		if (!IsUndone()) // Can't redo an action that hasn't been undone
			return;

		UndoableActionPtr lockedPrev = previous.lock();
		if (lockedPrev && lockedPrev->IsUndone()) // Find the most distant action that hasn't been undone
			lockedPrev->Redo();
		redoAction();

		m_Undone = false;
	}

	bool IUndoableAction::IsUndone() const
	{
		return m_Undone;
	}

	void IUndoableAction::Remove()
	{
		UndoableActionPtr lprevious = previous.lock();
		if (lprevious)
			lprevious->next.reset();
		if (next)
			next->previous.reset();
	}

	void UndoableActionQueue::SetMaxLength(unsigned int length)
	{
		FSN_ASSERT(length > 0);

		while (m_Length > length)
		{
			m_OldestAction = m_OldestAction->next;
			--m_Length;
		}
		m_MaxLength = length;
	}

	void UndoableActionQueue::PushBack(const UndoableActionPtr &action)
	{
		eraseUndoneActions();

		if (!m_OldestAction)
			m_OldestAction = action;

		if (m_NewestAction)
		{
			m_NewestAction->next = action;
			action->previous = m_NewestAction;
		}
		m_NewestAction = action;

		++m_Length;

		while (m_Length > m_MaxLength)
		{
			m_OldestAction = m_OldestAction->next;
			--m_Length;
		}
	}

	void UndoableActionQueue::Clear()
	{
		m_OldestAction.reset();
		m_NewestAction.reset();
	}

	void UndoableActionQueue::eraseUndoneActions()
	{
		if (m_NewestAction)
			while (m_NewestAction->IsUndone())
			{
				m_NewestAction = m_NewestAction->previous.lock();
				if (!m_NewestAction)
				{
					m_OldestAction.reset();
					break;
				}
			}
	}

}
