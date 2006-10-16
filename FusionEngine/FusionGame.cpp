
#include "FusionEngineCommon.h"

/// Fusion
#include "FusionResourceLoader.h"
#include "FusionClientEnvironment.h"

/// Class
#include "FusionGame.h"

using namespace FusionEngine;

FusionGame::FusionGame()
{
}

void FusionGame::RunClient(const std::string &hostname, const std::string &port, ClientOptions *options)
{
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

	bool keepGoing = false;

	ResourceLoader *resLoader = new ResourceLoader();
	resLoader->LoadShips(resLoader->GetInstalledShips());

	// Try to setup the gameplay env
	ClientEnvironment *env = new ClientEnvironment(hostname, port, options);
	keepGoing = env->Initialise(resLoader);

	unsigned int lastTime = CL_System::get_time();
	unsigned int split = 0;

	while (keepGoing)
	{
		split = CL_System::get_time() - lastTime;
		lastTime = CL_System::get_time();

		keepGoing = env->Update(split);
		env->Draw();
	}

	delete env;
	delete resLoader;

}

void FusionGame::RunServer(const std::string &port, ServerOptions *options)
{
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
