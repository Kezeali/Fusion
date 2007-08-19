/*
  Copyright (c) 2006-2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#include "FusionCommon.h"

/// Class
#include "FusionGame.h"

/// Fusion
#include "FusionGUI.h"

#include "FusionScriptingEngine.h"
#include "FusionStateManager.h"
#include "FusionPackSyncClient.h"
#include "FusionEnvironmentClient.h"
#include "FusionEnvironmentServer.h"
#include "FusionClientLoadingState.h"

#include "FusionException.h"
#include "FusionConsole.h"
#include "FusionLogger.h"

namespace FusionEngine
{

	void FusionGame::Run(const CmdOptions* opts)
	{
		bool keepGoing = false;

		try
		{
			new Console();
			Logger *logger = new Logger();

			// Initialise options
			m_ClientOpts = new ClientOptions();
			m_ClientOpts->LoadFromFile("clientcfg.xml");
			m_ServerOpts = new ServerOptions();
			m_ServerOpts->LoadFromFile("servercfg.xml");

			if (m_ClientOpts->mConsoleLogging)
				logger->ActivateConsoleLogging();

			StateManager *state_man = new StateManager();

			// Host option is set
			if (opts->OptionExists("host"))
			{
				std::string hostname = opts->GetOption("host");
				std::string port = opts->GetOption("port");

				if (port.empty())
					port = g_DefaultPort;


				// Set up the initial states for instant-action
				SharedState gui(new GUI());
				SharedState load(new ClientLoadingState(hostname, port, m_ClientOpts));
				SharedState env(new ClientEnvironment(hostname, port, m_ClientOpts));

				// Init the state manager
				keepGoing = state_man->SetExclusive(load);
				state_man->AddState(gui);

				state_man->AddStateToQueue(env);
			}

			// start_server option is set
			else if (opts->OptionExists("start_server"))
			{
				std::string port = opts->GetOption("port");

				if (port.empty())
					port = g_DefaultPort;


				// Set up the initial states for dedicated-server
				SharedState gui(new GUI());
				SharedState load(new ServerLoadingState(port, m_ServerOpts));
				SharedState env(new ServerEnvironment(port, m_ServerOpts));

				// Init the state manager
				keepGoing = state_man->SetExclusive(load);
				state_man->AddState(gui);

				state_man->AddStateToQueue(env);
			}

			// Normal startup
			else
			{
				// Set up the initial states for the menu
				SharedState gui(new GUI());
				SharedState menu(new MainMenu());
				

				// Init the state manager
				keepGoing = state_man->SetExclusive(gui);
				state_man->AddState(menu);
			}


			unsigned int lastTime = CL_System::get_time();
			unsigned int split = 0;

			while (keepGoing)
			{
				split = CL_System::get_time() - lastTime;
				lastTime = CL_System::get_time();

				// Catch trivial exceptions without stopping the game
				try
				{
					// Stop if any states don't update
					keepGoing = state_man->Update(split);

					// Stop if any states think we should
					keepGoing = state_man->KeepGoing();

					state_man->Draw();
				}
				catch (Exception &e)
				{
					// Not critical and trivial, so record it in the console and continue
					if (!e.IsCritical() && e.GetType() == Exception::TRIVIAL)
						Console::getSingletonPtr()->Add(e.GetError());

					// Critical, exit the game and log the message
					else if (e.IsCritical())
						throw;

					// Non-critical
					else
					{
						GUI* gui = GUI::getSingletonPtr();
						if (gui == NULL)
							throw Exception(ExceptionType::INTERNAL_ERROR, "FusionGame::Run - GUI not initialised", true);

						// Do something like this...
						//gui.ShowMessageBox(...);
					}
				}

			}

			state_man->Clear();

			delete state_man;
		}
		catch (Exception& e)
		{
			Logger* logger = Logger::getSingletonPtr();

			logger->Add(e, g_LogException, LOG_CRITICAL);

			delete Console::getSingletonPtr();
			delete logger;
		}
		catch (CL_Error err)
		{
			Logger* logger = Logger::getSingletonPtr();

			logger->Add(e, g_LogException, LOG_CRITICAL);

			delete Console::getSingletonPtr();
			delete logger;
		}

	}

	//void FusionGame::RunServer(const std::string &port, ServerOptions *options)
	//{
	//	bool keepGoing = false;

	//	Console *console = new Console();
	//	Logger *logger = new Logger();
	//	if (options->mConsoleLogging)
	//		log->ActivateConsoleLogging();

	//	try
	//	{

	//		// Try to setup the server env
	//		StateManager *state_man = new StateManager();
	//		SharedState load(new ServerLoadingState(port, options));
	//		SharedState env(new ServerEnvironment(port, options));

	//		// Start the state manager
	//		keepGoing = state_man->SetExclusive(load);
	//		state_man->AddStateToQueue(env);

	//		unsigned int lastTime = CL_System::get_time();
	//		unsigned int split = 0;

	//		while (keepGoing)
	//		{
	//			split = CL_System::get_time() - lastTime;
	//			lastTime = CL_System::get_time();

	//			// Stop if any states encounter an error
	//			keepGoing = state_man->Update(split);

	//			// Stop if any states think we should
	//			keepGoing = state_man->KeepGoing();

	//			// I guess this doesn't do anything for the server :P
	//			//  Maybe it could output to the console...
	//			state_man->Draw();
	//		}

	//		state_man->Clear();
	//	}
	//	catch (Error e)
	//	{
	//		console->Add(e.GetError());
	//		logger->Add(e, g_LogExceptionServer, LOG_CRITICAL);

	//		delete console;
	//		delete logger;

	//		throw CL_Error( "Fusion aborted with the error: " + e.GetError() );
	//	}

	//}

}

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
