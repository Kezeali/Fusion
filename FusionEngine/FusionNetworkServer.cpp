
#include "FusionNetworkServer.h"

/// Fusion
#include "FusionNetworkUtils.h"

using namespace FusionEngine;

FusionNetworkServer::FusionNetworkServer(const std::string &port)
: FusionNetworkGeneric(port)
{
	m_RakServer = RakNetworkFactory::GetRakServerInterface();
	m_RakServer->Start(0, 0, 30, atoi(port.c_str()));

	// Required for timestamps (it should be on by default anyway)
	m_RakServer->StartOccasionalPing();
}

FusionNetworkServer::FusionNetworkServer(const std::string &port, ServerOptions *options)
: FusionNetworkGeneric(port)
{
	m_RakServer = RakNetworkFactory::GetRakServerInterface();
	m_RakServer->Start(options->MaxClients, 0, options->NetDelay, atoi(port.c_str()));

	// Required for timestamps (it should be on by default anyway)
	m_RakServer->StartOccasionalPing();
}

FusionNetworkServer::~FusionNetworkServer()
{
	// The param makes it wait a while for packets to send before disconnecting
	m_RakServer->Disconnect(0, 0);
	RakNetworkFactory::DestroyRakServerInterface(m_RakServer);

	delete m_Queue;
}

void FusionNetworkServer::run()
{
	Packet *p = m_RakServer->Receive();
	while (p)
	{
		bool sysPacket = handleRakPackets(p);

		// If it wasn't a rakNet packet, we can deal with it
		if (!sysPacket)
		{
			PlayerIndex pind = (PlayerIndex)p->playerIndex;
			FusionMessage *m = MessageBuilder::BuildMessage(p, m_PlayerIDMap[pind]);
			m_Queue->_addInMessage(m, m->GetChannel());
		}

		m_RakServer->DeallocatePacket(p);

		// Check for more packets
		p = m_RakServer->Receive();
	}
}


bool FusionNetworkServer::handleRakPackets(Packet *p)
{
	unsigned char packetId = NetUtils::GetPacketIdentifier(p);
	switch (packetId)
	{
		// Give the server env any messages it should handle.
	case ID_NEW_INCOMING_CONNECTION:
	case ID_DISCONNECTION_NOTIFICATION:
		m_Queue->_addEvent(MessageBuilder::BuildMessage(p, m_PlayerIDMap[p->playerIndex]));
		return true;
		break;
	}

	return false;
}
