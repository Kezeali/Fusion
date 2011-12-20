/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEditorMultiAction.h"

namespace FusionEngine
{

	MultiAction::MultiAction()
	{
	}

	void MultiAction::AddAction(const UndoableActionPtr& action)
	{
		m_Actions.push_back(action);

		// Add this action to the title count
		const std::string& title = action->GetTitle();
		std::string verb = title.substr(0, title.find(' ')); // The verb describing the action is usually the first word of the title
		if (!verb.empty())
			++m_ActionTitles[verb];
		// Update the title (a summary of how many actions of each type are in this container)
		std::stringstream titleStream;
		for (auto it = m_ActionTitles.begin(), end = m_ActionTitles.end(); it != end; ++it)
		{
			titleStream << it->first << " (" << it->second << ") ";
		}
		m_Title = titleStream.str();
	}

	void MultiAction::undoAction()
	{
		std::for_each(m_Actions.rbegin(), m_Actions.rend(), [](UndoableActionPtr& action) { action->Undo(); });
	}

	void MultiAction::redoAction()
	{
		std::for_each(m_Actions.begin(), m_Actions.end(), [](UndoableActionPtr& action) { action->Redo(); });
	}

}
