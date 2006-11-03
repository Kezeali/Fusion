
#include "FusionStatePackSync.h"

using namespace FusionEngine;

PackSyncState::PackSyncState()
{
}

~PackSyncState::PackSyncState()
{
}

void PackSyncState::MakeClient(const std::string &host, const std::string &port)
{
	m_Server = true;
	m_Host = host;
	m_Port = port;
}

void PackSyncState::MakeServer(const std::string &port)
{
	m_Server = false;
	m_Port = port;
}

bool PackSyncState::Update()
{
	if (resources->LoadVerified() == false)
		return false;

	m_ShipResources = resources->GetLoadedShips();
}