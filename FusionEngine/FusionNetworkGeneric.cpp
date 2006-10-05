
#include "FusionNetworkServer.h"

using namespace FusionEngine;

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

void FusionNetworkGeneric::QueueMessage(FusionMessage *message, int channel)
{
	m_Queue->_addOutMessage(message, channel);
}

FusionMessage *FusionNetworkGeneric::GetNextMessage(int channel)
{
	return m_Queue->_getInMessage(channel);
}

FusionMessage *FusionNetworkGeneric::GetNextEvent() const
{
	return m_Queue->GetEvent();
}

/*
void FusionNetworkGeneric::_notifyNetEvent(unsigned char messageId)
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}

EventQueue &FusionNetworkGeneric::GetEvents()
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}
*/

bool FusionNetworkGeneric::handleRakPackets(Packet *p)
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
