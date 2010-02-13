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

#include "Common.h"

#include "FusionCmdOptions.h"

namespace FusionEngine
{

	CmdOptions::CmdOptions(int argc, char **argv)
	{
		// We can read in reverse here, which is faster for options but slower for 
		//  console (script) commands (as their order will have to be reversed.)
#  ifdef CMDOPTS_REVERSEREAD
		while (--argc > 0)
		{
			std::string argument( argv[argc] );
#  else
		for (int count=0; count<argc; count++)
		{
			std::string argument( argv[count] );
#  endif

			// If the first char is '+', this is to be executed as a console command
			if (argument[0] == '+')
			{
				m_Commands.push_back(argument.substr(1));
			}

			// otherwise, the argument is an option
			else
			{
				// look for " " or "="
				int flag_end;
				if ( (flag_end = argument.find(" ")) == std::string::npos )
					flag_end = argument.find("=");

				std::string flag = argument.substr(0, flag_end);
				std::string value = argument.substr(flag_end+1);

				m_Options.insert( OptionMap::value_type(flag, value) );
			}

		} // End for (int count=0; count<argc; count++)

		// -- If 'while (--argc > 0)' is used above --
#  ifdef CMDOPTS_REVERSEREAD
		// Reverse command list, as they should be executed in the same order they were
		//  mentioned in the cmd-line arguments (we read the args in reverse)
		std::reverse(m_Commands.begin(), m_Commands.end());
#  endif

	}

}