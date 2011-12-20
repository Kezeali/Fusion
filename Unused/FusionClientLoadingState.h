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

#ifndef Header_FusionEngine_ClientLoadingState
#define Header_FusionEngine_ClientLoadingState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionLoadingState.h"


#include <CEGUI/CEGUI.h>


namespace FusionEngine
{

	//! Syncs packages, loads data, shows gui.
	/*!
	 * \todo
	 * Some sort of FusionState based singleton wrapper for RakClientInterface and
	 * RakServerInterface which can create, store and destroy the peer from
	 * loading till quiting. This could wrap functions, or just allow access directly
	 * to the RakNet class in question. ClientLoadingState, ServerLoadingState,
	 * FusionNetworkClient, and FusionNetworkServer would all use one of these two
	 */
	class ClientLoadingState : public LoadingState
	{
	public:
		//! Constructor
		ClientLoadingState(RakPeerInterface* peer, const std::string& host, unsigned short port, ClientOptions* options);

		//! Destructor
		~ClientLoadingState();

	public:
		//! Initialise
		bool Initialise();
		//! Update
		bool Update(unsigned int split);
		//! Draw
		void Draw();
		//! CleanUp
		void CleanUp();

	protected:
		//! Peer to make the main connection on
		RakPeerInterface* m_MainConnection;

		//! Host to connect to
		std::string m_Host;
		//! Port on the host
		unsigned short m_Port;
		//! Options
		ClientOptions *m_Options;

		//! Pack sync client
		PackSyncClient* m_PackSyncClient;

		//! Progress bar for the GUI
		CEGUI::ProgressBar* m_ProgressBar;

	};

}

#endif