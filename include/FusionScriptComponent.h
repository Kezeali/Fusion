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

#ifndef H_FusionScriptComponent
#define H_FusionScriptComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionThreadSafeProperty.h"

#include "FusionVector2.h"

namespace FusionEngine
{

	FSN_BEGIN_COIFACE(IScript)
	public:
		ThreadSafeProperty<std::string> ScriptPath;
		//ThreadSafeProperty<std::string> Script;

		FSN_COIFACE_CTOR(IScript,
			((FSN_GET_SET)(ScriptPath)) )

		static bool IsThreadSafe() { return true; }

	public:
		virtual const std::string& GetScriptPath() const = 0;
		virtual void SetScriptPath(const std::string& pos) = 0;
	};

}

#endif
