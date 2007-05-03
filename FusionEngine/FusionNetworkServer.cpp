
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
	m_RakServer->Start(options->mMaxClients, 0, options->mNetDelay, atoi(port.c_str()));

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
	//////////
	// Receive
	Packet *p = m_RakServer->Receive();
	while (p)
	{
		//bool sysPacket = grabEvents(p);

		// grabEvents(*p) handles system-related messages
		//  If it wasn't a rakNet system packet, queue up a message
		if (!grabEvents(p))
		{
			PlayerIndex pind = (PlayerIndex)p->PlayerIndex;
			FusionMessage *m = MessageBuilder::BuildMessage(p, m_SystemAddressMap[pind]);
			m_Queue->_addInMessage(m, m->GetChannel());
		}

		m_RakServer->DeallocatePacket(p);

		// Check for more packets
		p = m_RakServer->Receive();
	}

	///////
	// Send
	for (int chan = 0; chan < g_ChannelNum; chan++)
	{
		FusionMessage *m = m_Queue->_getOutMessage(chan);

		 //&& CL_System::get_time() < e_time
		while (m)
		{
			// System messages - RELIABLE_SEQUENCED, HIGH_PRIORITY, no timestamps
			if (chan == CID_SYSTEM)
				m_RakClient->Send(m->GetBitStream(), HIGH_PRIORITY, RELIABLE_SEQUENCED, CID_SYSTEM);
			// File messages - RELIABLE_SEQUENCED, HIGH_PRIORITY, no timestamps
			else if (chan == CID_FILESYS)
				m_RakClient->Send(m->GetBitStream(), MEDIUM_PRIORITY, RELIABLE_SEQUENCED, CID_FILESYS);
			// Gameplay messages - UNRELIABLE_SEQUENCED, MEDIUM_PRIORITY, timestamps
			else if (chan == CID_GAME)
				m_RakClient->Send(m->GetTimedBitStream(), MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_GAME);
			// Chat messages - RELIABLE, LOW_PRIORITY, timestamps
			else if (chan == CID_CHAT)
				m_RakClient->Send(m->GetTimedBitStream(), LOW_PRIORITY, RELIABLE, CID_CHAT);

			// Check for more messages
			FusionMessage *m = m_Queue->_getOutMessage(chan);
		}
	}
}
