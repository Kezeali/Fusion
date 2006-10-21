
#include "FusionEngineCommon.h"

/// Class
#include "FusionGame.h"

/// Fusion
#include "FusionResourceLoader.h"
#include "FusionClientEnvironment.h"
#include "FusionStateManager.h"

using namespace FusionEngine;

FusionGame::FusionGame()
{
}

void FusionGame::RunClient(const std::string &hostname, const std::string &port, ClientOptions *options)
{
	//! \todo FusionGame GUI (for displaying error messages.)
	//CL_GUIManager gui();

	bool keepGoing = false;
	Error *lastError = NULL;

	// Try to setup the gameplay env
	StateManager *state_man = new StateManager();
	ClientEnvironment *env = new ClientEnvironment(hostname, port, options);

	// Start the state manager
	keepGoing = state_man->SetExclusive(env);

	unsigned int lastTime = CL_System::get_time();
	unsigned int split = 0;

	while (keepGoing)
	{
		split = CL_System::get_time() - lastTime;
		lastTime = CL_System::get_time();

		if ((keepGoing = state_man->Update(split)) == false)
			lastError = state_man->GetLastError();

		// Stop if any states think we should
		keepGoing = state_man->KeepGoing();

		state_man->Draw();
	}

	state_man->Clear();

	if (lastError != NULL)


	/*
	CL_SetupNetwork setup_network;
	CL_NetSession *netgame = NULL;

	//create a new netsession
	netgame = new CL_NetSession("FusionEngine");
	//    //connect the disconnect and receive signals to some slots
	//    slot_disconnect = netgame->sig_computer_disconnected().connect(this,
	//                      &NetChannelDemo::on_client_disconnect);
	//
	//    //we have two different slots depending on the packet channel received
	//    slot_receive_ping = netgame->sig_netpacket_receive("ping").connect(this,
	//                        &NetChannelDemo::on_ping_receive);
	//    slot_receive_message = netgame->sig_netpacket_receive("message").connect(this,
	//                           &NetChannelDemo::on_message_receive);

	//connect to the server (running on the local machine in this case)
	CL_IPAddress server_ip;
	server_ip.set_address(hostname, port);
	netgame->connect(server_ip);
	*/
}

void FusionGame::RunServer(const std::string &port, ServerOptions *options)
{
	bool keepGoing = false;

	ResourceLoader *resLoader = new ResourceLoader();
	resLoader->LoadShips(resLoader->GetInstalledShips());

	// Try to setup the gameplay env
	ServerEnvironment *env = new ServerEnvironment(port, options);
	keepGoing = env->Initialise(resLoader);

	unsigned int lastTime = CL_System::get_time();
	unsigned int split = 0;

	while (keepGoing)
	{
		split = CL_System::get_time() - lastTime;
		lastTime = CL_System::get_time();

		keepGoing = env->Update(split);
		env->Draw(); // I guess this doesn't do anything :P maybe it could output to the console...
	}

	delete env;
	delete resLoader;

	/*
	CL_SetupNetwork setup_network;
	CL_NetSession *netgame = NULL;

	//Create a new netsession
	netgame = new CL_NetSession("FusionEngine");
	//  //connect the connect and disconnect signals to some slots
	//  slot_connect = netgame->sig_computer_connected().connect(this,
	//   &NetChannelDemo::on_server_connect);
	//  slot_disconnect = netgame->sig_computer_disconnected().connect(this,
	//   &NetChannelDemo::on_server_disconnect);
	//start the server listening for client activity
	netgame->start_listen(port);
	*/
}
