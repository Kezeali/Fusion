// The loading state should first connect to the server and find out where the 
//  fileserver is, then use that data to initialise the pack sync client.
//  once the pack sync client has run, it should create the resource loader
//  and ask the server what ships to load, the server will then send
//  VerifyPackage packets, with which the resource loader will verify
//  each package. LoadVerified will then be called on the resource loader.
//  Finally, the state will remove itself allowing the state manager to run
//  the next state in the queue, which should be ClientEnvironment.

// This class should run a gui, so at each stage indicated above, it can update
//  a progress bar. Obviously, it won't be able to tell how far completed the
//  PackSync is, but pack sync has it's own GUI, so that is a non-issue.
//  Also, it won't be able to get the progress of LoadVerified, but that
//  makes console messages, so the interested user can check those out if they
//  really want to :P Anyway, Source does it like that (doesn't show the progr-
//  ess of data loading, just says "Loading Resources"), so I feel assured.

#include "FusionClientLoadingState.h"

#include "FusionClientOptions.h"
#include "FusionPackSyncClient.h"
#include "FusionResourceLoader.h"

#include <RakNet/RakClientInterface.h>

namespace FusionEngine
{

	ClientLoadingState::ClientLoadingState(RakClientInterface* peer, const std::string& host, unsigned short port, ClientOptions* options)
		: FusionState(true),
		m_Connection(peer),
		m_Host(host),
		m_Port(port),
		m_Options(options)
	{
		m_FileConnection = RakNetworkFactory::GetRakClientInterface();
		m_PackSyncClient = new PackSyncClient(fsPeer, this);

		m_ProgressBar = static_cast<ProgressBar*>(WindowManager::getSingleton().getWindow("Loading/Wind/ProgBar"));

		SharedStage connect;

		AddLoadingStage();
	}

	ClientLoadingState::~ClientLoadingState()
	{
	}

	bool ClientLoadingState::Initialise()
	{
		m_Connection->Connect(m_Host.c_str(), m_Port, m_Options->mNetworkOptions.mLocalPort, 0, 0);

		m_PackSyncClient->Initialise();
	}

	bool ClientLoadingState::Update(unsigned int split)
	{
		

		//switch (m_Stage)
		//{
		//case LOAD_CONNECT:
		//case LOAD_SYNC:
		//case LOAD_RESOURCES:
		//	// If sync is finished, make sure the progress bar is at the file loading position
		//	m_Progress = PROG_BEGINFILELOAD;
		//	ResourceLoader::getSingletonPtr();
		//	f
		//	break;
		//case LOAD_DONE:
		//	_pushMessage(new StateMessage(StateMessage::REMOVESTATE, this));
		//}
	}

	void ClientLoadingState::OnFile(
		unsigned fileIndex,
		char *filename,
		char *fileData,
		unsigned compressedTransmissionLength,
		unsigned finalDataLength,
		unsigned short setID,
		unsigned setCount,	
		unsigned setTotalCompressedTransmissionLength,
		unsigned setTotalFinalLength,
		unsigned char context)
	{
		if (setCount > 0)
		{
			m_Progress += m_PerFileinc;
			m_ProgressBar->setProgress(m_Progress);
		}
		
	}

}
