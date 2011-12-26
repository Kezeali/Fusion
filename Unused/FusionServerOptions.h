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

#ifndef Header_FusionEngine_ServerOptions
#define Header_FusionEngine_ServerOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Server ops.
	class ServerOptions
	{
	public:
		//! Constructor
		ServerOptions();
		//! Constructor +file
		ServerOptions(const std::string &filename);

	public:
		//! Max connections
		unsigned int mMaxClients;
		//! Maximum messages per second (how oftern to send states to a specific client)
		std::vector<ObjectID> mRate;
		//! Underlying send rate limiter (max time the net can take per step)
		//! \todo Evaluate the practicality and advantageousness of this
		unsigned int mNetDelay;
		//! The 'pixels per bit' (resolution) setting to use for bitmasks
		int mBitmaskResolution;

		//! Address of the fileserver
		std::string mFileServerHost;
		short mFileServerPort;
		
		//! Max FPS for server (value also sent to clients on connect)
		unsigned int mMaxFPS;

		//! The initial (maximum) health for ships
		int mMaxHealth;

		//! Saves to the most recently loaded file.
		bool Save();
		//! Saves the current options to a file
		bool SaveToFile(const std::string &filename);
		//! Loads a set of options from a file
		bool LoadFromFile(const std::string &filename);

	protected:
		//! Last opened options file
		std::string m_Filename;

	};

}

#endif