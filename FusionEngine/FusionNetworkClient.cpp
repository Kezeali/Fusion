
#include "FusionNetworkClient.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
: m_Host(host),
m_Port(port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);
}

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port, ClientOptions *options)
: m_Host(host),
m_Port(port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);
}

FusionNetworkClient::~FusionNetworkClient()
{
	// The param makes it wait a while for packets to send before disconnecting
	m_RakClient->Disconnect(5);
	RakNetworkFactory::DestroyRakClientInterface(m_RakClient);
}

void FusionNetworkClient::QueueMessage(FusionMessage *message, int channel)
{
	m_Queue->_addOutMessage(message, channel);
}

const MessageQueue &FusionNetworkClient::GetAllMessages(int channel)
{
	return m_Queue->_getInMessages(channel);
}

FusionMessage *FusionNetworkClient::GetNextMessage(int channel)
{
	FusionMessage *ret = m_Queue->_getInMessage(channel);
	return ret;
}

void FusionNetworkClient::run()
{
	do
	{
		// Check for more packets
		Packet *p = m_RakClient->Receive();

		bool rakPacket = handleRakPackets(p);

		// If it wasn't a rakNet packet, we can deal with it
		if (!rakPacket)
		{
			// playerid is set to 0, as only the server needs to know that
			FusionMessage *m = FusionMessageBuilder::BuildMessage(p, 0);
			m_Queue->_addInMessage(m, m->m_Channel);
		}

		m_RakClient->DeallocatePacket(p);
	} while (p);
}

/*
void FusionNetworkClient::_notifyNetEvent(unsigned char messageId)
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}

EventQueue &FusionNetworkClient::GetEvents()
{
	m_Mutex->enter();

	m_Mutex->notify();
	m_Mutex->leave();
}
*/

EventList FusionNetworkClient::GetEvents() const
{
	EventList events = m_MessageQueue->GetEvents();
	m_MessageQueue->ClearEvents();
	return events;
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

unsigned char FusionNetworkClient::getPacketIdentifier(Packet *p)
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
