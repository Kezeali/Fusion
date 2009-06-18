/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptVector
#define Header_FusionEngine_ScriptVector

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine { namespace Scripting
{

	//! Reference counting Vector2 for AngelScript
	class ScriptVector
	{
	public:
		//! Copying constructor.
		ScriptVector(const ScriptVector &other);
		//! Init. constructor
		/*!
		 * Are these defaults necessary?
		 */
		ScriptVector(float x = 0.0, float y = 0.0);
		//! Init. constructor
		ScriptVector(const Vector2 &other);

	public:
		//! Increase reference count
		void AddRef();
		//! Decrease reference count. Deletes if refCount = 0
		void Release();

		//! Assign
		ScriptVector &operator=(const ScriptVector &other);
		//! Add-assign
		ScriptVector &operator+=(const ScriptVector &other);

		//! Actual vector
		Vector2 Data;

	protected:
		//! Destructor
		~ScriptVector();
		int m_RefCount;
	};

	//! Register ScriptVector using generic call methods
	/*!
	 * Call this function to register the string type
	 * using native calling conventions
	 */
	void RegisterScriptVector_Native(asIScriptEngine *engine);

	//! Register ScriptVector using native call methods
	/*!
	 * Use this one instead if native calling conventions
	 * are not supported on the target platform
	 */
	void RegisterScriptVector_Generic(asIScriptEngine *engine);

	//! Automaticaly register ScriptVector (using Native or Generic methods)
	/*!
	 * This function will determine the configuration of the engine
	 * and use one of the two functions below to register the string type
	 */
	void RegisterScriptVector(asIScriptEngine *engine);

}}

#endif
