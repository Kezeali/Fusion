/*
  Copyright (c) 2006 Elliot Hayward

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

#ifndef Header_FusionEngine_ServerOptions
#define Header_FusionEngine_ServerOptions

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	//! Server ops.
	class ServerOptions
	{
	public:
		//! Constructor
		ServerOptions();

		//! Max connections
		unsigned int MaxClients;
		//! Maximum messages per second (how oftern to send states to a specific client)
		std::vector<PlayerInd> MaxMessageRate;
		//! Underlying send rate limiter
		unsigned int NetDelay;
		//! The 'pixels per bit' (resolution) setting to use for bitmasks
		int BitmaskResolution;

	};

}

#endif