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

#ifndef H_FusionNetworkManager
#define H_FusionNetworkManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <RakNetTypes.h>
#include <boost/signals2.hpp>

#include "FusionPacketHandler.h"
#include "FusionSingleton.h"
#include "FusionIDStack.h"

namespace FusionEngine
{

	static const unsigned int s_NetCompressedStringTrunc = 256;

	class ElectionPacketHandler : public PacketHandler
	{
	public:
		ElectionPacketHandler();

		//! Returns the GUID of the current arbiter
		const RakNet::RakNetGUID &GetArbitratorGUID() const;

		//void VoteNoConfidence();

		//! impl. PacketHandler
		void HandlePacket(RakNet::Packet *packet);

		RakNet::RakNetGUID m_ArbitratorGUID;
	};

	class PeerIDManager : public PacketHandler
	{
	public:
		PeerIDManager(RakNetwork *network);
		uint8_t m_PeerID;

	private:
		void HandlePacket(RakNet::Packet *packet);

		RakNetwork* m_Network;
		IDSet<uint8_t> m_UnusedIDs;
		bool m_WaitingForID;
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
		static RakNet::RakNetGUID GetArbitratorGUID();

		//! Convinience function - returns true if GetLocalGUID() == GetArbitratorGUID()
		static bool ArbitratorIsLocal();

		// TEMP - makes this peer the default arbitrator during the limbo
		//  period where GetArbitratorGUID returns UNASSIGNED_RAKNET_GUID
		//  One solution to this problem would be to use a plugin that
		//  sets m_Hosting to false if you start a connection attempt
		//  (then resets m_Hosting if that attempt fails)
		static void SetHosting(bool value);

		//! Returns the peer index of this machine (from 0 - s_MaxPeers)
		/*!
		* Lower index indicates earlier connection.
		*/
		static uint8_t GetPeerSeniorityIndex();

		//! Returns true if the given peer has seniority over the local peer
		static bool IsSenior(const RakNet::RakNetGUID &peer); 

		//! Returns true if peerA has seniority over peerB
		static bool IsSenior(const RakNet::RakNetGUID &peerA, const RakNet::RakNetGUID &peerB); 

		static bool IsSenior(const PlayerInfo &playerA, const PlayerInfo &playerB); 

		static void ForEachPeer(std::function<void (const RakNet::RakNetGUID &)>&& fn);

		//! Returns the unique peer-id of this peer
		static uint8_t GetPeerID();

		//! Returns the current network object
		static RakNetwork* GetNetwork();

		void Subscribe(unsigned char type, PacketHandler *handler);
		void Unsubscribe(unsigned char type, PacketHandler *handler);

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
		PeerIDManager m_PeerIDManager;

		bool m_Hosting;
	};

}

#endif
