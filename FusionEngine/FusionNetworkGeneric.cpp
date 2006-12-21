
#include "FusionNetworkGeneric.h"

/// Fusion
#include "FusionNetworkUtils.h"

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

	void FusionNetworkGeneric::SendRaw(char *data, char channel)
	{
		send(data, MEDIUM_PRIORITY, RELIABLE, channel);
	}

	void FusionNetworkGeneric::SendAddPlayer()
	{
		// The client sends this, the server replies with a PlayerInd in a initial State,
		//  then the client sends a SendPlayerConfig packet with the ShipPackageID used
		//  by the player's ship.
		// THIS SHOULDN'T BE IN GENERIC, CLIENT AND SERVER USE ADDPLAYER DIFFERENTLY
		send(data, MEDIUM_PRIORITY, RELIABLE, CID_SYSTEM);
	}

	Packet *FusionNetworkGeneric::PopNextMessage(char channel)
	{
		return m_Queue->PopMessage(channel);
	}

	Packet *FusionNetworkGeneric::PopNextEvent() const
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
			e.playerID = p->playerId;
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
