
#include "FusionNetworkClient.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
: FusionNetworkGeneric(host, port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);
}

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port, ClientOptions *options)
: FusionNetworkGeneric(host, port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);
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
	Packet *p = m_RakClient->Receive();
	while (p);
	{
		bool sysPacket = handleRakPackets(p);

		// If it wasn't a rakNet packet, we can deal with it
		if (!sysPacket)
		{
			// playerid is set to 0, as only the server needs to know that
			FusionMessage *m = FusionMessageBuilder::BuildMessage(p, 0);
			m_Queue->_addInMessage(m, m->GetChannel());
		}

		m_RakClient->DeallocatePacket(p);

		// Check for more packets
		p = m_RakClient->Receive();
	}
}


bool FusionNetworkClient::handleRakPackets(Packet *p)
{
	unsigned char packetId = FusionMessageBuilder::_getPacketIdentifier(p);
	switch (packetId)
	{
		// Give the client env any messages it should handle.
	case ID_CONNECTION_REQUEST_ACCEPTED:
	case ID_NO_FREE_INCOMING_CONNECTIONS:
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:
		m_Queue->_addEvent(FusionMessageBuilder::BuildMessage(p, 0));
		return true;
		break;
	}

	return false;
}
