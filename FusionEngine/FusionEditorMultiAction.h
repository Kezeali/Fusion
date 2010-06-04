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

#ifndef Header_FusionMultiUndoableAction
#define Header_FusionMultiUndoableAction

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"
#include "FusionEditorUndoAction.h"

namespace FusionEngine
{

	//! An action that is actually a collection of actions
	/*!
	* \remarks
	* As actions are added (using AddAction()) the title is regerated to
	* indicate what these actions are.
	*/
	class MultiAction : public UndoableAction
	{
	public:
		MultiAction();

		//! Adds the given action
		void AddAction(const UndoableActionPtr &action);

		const std::string &GetTitle() const { return m_Title; }

	protected:
		void undoAction();
		void redoAction();

		std::map<std::string, unsigned int> m_ActionTitles; // Count of each type of action
		std::string m_Title;

		std::vector<UndoableActionPtr> m_Actions;
	};

}

#endif