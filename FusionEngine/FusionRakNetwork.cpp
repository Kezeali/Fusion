/*
  Copyright (c) 2007 Fusion Project Team

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

#include "FusionRakNetwork.h"

#include <RakNet/RakNetworkFactory.h>
#include <RakNet/RakNetStatistics.h>
#include <RakNet/BitStream.h>
#include <RakNet/GetTime.h>

namespace FusionEngine
{

	///////////////
	// RakNetPacket
	RakNetPacket::RakNetPacket(Packet* originalPacket)
		: m_OriginalPacket(originalPacket),
		m_Handle(std::string(originalPacket->systemAddress.ToString()))
	{
		RakNet::BitStream bits(originalPacket->data, originalPacket->bitSize / 8, false);
		// Timestamp
		m_TimeStamped = false;
		m_Time = 0;
		if (originalPacket->data[0] == ID_TIMESTAMP)
		{
			char idTimestamp;
			bits.Read(idTimestamp);
			bits.Read(m_Time);
			m_TimeStamped = true;
		}
		// Type
		char type;
		bits.Read(type);
	}

	RakNetPacket::RakNetPacket(NetHandle from, Packet* originalPacket)
		: m_OriginalPacket(originalPacket),
		m_Handle(from)
	{
		RakNet::BitStream bits(originalPacket->data, originalPacket->bitSize / 8, false);
		// Timestamp
		m_TimeStamped = false;
		m_Time = 0;
		if (originalPacket->data[0] == ID_TIMESTAMP)
		{
			char idTimestamp;
			bits.Read(idTimestamp);
			bits.Read(m_Time);
			m_TimeStamped = true;
		}
		// Type
		char type;
		bits.Read(type);
	}

	RakNetPacket::~RakNetPacket()
	{
		assert(m_OriginalPacket == 0);
	}

	std::string RakNetPacket::GetDataString()
	{
		unsigned int header = (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength);
		unsigned int length = m_OriginalPacket->bitSize / 8;
		char* dataBegin;
		char* dataEnd;

		dataBegin = (char*)(m_OriginalPacket->data + header);
		dataEnd = (char*)(m_OriginalPacket->data + length);
		return std::string(dataBegin, dataEnd);
	}

	const char* RakNetPacket::GetData() const
	{
		return (char*)m_OriginalPacket->data + (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength);
	}

	unsigned int RakNetPacket::GetLength() const
	{
		return m_OriginalPacket->bitSize / 8 - (IsTimeStamped() ? g_HeaderLengthTimestamp : g_HeaderLength);
	}

	unsigned char RakNetPacket::GetType() const
	{
		return m_OriginalPacket->data[IsTimeStamped() ? g_TimestampLength : 0];
	}

	bool RakNetPacket::IsTimeStamped() const
	{
		return m_TimeStamped;
	}

	NetTime RakNetPacket::GetTime() const
	{
		return m_Time;
	}

	const NetHandle& RakNetPacket::GetSystemHandle() const
	{
		return m_Handle;
	}

	/////////////
	// RakNetwork
	RakNetwork::RakNetwork()
		: m_MinLagMilis(0), m_LagVariance(0),
		m_AllowBps(0.0)
	{
		m_NetInterface = RakNetworkFactory::GetRakPeerInterface();
	}

	RakNetwork::~RakNetwork()
	{
		RakNetworkFactory::DestroyRakPeerInterface(m_NetInterface);
	}

	bool RakNetwork::Startup(unsigned short maxConnections, unsigned short incommingPort, unsigned short maxIncommingConnections)
	{
		SocketDescriptor socDesc(incommingPort, 0);
		if (m_NetInterface->Startup(maxConnections, 0, &socDesc, 1))
		{
			m_NetInterface->SetMaximumIncomingConnections(maxIncommingConnections);
			return true;
		}
		else
			return false;
	}

	bool RakNetwork::Connect(const std::string& host, unsigned short port)
	{
		if (m_NetInterface->Connect(host.c_str(), port, 0, 0, 0))
		{
			m_NetInterface->SetOccasionalPing(true);
			return true;
		}
		else
			return false;
	}

	void RakNetwork::Disconnect()
	{
		m_NetInterface->Shutdown(100);
	}

	bool RakNetwork::Send(bool timestamped, char type, char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		RakNet::BitStream bits;
		if (timestamped)
		{
			bits.Write((MessageID)ID_TIMESTAMP);
			bits.Write(RakNet::GetTime());
		}
		bits.Write((MessageID)type);
		bits.Write(data, length);

		return m_NetInterface->Send(&bits, rakPriority(priority), rakReliability(reliability), channel, m_SystemAddresses[destination], false);
	}

	bool RakNetwork::SendRaw(char* data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		return m_NetInterface->Send(data, length, rakPriority(priority), rakReliability(reliability), channel, m_SystemAddresses[destination], false);
	}

	IPacket* RakNetwork::Receive()
	{
		Packet* rakPacket = m_NetInterface->Receive();
		if (rakPacket == 0)
			return 0;

		NetHandle handle(rakPacket->systemAddress.ToString());
		m_SystemAddresses.insert( SystemAddressMap::value_type(handle, rakPacket->systemAddress) );

		return new RakNetPacket(rakPacket);
	}

	void RakNetwork::PushBackPacket(IPacket *packet, bool toHead)
	{
		RakNetPacket* nativePacket = dynamic_cast<RakNetPacket*>(packet);
		if (nativePacket != 0)
			m_NetInterface->PushBackPacket(nativePacket->m_OriginalPacket, toHead);
	}

	void RakNetwork::DeallocatePacket(IPacket *packet)
	{
		RakNetPacket* nativePacket = dynamic_cast<RakNetPacket*>(packet);
		if (nativePacket != 0)
			m_NetInterface->DeallocatePacket(nativePacket->m_OriginalPacket);

		nativePacket->m_OriginalPacket = 0;

		delete nativePacket;
	}

	RakNetStatistics *const RakNetwork::GetStatistics(const SystemAddress system)
	{
		return m_NetInterface->GetStatistics(system);
	}

	int RakNetwork::GetPing(const NetHandle& handle)
	{
		return m_NetInterface->GetAveragePing(m_SystemAddresses[handle]);
	}

	int RakNetwork::GetLastPing(const NetHandle& handle)
	{
		return m_NetInterface->GetLastPing(m_SystemAddresses[handle]);
	}

	int RakNetwork::GetAveragePing(const NetHandle& handle)
	{
		return m_NetInterface->GetAveragePing(m_SystemAddresses[handle]);
	}

	int RakNetwork::GetLowestPing(const NetHandle& handle)
	{
		return m_NetInterface->GetLowestPing(m_SystemAddresses[handle]);
	}

	void RakNetwork::SetDebugLag(unsigned int minLagMilis, unsigned int variance)
	{
		m_MinLagMilis = minLagMilis;
		m_LagVariance = variance;
		m_NetInterface->ApplyNetworkSimulator(m_AllowBps, (unsigned short)minLagMilis, (unsigned short)variance);
	}

	void RakNetwork::SetDebugPacketLoss(double allowBps)
	{
		m_AllowBps = allowBps;
		m_NetInterface->ApplyNetworkSimulator(allowBps, (unsigned short)m_MinLagMilis, (unsigned short)m_LagVariance);
	}

	unsigned int RakNetwork::GetDebugLagMin() const
	{
		return m_MinLagMilis;
	}

	unsigned int RakNetwork::GetDebugLagVariance() const
	{
		return m_LagVariance;
	}

	double RakNetwork::GetDebugAllowBps() const
	{
		return m_AllowBps;
	}


	inline PacketPriority RakNetwork::rakPriority(NetPriority priority)
	{
		return (PacketPriority)priority;
	}

	inline PacketReliability RakNetwork::rakReliability(NetReliability reliability)
	{
		return (PacketReliability)reliability;
	}

	//int RakNetwork::addHeader(unsigned char* buf, const unsigned char* data, int length, bool timeStamp, unsigned char mtid)
	//{
	//	// Space for [ID_TIMESTAMP <timestamp> MTID_x CID_x]
	//	size_t bufLen;
	//	if (timeStamp)
	//		bufLen = g_HeaderLengthTimestamp + len;
	//	else
	//		bufLen = g_HeaderLength + len;

	//	buf = malloc(bufLen);

	//	//! \todo Optimise this
	//	RakNet::BitStream bitStream(bufLen);
	//	if (timeStamp)
	//	{
	//		bitStream.Write(ID_TIMESTAMP);
	//		bitStream.Write(RakNet::GetTime());
	//	}
	//	bitStream.Write(mtid);

	//	int bitStrLen = bitStream.GetNumberOfBytesUsed();

	//	memcpy(buf, b.GetData(), bitStrLen);
	//	memcpy(buf+bitStrLen, data, len);

	//	return (int)bufLen;
	//}

	//int RakNetwork::removeHeader(unsigned char* buf, const unsigned char* data, int dataLen)
	//{
	//	unsigned int headLen = g_HeaderLength;
	//	int bufLen = dataLen - headLen;

	//	buf = malloc(bufLen);
	//	memcpy(buf, data+headLen, bufLen);

	//	return bufLen;
	//}

	//// Not used
	//bool RakNetwork::isChaff(Packet *p)
	//{
	//	unsigned char packetId = NetUtils::GetPacketIdentifier(p);
	//	switch (packetId)
	//	{
	//		// Stuff we don't want to handle
	//	case ID_REMOTE_DISCONNECTION_NOTIFICATION:
	//	case ID_REMOTE_CONNECTION_LOST:
	//	case ID_REMOTE_NEW_INCOMING_CONNECTION:
	//	case ID_REMOTE_EXISTING_CONNECTION:

	//	case ID_CONNECTION_BANNED:
	//	case ID_CONNECTION_REQUEST_ACCEPTED:
	//	case ID_NO_FREE_INCOMING_CONNECTIONS:
	//	case ID_INVALID_PASSWORD:

	//	case ID_NEW_INCOMING_CONNECTION:

	//	case ID_DISCONNECTION_NOTIFICATION:
	//	case ID_CONNECTION_LOST:
	//	case ID_RECEIVED_STATIC_DATA:
	//	case ID_MODIFIED_PACKET:
	//	case ID_CONNECTION_ATTEMPT_FAILED:
	//		return true;
	//	default:
	//	}

	//	return false;
	//}

	}