/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionActiveEntityDirectory
#define H_FusionActiveEntityDirectory

#include "FusionPrerequisites.h"

#include "FusionTypes.h"
#include "FusionVector2.h"

#include <tbb/concurrent_hash_map.h>

namespace FusionEngine
{

	//! Stores active entity locations
	class ActiveEntityDirectory
	{
	public:
		ActiveEntityDirectory()
		{
		}

		void StoreEntityLocation(ObjectID id, const Vector2T<int32_t>& location)
		{
			m_Directory.insert(std::make_pair(id, location));
		}

		bool RetrieveEntityLocation(ObjectID id, Vector2T<int32_t>& location) const
		{
			Directory_t::const_accessor accessor;
			if (m_Directory.find(accessor, id))
			{
				location = accessor->second;
				return true;
			}
			else
				return false;
		}

		void DropEntityLocation(ObjectID id)
		{
			m_Directory.erase(id);
		}

	private:
		typedef tbb::concurrent_hash_map<ObjectID, Vector2T<int32_t>> Directory_t;
		Directory_t m_Directory;
	};

}

#endif
