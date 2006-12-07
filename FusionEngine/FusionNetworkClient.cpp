
#include "FusionNetworkClient.h"

/// Fusion
#include "FusionNetworkUtils.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
: FusionNetworkGeneric(host, port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);

	// Required for timestamps (it should be on by default anyway)
	m_RakClient->StartOccasionalPing();
}

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port, ClientOptions *options)
: FusionNetworkGeneric(host, port), m_Options(options)
{
	unsigned short local_port = options->NetworkOptions.LocalPort;

	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(
		host.c_str(),
		atoi(port.c_str()),
		local_port, 0, 0);

	// Required for timestamps (it should be on by default anyway, so JIC)
	m_RakClient->StartOccasionalPing();
}

FusionNetworkClient::~FusionNetworkClient()
{
	// The param makes it wait a while for packets to send before disconnecting
	m_RakClient->Disconnect(5);
	RakNetworkFactory::DestroyRakClientInterface(m_RakClient);

	delete m_Queue;
}

void FusionNetworkClient::run()
{
	//////////
	// Receive
	Packet *p = m_RakClient->Receive();

	//unsigned int e_time = CL_System::get_time() + m_Options->mNetworkOptions.mNetDelay;
	// The same e_time applies to both receive and send

	 //&& CL_System::get_time() < e_time
	while (p)
	{
		// Queue packets that should be considered events
		bool handle = !handleRakPackets(p);

		// Don't handle non-fusion packets (might be file transfer packets).
		handle = NetUtils::IsFusionPacket(p);

		// If it wasn't queued as an event, we can deal with it.
		if (handle)
		{
			// playerid is set to 0, as only the server needs to know that
			FusionMessage *m = MessageBuilder::BuildMessage(p, 0);
			m_Queue->_addInMessage(m, m->GetChannel());
		}

		m_RakClient->DeallocatePacket(p);

		// Check for more packets
		p = m_RakClient->Receive();
	}

	///////
	// Send
	for (int chan = 0; chan < g_ChannelNum; chan++)
	{
		FusionMessage *m = m_Queue->_getOutMessage(chan);

		 //&& CL_System::get_time() < e_time
		while (m)
		{
			// System messages - RELIABLE, HIGH_PRIORITY, no timestamps
			if (m->GetChannel() == CID_SYSTEM)
				m_RakClient->Send(m->GetBitStream(), HIGH_PRIORITY, RELIABLE, CID_SYSTEM);
			// File messages - RELIABLE, HIGH_PRIORITY, no timestamps
			if (m->GetChannel() == CID_FILESYS)
				m_RakClient->Send(m->GetBitStream(), MEDIUM_PRIORITY, RELIABLE, CID_FILESYS);
			// Gameplay messages - UNRELIABLE_SEQUENCED, MEDIUM_PRIORITY, timestamps
			if (m->GetChannel() == CID_GAME)
				m_RakClient->Send(m->GetTimedBitStream(), MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_GAME);
			// Chat messages - RELIABLE, LOW_PRIORITY, timestamps
			if (m->GetChannel() == CID_CHAT)
				m_RakClient->Send(m->GetTimedBitStream(), LOW_PRIORITY, RELIABLE, CID_CHAT);

			// Check for more messages
			FusionMessage *m = m_Queue->_getOutMessage(chan);
		}
	}
}


bool FusionNetworkClient::handleRakPackets(Packet *p)
{
	unsigned char packetId = NetUtils::GetPacketIdentifier(p);
	switch (packetId)
	{
		// Give the client env any messages it should handle.
	case ID_CONNECTION_REQUEST_ACCEPTED:
	case ID_NO_FREE_INCOMING_CONNECTIONS:
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:
		m_Queue->_addEvent(MessageBuilder::BuildMessage(p, 0));
		return true;
		break;
	}

	return false;
}
