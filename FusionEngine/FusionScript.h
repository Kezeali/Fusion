/*
  Copyright (c) 2006 Fusion Project Team

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
*/

#ifndef Header_FusionEngine_ScriptingEngine
#define Header_FusionEngine_ScriptingEngine

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion

namespace FusionEngine
{

	/*!
	 * \brief
	 * Provides scripting for FusionEngine objects.
	 *
	 * \sa
	 * ScriptingEngine
	 */
	class Script
	{
	public:
		//! Basic constructor.
		Script() {}
		//! Constructor +filename
		Script(const std::string &filename)
		{
			LoadFile(filename);
		}

	public:
		//! Retuns the module this script was assigned to by the ScriptingEngine.
		char *GetModule() const;

		//! Loads and Initialises this Script from a file
		bool LoadFile(const std::string &filename);

		//! Returns the ptr. returned by the script (if applicable)
		void *GetReturnObject() const;

		//! Sets the module ID.
		/*!
		 * Allows the ScriptingEngine to store the module ID the which this script
		 * has been compiled into.
		 */
		void _setModule(char *module);

		//! Sets m_Registered to true.
		/*!
		 * Allows the ScriptingEngine notify a script that it has been registered 
		 * (i.e. added to a module.)
		 * Registers registration, if-you-will. :)
		 */
		void _notifyRegistration();

	private:
		//! Unique module ID.
		/*!
		 * The unique ID for the module in which this script resides.
		 */
		char *m_Module;

		//! Stores the raw script.
		std::string m_Script;

		//! Total character length of the script.
		size_t m_Length;

		//! True if the last load attempt completed successfully.
		bool m_Loaded;

		//! True if this Script has been registered.
		bool m_Registered;

		//! Stores the returned object.
		void *m_RetObj;

	};

}

#endif
