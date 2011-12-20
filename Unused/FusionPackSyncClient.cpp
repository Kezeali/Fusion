
#include "FusionPackSyncClient.h"

#include "FusionPaths.h"

using namespace FusionEngine;

PackSyncClient::PackSyncClient(RakPeerInterface *peer, FileListTransferCBInterface* fileCallback)
: m_Peer(peer),
m_FileCallback(fileCallback)
{
	//! \todo Is this ok?
	cl_assert(peer != NULL);
	if (!peer->IsActive())
		throw Exception(Exception::NETWORK, "failed to construct PSC");

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
		s_PackagesPath.c_str(), s_PackagesPath.c_str(),
		true, m_Peer->GetSystemAddressFromIndex(0), m_FileCallback,
		HIGH_PRIORITY, 0
		);
}
