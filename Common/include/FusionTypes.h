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

#ifndef H_FusionTypes
#define H_FusionTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <boost/intrusive_ptr.hpp>

#include "FusionPrerequisites.h"

namespace FusionEngine
{
	
	//! Unique identifier type for game objects
	typedef uint32_t ObjectID;
	//! Unique identifier type for players
	typedef uint8_t PlayerID;

	typedef std::vector<std::string> StringVector;
	typedef std::set<std::string> StringSet;

	typedef std::shared_ptr<Entity> EntityPtr;

	typedef boost::intrusive_ptr<EntityComponent> ComponentPtr;

	//! Log pointer
	typedef std::shared_ptr<Log> LogPtr;
	typedef std::shared_ptr<Module> ModulePtr;

	typedef unsigned char EntityDomain;

}

#endif
