
#include "FusionNetworkGeneric.h"

/// Fusion
#include "FusionNetworkUtils.h"

#include <RakNet/BitStream.h>

namespace FusionEngine
{

	FusionNetworkGeneric::FusionNetworkGeneric(const std::string &port)
		: m_Port(port)
	{
		// Init the queue
		m_Queue = new FusionNetworkMessageQueue();
		m_Queue->Resize(FusionEngine::g_ChannelNum);
	}

	FusionNetworkGeneric::FusionNetworkGeneric(const std::string &host, const std::string &port)
		: m_Host(host),
		m_Port(port)
	{
		// Init the queue
		m_Queue = new FusionNetworkMessageQueue();
		m_Queue->Resize(FusionEngine::g_ChannelNum);
	}

	FusionNetworkGeneric::~FusionNetworkGeneric()
	{
		delete m_Queue;
	}

	void FusionNetworkGeneric::SendRaw(char *data, int len)
	{
		send(data, len, MEDIUM_PRIORITY, RELIABLE, (char)CID_NONE);
	}

	void FusionNetworkGeneric::SendShipState(const ShipState& state)
	{
		unsigned char* buf;
		int length;

		length = state.Serialize(buf);

		length = addHeader(buf, length, true, MTID_SHIPFRAME, CID_GAME);

		send(buf, length, LOW_PRIORITY, UNRELIABLE_SEQUENCED, CID_GAME);
	}

	Packet *FusionNetworkGeneric::PopNextMessage(char channel)
	{
		return m_Queue->PopMessage(channel);
	}

	Event *FusionNetworkGeneric::PopNextEvent() const
	{
		Event *e = m_Events.front();
		m_Events.pop_front();
		return e;
	}

	void FusionNetworkGeneric::ClearEvents()
	{
		// Each Message
		EventQueue::iterator it = m_Events.begin();
		for (; it != m_Events.end(); ++it)
		{
			delete (*it);
		}

		m_Events.clear();
	}

	int FusionNetworkGeneric::addHeader(unsigned char* buf, const unsigned char* data, int length, bool timeStamp, MessageType mtid, ChannelID cid)
	{
		// Space for [ID_TIMESTAMP <timestamp> MTID_x CID_x]
		size_t bufLen;
		if (timeStamp)
			bufLen = g_HeaderLengthTimestamp + len;
		else
			bufLen = g_HeaderLength + len;

		buf = malloc(bufLen);

		//! \todo Optimise this
		RakNet::BitStream bitStream(bufLen);
		if (timeStamp)
		{
			bitStream.Write(ID_TIMESTAMP);
			bitStream.Write(RakNet::GetTime());
		}
		bitStream.Write(mtid);
		bitStream.Write(cid);

		int bitStrLen = bitStream.GetNumberOfBytesUsed();

		memcpy(buf, b.GetData(), bitStrLen);
		memcpy(buf+bitStrLen, data, len);

		return (int)bufLen;
	}

	int FusionNetworkGeneric::removeHeader(unsigned char* buf, const unsigned char* data, int dataLen)
	{
		int headLen = NetUtils::GetHeaderLength(data);
		int bufLen = dataLen - headLen;

		buf = malloc(bufLen);
		memcpy(buf, data+headLen, bufLen);

		return bufLen;
	}

	bool FusionNetworkGeneric::grabEvents(Packet *p)
	{
		unsigned char packetId = NetUtils::GetPacketIdentifier(p);
		switch (packetId)
		{
			// Grab any event's that should be handled
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
		case ID_REMOTE_CONNECTION_LOST:
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
		case ID_REMOTE_EXISTING_CONNECTION:

		case ID_CONNECTION_BANNED:
		case ID_CONNECTION_REQUEST_ACCEPTED:
		case ID_NO_FREE_INCOMING_CONNECTIONS:
		case ID_INVALID_PASSWORD:

		case ID_NEW_INCOMING_CONNECTION:

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
		case ID_RECEIVED_STATIC_DATA:
		case ID_MODIFIED_PACKET:
		case ID_CONNECTION_ATTEMPT_FAILED:
			Event e = new Event;
			e.eventID = packetID;
			e.SystemAddress = p->playerId;
			m_Events.push_back(e);
			return true;
		default:
			// I'm not sure if all compilers can handle not having default...
			//  So JIC
			break;
		}

		return false;
	}

	/*
	void FusionNetworkGeneric::_notifyNetEvent(unsigned char messageId)
	{
	m_Mutex->enter();

	//add event

	m_Mutex->notify();
	m_Mutex->leave();
	}

	EventQueue &FusionNetworkGeneric::GetEvents()
	{
	m_Mutex->enter();

	//return events

	m_Mutex->notify();
	m_Mutex->leave();
	}
	*/
}
