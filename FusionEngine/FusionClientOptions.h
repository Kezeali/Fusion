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

#ifndef Header_FusionEngine_ClientOptions
#define Header_FusionEngine_ClientOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionInputMap.h"

namespace FusionEngine
{
	//! Max local (split-screen) players per client
	static const unsigned int g_MaxPlayers = 4;

	//! Settings specfic to each player
	class PlayerOptions
	{
	public:
		//! Constructor.
		PlayerOptions()
			: mHUD(true)
		{}

		//! Display the HUD for this player
		bool mHUD;
	};

	//! Settings for network related stuff.
	class NetworkSettings
	{
	public:
		//! Constructor
		NetworkSettings()
			: mRate(100),
			mLocalPort(1337)
		{
		}

		//! Maximum messages per second
		unsigned int mRate;

		//! The local port (to connect from)
		unsigned short mLocalPort;

	};

	/*!
	 * \brief
	 * Encapsulates client-side options.
	 */
	class ClientOptions
	{
	public:
		//! Constructor
		ClientOptions();
		//! Constructor +file
		ClientOptions(const std::string &filename);

	public:
		//! General Player options list
		typedef std::vector<PlayerOptions> PlayerOptionsList;
		//! Player Input mappings list
		typedef std::vector<PlayerInputMap> PlayerInputsList;

	public:
		//! Number of local players
		unsigned int mNumPlayers;

		//! True if console history should be logged.
		bool mConsoleLogging;

		//! General player options (other than inputs)
		PlayerOptionsList mPlayerOptions;

		//! Player input mappings
		PlayerInputsList mPlayerInputs;
		//! Global input mappings
		GlobalInputMap mGlobalInputs;

		//! Nework options
		NetworkSettings mNetworkOptions;

		//! Set the controls for defaults
		void DefaultPlayerControls(ObjectID player);
		//! Sets all the controls to the defaults
		void DefaultGlobalControls();

		//! Saves to the most recently loaded file.
		bool Save();
		//! Saves the current options to a file
		bool SaveToFile(const std::string &filename);
		//! Loads a set of options from a file
		bool LoadFromFile(const std::string &filename);

	protected:
		//! Last opened options file
		std::string m_LastFile;

	};

}

#endif
