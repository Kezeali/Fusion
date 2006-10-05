
#include "FusionNetworkServer.h"

using namespace FusionEngine;

FusionNetworkServer::FusionNetworkServer(const std::string &port)
: m_Port(port)
{
	m_RakServer = RakNetworkFactory::GetRakServerInterface();
	m_RakServer->Start(0, 0, 30, atoi(port.c_str()));

	// Init the queue
	m_Queue = new FusionNetworkMessageQueue();
	m_Queue->Resize(FusionEngine::g_ChannelNum);
}

FusionNetworkServer::FusionNetworkServer(const std::string &port, ServerOptions *options)
: m_Port(port)
{
	m_RakServer = RakNetworkFactory::GetRakServerInterface();
	m_RakServer->Start(options->MaxClients, 0, options->NetDelay, atoi(port.c_str()));
	
	// Init the queue
	m_Queue = new FusionNetworkMessageQueue();
	m_Queue->Resize(FusionEngine::g_ChannelNum);
}

FusionNetworkServer::~FusionNetworkServer()
{
	// The param makes it wait a while for packets to send before disconnecting
	m_RakServer->Disconnect(0, 0);
	RakNetworkFactory::DestroyRakServerInterface(m_RakServer);

	delete m_Queue;
}

void FusionNetworkServer::QueueMessage(FusionMessage *message, int channel)
{
	m_Queue->_addOutMessage(message, channel);
}

FusionNetworkServer::MessageQueue FusionNetworkServer::GetAllMessages(int channel)
{
	return m_Queue->_getInMessages(channel);
}

FusionMessage *FusionNetworkServer::GetNextMessage(int channel)
{
	return m_Queue->_getInMessage(channel);
}

void FusionNetworkServer::run()
{
	Packet *p = m_RakServer->Receive();
	while (p)
	{
		bool rakPacket = handleRakPackets(p);

		// If it wasn't a rakNet packet, we can deal with it
		if (!rakPacket)
		{
			PlayerIndex pind = (PlayerIndex)p->playerIndex;
			FusionMessage *m = FusionMessageBuilder::BuildMessage(p, m_PlayerIDMap[pind]);
			m_Queue->_addInMessage(m, m->GetChannel());
		}

		m_RakServer->DeallocatePacket(p);

		// Check for more packets
		p = m_RakServer->Receive();
	}
}

/*
void FusionNetworkServer::_notifyNetEvent(unsigned char messageId)
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}

EventQueue &FusionNetworkServer::GetEvents()
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}
*/

FusionMessage *FusionNetworkServer::GetNextEvent() const
{
	return m_Queue->GetEvent();
}

bool FusionNetworkServer::handleRakPackets(Packet *p)
{
	unsigned char packetId = FusionMessageBuilder::_getPacketIdentifier(p);
	switch (packetId)
	{
		// Give the server env any messages it should handle.
	case ID_NEW_INCOMING_CONNECTION:
	case ID_DISCONNECTION_NOTIFICATION:
		m_Queue->_addEvent(FusionMessageBuilder::BuildMessage(p, m_PlayerIDMap[p->playerIndex]));
		return true;
		break;
	}

	return false;
}

unsigned char FusionNetworkServer::getPacketIdentifier(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p->data[0];
}
