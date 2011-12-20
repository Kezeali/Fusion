
#include "FusionStatePackSyncClient.h"

namespace FusionEngine
{

	PackSyncClientState::PackSyncClientState(const std::string &host, const std::string &port)
		: m_Host(host), m_Port(port)
	{
	}

	~PackSyncClientState::PackSyncClientState()
	{
	}

	bool PackSyncClientState::Initialise()
	{
		m_Peer = RakNetworkFactory::GetRakPeerInterface();
		m_Peer->Connect(
			m_Host.c_str(),
			atoi(m_Port.c_str()),
			0, 0);

		return true;
	}

	bool PackSyncClientState::Update()
	{
		return true;
	}

	void PackSyncClientState::Draw()
	{
	}

	void PackSyncClientState::CleanUp()
	{
		m_Peer->Disconnect(0);
		RakNetworkFactory::DestroyRakClientInterface(m_RakClient);
	}

}