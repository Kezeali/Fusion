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

#ifndef Header_FusionEngine_FusionPackSyncClient
#define Header_FusionEngine_FusionPackSyncClient

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionState.h"

#include <RakNet/RakPeerInterface.h>
#include <RakNet/FileListTransfer.h>


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
	class ClientLoadingState : public FusionState
	{
	public:
		ClientLoadingState(RakClientInterface)
			: FusionState(true),
			m_Server(peer)
		{}

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

	};


	//! Updates the loading GUI when a file is synced
	class ClientLoadingSyncCallback : public FileListTransferCBInterface
	{
	public:
		ClientLoadingSyncCallback();

	public:
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
			unsigned char context)
		{
			if (setCount > 0)
				OnFileSignal(fileIndex/setCount);
		}
	};

}