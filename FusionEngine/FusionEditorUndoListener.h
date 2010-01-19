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

#ifndef Header_FusionEngine_UndoListener
#define Header_FusionEngine_UndoListener

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"
#include "FusionEditorUndoAction.h"


namespace FusionEngine
{

	//! Receives update events from an undo manager
	class UndoListener
	{
	public:
		enum Direction {
			NONE, // Delete only the given index
			FORWARD, // Delete the given index and higher
			REVERSE // Delete the given index and lower
		};
		//virtual void OnSetMaxActions(unsigned int max) =0;
		virtual void OnActionAdd(const UndoableActionPtr &action) =0;
		virtual void OnActionRemove(unsigned int first, Direction direction) =0;
	};

}

#endif