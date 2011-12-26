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

#ifndef Header_FusionEngine_FusionPackSyncHost
#define Header_FusionEngine_FusionPackSyncHost

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <RakNet/RakPeerInterface.h>
#include <RakNet/DirectoryDeltaTransfer.h>
#include <RakNet/FileListTransfer.h>


namespace FusionEngine
{

	/*!
	 * \brief
	 * Syncronises data files. ATM, basically a high-level interface to DirectoryDeltaTransfer
	 */
	class PackSyncHost
	{
	public:
		//! Constructor
		PackSyncHost(RakPeerInterface *peer);
		//! Destructor
		~PackSyncHost();

		//! Clears (if necessary) and rebuilds the file lists
		void Initialise();

	protected:
		//! Main plugin
		DirectoryDeltaTransfer *m_SyncPlugin;
		//! Transfer helper
		FileListTransfer *m_TransferPlugin;

		//! The network interface this is attached to
		RakPeerInterface *m_Peer;
	};

}

#endif