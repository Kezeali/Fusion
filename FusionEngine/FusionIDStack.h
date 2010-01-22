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

#ifndef Header_FusionEngine_IdStack
#define Header_FusionEngine_IdStack

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	//! Supplies ObjectIDs which aren't assigned
	struct IDStack
	{
		ObjectID m_NextId;
		typedef std::deque<ObjectID> ObjectIDStack;
		// Lists IDs between 0 and m_NextId that have been freed by Entity removal
		ObjectIDStack m_UnusedIds;

		//! Initialises m_NextId to one (Entity IDs start at 1)
		IDStack()
			: m_NextId(1)
		{}

		//! Returns an ObjectID which is not in use
		ObjectID getFreeID();
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		void freeID(ObjectID id);
		//! Resets this object
		void freeAll();
	};

}

#endif
