/*
*  Copyright (c) 2010 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef Header_FusionNetworkManager
#define Header_FusionNetworkManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <RakNetTypes.h>

#include "FusionPacketHandler.h"
#include "FusionSingleton.h"

namespace FusionEngine
{

	class ElectionPacketHandler : public PacketHandler
	{
	public:
		//! Returns the GUID of the current arbiter
		const RakNetGUID &GetArbitratorGUID() const;

		//void VoteNoConfidence();

		//! impl. PacketHandler
		void HandlePacket(Packet *packet);

	protected:
		RakNetGUID m_ArbitratorGUID;
	};

	//! Singleton class - manages automatic network stuff
	/*! 
	* Dispatches incomming packet, elects new Arbitrators when neccessary
	*/
	class NetworkManager : public Singleton<NetworkManager>
	{
	public:
		NetworkManager(RakNetwork *network, PacketDispatcher *dispatcher);
		~NetworkManager();

		//! Convinience function - calls the relevant method on the NetworkManager singleton
		static const RakNetGUID &GetArbitratorGUID();

		//! Dispatches new packets
		void DispatchPackets();

		//! Allows a peer to take control of the update rate.
		/*
		* This allows step-through debugging without the game going out of sync.<br>
		* NOTE: If multiple clients request step control, the server waits for all
		* clients to reach the current step before sending out the continue message
		* at each step.
		*/
		virtual void RequestStepControl();

	private:
		RakNetwork *m_Network;
		PacketDispatcher *m_Dispatcher;

		ElectionPacketHandler m_ArbitratorElector;
	};

}

#endif
