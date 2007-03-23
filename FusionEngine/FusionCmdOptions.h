/*
  Copyright (c) 2006-2007 Fusion Project Team

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
#ifndef Header_FusionEngine_CmdOptions
#define Header_FusionEngine_CmdOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"




namespace FusionEngine
{

	//! Parses and stores options from the commandline
	class CmdOptions
	{
	public:
		typedef std::map<std::string, std::string> OptionMap;
		typedef std::deque<std::string> CommandQueue;

	public:
		//! Constructor +argc +argv
		CmdOptions(int argc, char **argv);
		//! Destructor
		~CmdOptions();

	public:
		//! Returns true if the given option key exists
		const bool OptionExists(const std::string& key) const;
		//! Gets the option paired with the given key
		const std::string& GetOption(const std::string& key) const;
		//! Converts the given option to an int and returns the result
		/*!
		 * If there is no such key, 0 is returned. <br>
		 */
		int GetOptionAsInt(const std::string& key) const;
		//! Converts the given option to a float and returns the result
		/*!
		 * If there is no such key, 0.0f is returned. <br>
		 */
		float GetOptionAsFloat(const std::string& key) const;
		//! Converts the given option to bool and returns the result
		/*!
		 * If there is no such key, false is returned. <br>
		 */
		bool GetOptionAsBool(const std::string& key) const;
		//! Returns the list of console command arguments
		StringVector GetCommandList() const;

	protected:
		// Option flags mapped to values
		OptionMap m_Options;
		// Console commands
		StringVector m_Commands;

	};

}

#endif