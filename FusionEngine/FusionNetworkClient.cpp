
#include "FusionNetworkClient.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
: m_Host(host),
m_Port(port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(), atoi(port.c_str()), atoi(port.c_str()), 0, 0);
}

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port,
																				 ClientOptions *options)
																				 : m_Host(host),
																				 m_Port(port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str, atoi(port.c_str()), atoi(port.c_str()), 0, 0);
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

MessageQueue FusionNetworkClient::GetAllMessages(int channel)
{
	return m_Queue->_getInMessages(channel);
}

FusionMessage *FusionNetworkClient::GetNextMessage(int channel)
{
	FusionMessage *ret = m_Queue->_getInMessages(channel).front();
	m_Queue->_getInMessages(channel).pop_front();
	return ret;
}

void FusionNetworkClient::Run()
{
	Packet *p = m_RakClient->Receive();
	CL_Timer time = CL_Timer(1000);
	while (p && time <)
	{
		bool rakPacket = handleRakPackets(p);

		// If it wasn't a rakNet packet, we can deal with it
		if (!rakPacket)
		{
			unsigned char packetId = getPacketIdentifier(p);
			switch (packetId)
			{
			case MTID_SHIPFRAME:
				break;
			}
		}

		// Check for more packets
		p = m_RakClient->Receive();
	}
}

bool FusionNetworkClient::handleRakPackets(Packet *p)
{
	unsigned char packetId = getPacketIdentifier(p);
	switch (packetId)
	{
		// Give the client env any messages it should handle.
	case ID_CONNECTION_REQUEST_ACCEPTED:
	case ID_NO_FREE_INCOMING_CONNECTIONS:
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:
		_notifyNetEvent(packetId);
		break;
	}
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
