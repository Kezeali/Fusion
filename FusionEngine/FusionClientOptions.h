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

#ifndef Header_FusionEngine_ClientOptions
#define Header_FusionEngine_ClientOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionInputMap.h"

namespace FusionEngine
{
	//! Max players per client
	const unsigned int g_MaxPlayers = 4;

	//! Settings for network related stuff.
	class NetworkSettings
	{
	public:
		//! Constructor
		NetworkSettings();

	public:
		//! Maximum messages per second (how oftern to send / receive states)
		unsigned int MaxMessageRate;
		//! Sleep timer for network thread (for raknet)
		unsigned int NetDelay;
	};

	/*!
	 * Encapsulates client-side options.
	 */
	class ClientOptions
	{
	public:
		//! Constructor
		ClientOptions();

	public:
		//! Number of local players
		unsigned int NumPlayers;

		typedef std::vector<PlayerInputMap> PlayerInputMapList;

		//! Player input mappings
		PlayerInputMapList PlayerInputs;
		//! Global input mappings
		GlobalInputMap GlobalInputs;

		//! Nework options
		NetworkSettings NetworkOptions;

		//! Set the controls for defaults
		void DefaultPlayerControls(PlayerInd player);
		//! Sets all the controls to the defaults
		void DefaultGlobalControls();
	};

}

#endif
