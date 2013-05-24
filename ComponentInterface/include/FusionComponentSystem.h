/*
*  Copyright (c) 2011-2013 Fusion Project Team
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

#ifndef H_FusionComponentSystem
#define H_FusionComponentSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSystemType.h"

#include <angelscript.h>
#include <memory>
#include <string>

namespace FusionEngine
{

	class SystemTaskBase;
	class SystemWorldBase;
	class IComponentSystem;

	typedef std::shared_ptr<SystemWorldBase> SystemWorldPtr;

	//! Component System
	class IComponentSystem
	{
	public:
		virtual ~IComponentSystem() {}

		virtual SystemType GetType() const = 0;

		virtual std::string GetName() const = 0;

		virtual void RegisterScriptInterface(asIScriptEngine* engine) {}

		virtual std::shared_ptr<SystemWorldBase> CreateWorld() = 0;
	};

}

#endif