
#include "FusionPackSyncClient.h"

#include "FusionPaths.h"

using namespace FusionEngine;

PackSyncClient::PackSyncClient(RakPeerInterface *peer)
: m_Peer(peer)
{
	// install the directory transfer plugin...
	m_TransferPlugin = new FileListTransfer();
	peer->AttachPlugin(m_TransferPlugin);

	m_SyncPlugin = new DirectoryDeltaTransfer();
	m_SyncPlugin->SetFileListTransferPlugin(m_TransferPlugin);
	m_SyncPlugin->SetApplicationDirectory(CL_System::get_exe_path().c_str());

	peer->AttachPlugin(m_SyncPlugin);
}

PackSyncClient::~PackSyncClient()
{
	m_Peer->DetachPlugin(m_SyncPlugin);
	m_Peer->DetachPlugin(m_TransferPlugin);

	delete m_SyncPlugin;
	delete m_TransferPlugin;
}

void PackSyncClient::Initialise()
{
	if (m_SyncPlugin->GetNumberOfFilesForUpload() > 0)
		m_SyncPlugin->ClearUploads();

	m_SyncPlugin->DownloadFromSubdirectory(
		PackagesPath.c_str(), PackagesPath.c_str(),
		true, m_Peer->GetPlayerIDFromIndex(0), File
		);
}
