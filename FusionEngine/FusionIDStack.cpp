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

#include "FusionIDStack.h"


namespace FusionEngine
{

	ObjectID IDStack::getFreeID()
	{
		if (m_UnusedIds.empty())
			return m_NextId++;
		else
		{
			ObjectID id = m_UnusedIds.back();
			m_UnusedIds.pop_back();
			return id;
		}
	}

	void IDStack::freeID(ObjectID id)
	{
		if (id < m_NextId-1)
			m_UnusedIds.push_back(id); // record unused ID
		else
			--m_NextId;
	}

	void IDStack::freeAll()
	{
		m_UnusedIds.clear();
		m_NextId = 1; // Entity IDs start at 1, 0 indicates a pseudo-entity
	}

}
