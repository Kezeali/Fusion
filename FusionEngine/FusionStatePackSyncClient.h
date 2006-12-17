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

#ifndef Header_FusionEngine_PackSyncClientState
#define Header_FusionEngine_PackSyncClientState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionState.h"
#include "FileVerifier.h"

/// Fusion
#include "FusionPackSyncClient.h"
#include "FusionStateMessage.h"
#include "FusionError.h"

/// RakNet
#include <RakNet/RakNetworkFactory.h>
#include <RakNet/RakPeerInterface.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * State to be run when packages should be syncronised.
	 *
	 * PackSyncClientState runs a client Pack syncroniser. This class
	 * creats a new connection so it
	 * can connect to a seperate dedicated File sync server, and so
	 * as to not interfere with system/gameplay messages (though it usually
	 * runs before anything else, so this may be a non-issue...)
	 *
	 * \todo CRC verification.
	 */
	class PackSyncClientState : public FusionState
	{
	public:
		//! Basic constructor.
		PackSyncClientState(const std::string &host, const std::string &port);
		//! Deconstructor
		~PackSyncClientState();

	public:
		class DispFileReceived : public FileListTransferCBInterface
		{
		public:
			void OnFile(
				unsigned fileIndex,
				char *filename,
				unsigned char *fileData,
				unsigned compressedTransmissionLength,
				unsigned finalDataLength,
				unsigned short setID,
				unsigned setCount,	
				unsigned setTotalCompressedTransmissionLength,
				unsigned setTotalFinalLength)
			{
				char *buffer;
				sprintf(buffer, "%i. %i/%i %s %ib->%ib / %ib->%ib\n", setID, fileIndex, setCount, filename, compressedTransmissionLength, finalDataLength, setTotalCompressedTransmissionLength, setTotalFinalLength);
			}
		} m_TransferCallback;

	public:
		//! Initialises the link and GUI
		bool Initialise();
		//! Keeps checking for files
		bool Update(unsigned int split);
		//! Draws the GUI
		void Draw();
		//! Disconnects
		void CleanUp();

	protected:
		//! True if this is running as a server
		bool m_Server;
		//! The host to connect to
		std::string m_Host;
		//! The port to use
		std::string m_Port;

		//! The object that does the actual sync.
		PackSyncClient *m_Syncroniser;

		//! The network interface this is attached to.
		RakPeerInterface *m_Peer;

	};

}

#endif