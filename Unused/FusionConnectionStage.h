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

#ifndef Header_FusionEngine_ConnectionStage
#define Header_FusionEngine_ConnectionStage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionLoadingStage.h"

#include "FusionLoadingException.h"

#include <RakNet/RakPeerInterface.h>


namespace FusionEngine
{

	//! Could be useful...
	int g_DefaultTimeout = 2000;

	//! Connects the given peer to the given server
	class ConnectionStage : public LoadingStage
	{
	public:
		//! Stages during loading
		enum CLSStage { CS_STARTUP = 0, CS_CONNECTING = 50, CS_DONE = 100 };

	public:
		//! Constructor
		ConnectionStage(RakPeerInterface* peer, const char* address, unsigned short port, unsigned short localPort, unsigned short maxConnections, int threadSleep, int timeout);

		//! Destructor
		~ConnectionStage();

	public:
		//! Initialise
		bool Initialise();
		//! Update
		float Update(unsigned int split);
		//! CleanUp
		void CleanUp();

		//! Sets the connection timeout
		void SetTimeout(int time);
		//! Returns connection timeout
		int GetTimeout() const;

	protected:
		RakPeerInterface* m_Connection;
		const char* m_Address;
		unsigned short m_RemotePort;
		unsigned short m_LocalPort;
		unsigned short m_MaxConnections;
		int m_ThreadSleep;
		CLSStage m_Stage;
		int m_RunningTime;
		int m_Timeout;

	};

}

#endif