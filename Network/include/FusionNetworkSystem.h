/*
  Copyright (c) 2009 Fusion Project Team

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
#ifndef H_FusionEngine_NetworkSystem
#define H_FusionEngine_NetworkSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionPacketHandler.h"
//#include "FusionState.h"
#include "FusionTypes.h"
#include <string>

namespace FusionEngine
{

	//! Prints any packets it receives to a log
	class DebugPacketHandler : public PacketHandler
	{
	public:
		DebugPacketHandler();

		void HandlePacket(RakNet::Packet *packet);

	protected:
		LogPtr m_Log;
	};

	//! Runs the network automation stuff
	class NetworkSystem/* : public System*/
	{
	public:
		NetworkSystem();
		virtual ~NetworkSystem();

		virtual const std::string &GetName() const;

		virtual bool Initialise();
		virtual void CleanUp();

		virtual void Update(float split);
		virtual void Draw();

		bool IsConnected() const;

	protected:
		PacketDispatcher *m_PacketDispatcher;
		RakNetwork *m_Network;

		NetworkManager *m_NetworkManager;

		DebugPacketHandler m_DebugPacketHandler;
	};

}

#endif
