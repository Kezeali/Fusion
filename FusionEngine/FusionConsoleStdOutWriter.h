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

#ifndef Header_FusionEngine_ConsoleStdOutWriter
#define Header_FusionEngine_ConsoleStdOutWriter

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionConsole.h"

namespace FusionEngine
{

	//! Writes Console lines to cout.
	class ConsoleStdOutWriter
	{
	public:
		//! Constructor
		ConsoleStdOutWriter();
		//! Constructor + console
		ConsoleStdOutWriter(Console *console);

		//! Destructor
		~ConsoleStdOutWriter();

	public:
		//! Call to connect to either a given console, or the Console singleton
		void Activate();
		//! Disconnects from the console
		void Disable();

		//! Called by the OnNewLine signal from the console
		void onConsoleNewline(const std::wstring &message);

	protected:
		//! True if this object is connected to a Console
		bool m_Active;

		CL_Slot m_ConsoleOnNewLineSlot;

	};

}

#endif
