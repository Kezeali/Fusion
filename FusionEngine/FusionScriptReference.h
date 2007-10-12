/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptReference
#define Header_FusionEngine_ScriptReference

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Stores data needed to execute a script function
	/*!
	 * \sa Script | ScriptManager
	 */
	class ScriptReference
	{
	public:
		//! Constructor
		ScriptReference(const char* module, ScriptFuncSig signature);
		//! Constructor
		ScriptReference(const char* module, ScriptFuncSig signature, int funcID);

	public:
		const char* GetModule() const;
		ScriptFuncSig GetSignature() const;
		int GetFunctionID() const;

	protected:
		const char* m_Module;
		ScriptFuncSig m_Signature;
		int m_FunctionID;
	};

}

#endif