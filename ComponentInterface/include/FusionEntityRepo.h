/*
*  Copyright (c) 2011 Fusion Project Team
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

#ifndef H_FusionEntityRepo
#define H_FusionEntityRepo

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionTypes.h"

namespace FusionEngine
{

	//! Entity Repository interface (implemented by EngineFramework::EntityManager)
	class EntityRepo
	{
	public:
		virtual ~EntityRepo() {}

		virtual EntityPtr GetEntity(ObjectID id) const = 0;
		virtual EntityPtr GetEntity(ObjectID id, bool load) const = 0;

		virtual uint32_t StoreReference(ObjectID from, ObjectID to) = 0;
		virtual ObjectID RetrieveReference(uint32_t token) = 0;
		virtual void DropReference(uint32_t token) = 0;
	};

}

#endif
