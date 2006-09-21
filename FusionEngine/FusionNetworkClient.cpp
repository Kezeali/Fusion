
#include "FusionNetworkClient.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
: m_Host(host)
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

FusionNetworkClient::InitialiseChannel()
{
}

FusionNetworkClient::QueueMessage(FusionMessage *message, int channel)
{
	m_MessageQueue.push(message);
}