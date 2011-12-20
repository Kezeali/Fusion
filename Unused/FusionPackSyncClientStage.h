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

#ifndef Header_FusionEngine_PackSyncClientStage
#define Header_FusionEngine_PackSyncClientStage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionLoadingState.h"

#include <RakNet/RakPeerInterface.h>
#include <RakNet/FileListTransfer.h>
#include <RakNet/FileListTransferCBInterface.h>


namespace FusionEngine
{

	//! Syncs packages
	/*!
	 */
	class PackSyncClientStage : public LoadingStage, FileListTransferCBInterface
	{
	public:
		//! Constructor +connected server peer +options
		PackSyncClientStage(RakClientInterface* peer, ClientOptions* options);

		//! Destructor
		~PackSyncClientStage();

	public:
		//! Initialise
		bool Initialise();
		//! Update
		float Update(unsigned int split);
		//! CleanUp
		void CleanUp();

		//! Implementation of FileListTransferCBInterface#OnFile()
		void OnFile(
			unsigned fileIndex,
			char *filename,
			char *fileData,
			unsigned compressedTransmissionLength,
			unsigned finalDataLength,
			unsigned short setID,
			unsigned setCount,	
			unsigned setTotalCompressedTransmissionLength,
			unsigned setTotalFinalLength,
			unsigned char context);

	protected:
		//! Peer to make the main connection on
		RakClientInterface* m_MainConnection;
		//! Peer to make the file sync connection on
		RakClientInterface* m_FileConnection;

		//! File host to connect to
		std::string m_Host;
		//! Port on the file host
		unsigned short m_Port;

		//! Options
		ClientOptions *m_Options;

		//! Pack sync client
		PackSyncClient* m_PackSyncClient;


	};

}

#endif