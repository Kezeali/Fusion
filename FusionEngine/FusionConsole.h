/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_Console
#define Header_FusionEngine_Console

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionError.h"

namespace FusionEngine
{

	//! Provides console data access to all FusionEngine objects
	/*!
	 * This class does not actually do anything with the data it contains,
	 * but instead uses callbacks to allow other classes to deal with console
	 * data. On that note, this is also not a script executor; the console
	 * GUI does that.
	 */
	class Console : public Singleton<Console>
	{
	public:
		//! Basic constructor
		Console();

	public:
		//! Lines in the console
		typedef std::vector<std::string> ConsoleLines;

	public:
		//! Adds the given message to the console
		void Add(const std::string &message);

		//! Adds the given Error to the console
		void Add(const Error *error);

		//! Removes all data from the console
		void Clear();

		//! Triggers when new data is added to the console
		CL_Signal_v1<const std::string &> OnNewLine;

		//! Triggers when the console is cleared
		CL_Signal_v0 OnClear;

	protected:
		//! Lists all the data which has been added to the console.
		ConsoleLines m_Data;

	};

}

#endif
