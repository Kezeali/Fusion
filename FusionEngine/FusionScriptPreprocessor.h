/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptPreprocessor
#define Header_FusionEngine_ScriptPreprocessor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{

	//! Base class for scipt preprocessors
	class ScriptPreprocessor
	{
	public:
		struct ScriptMarker
		{
			ScriptMarker() : pos(0), line(0), column(0), type(0) {}

			std::string::size_type pos;

			unsigned int line;
			std::string::size_type column;

			char type;
		};
		typedef std::vector<ScriptMarker> MarkedLines;

	public:
		//! Implementation should process the given code
		/*!
		* Preprocessors should modify the code given in the code
		* param and optionally update the lines param (to remove processed lines).
		*
		* \param[in|out] code
		* The script code to modify
		*
		* \param[in|out]
		* The lines with known pre-processor markers. If a Preprocessor implementation
		* uses this list it is good practice for that Preprocessor remove entries for
		* lines that it removes during processing (since they will be of no use to subsequent
		* processors.
		*/
		virtual void Process(std::string &code, const char *module_name, MarkedLines &lines) =0;

		static void checkForMarkedLines(MarkedLines &lines, const std::string &code)
		{
			//std::string::size_type pos = 0; // Character position within the file
			//unsigned int line = 0, col = 0; // line is used to count newlines incountered, col stores the current char. within a line
			ScriptMarker marker;
			while (marker.pos < code.length())
			{
				if (code[marker.pos] == '#')
				{
					marker.type = '#';
					lines.push_back(marker);
				}

				else if (code[marker.pos] == '\n')
				{
					marker.line++;
					marker.column = 0;
				}

				marker.pos++;
				marker.column++;
			}
		}
	};

}

#endif
