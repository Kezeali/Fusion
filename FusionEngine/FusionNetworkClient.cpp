
#include "FusionNetworkClient.h"

using namespace FusionEngine;

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str(),atoi(port.c_str()),atoi(port.c_str()),0,0);
	m_Host = host;
	m_Port = port;
}

FusionNetworkClient::FusionNetworkClient(const std::string &host, const std::string &port,
																				 ClientOptions *options)
{
	m_RakClient = RakNetworkFactory::GetRakClientInterface();
	m_RakClient->Connect(host.c_str,atoi,<#unsigned short clientPort#>,<#unsigned int depreciated#>,<#int threadSleepTimer#>)
}

FusionNetworkClient::InitialiseChannel()
{
}