/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_PackSyncState
#define Header_FusionEngine_PackSyncState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionStateMessage.h"
#include "FusionError.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * State to be run when packages should be syncronised.
	 *
	 * PackSync state can run either a host or client Pack syncroniser
	 */
	class PackSyncState : public FusionState
	{
	public:
		//! Basic constructor.
		PackSyncState();
		//! Deconstructor
		~PackSyncState();

	public:
		//! A list of resources
		typedef std::map<std::string, ShipResource*> ShipResourceMap;

	public:
		//! Makes this into a client.
		void MakeClient(const std::string &host, const std::string &port);
		//! Makes this into a server.
		void MakeServer(const std::string &port);

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

		//! List of ship resources in loaded.
		ShipResourceMap m_ShipResources;

	};

}

#endif