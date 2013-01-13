/*
*  Copyright (c) 2013 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "stdafx.h"

#include "FusionInterprocessCommunication.h"

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
//#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "FusionEngineManager.h"
#include "FusionEditor.h"
#include "FusionEntity.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

namespace FusionEngine { namespace Interprocess {
	
	EditorServiceHandler::EditorServiceHandler(FusionEngine::EngineManager* manager, FusionEngine::Editor* editor)
		: engineManager(manager),
		editor(editor)
	{
	}

	void EditorServiceHandler::test(const Test& t)
	{
		SendToConsole(t.name);
		editor->ForEachSelected([](const EntityPtr& entity)->bool
		{
			SendToConsole(entity->GetName());
			return true;
		});
	}

	void EditorServiceHandler::stop()
	{
		editor->RequestQuit();
	}


	EditorServer::EditorServer()
	{
	}

	void EditorServer::Serve(FusionEngine::EngineManager* engineManager, FusionEngine::Editor* editor)
	{
		boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

		boost::shared_ptr<EditorServiceHandler> handler(new EditorServiceHandler(engineManager, editor));
		boost::shared_ptr<TProcessor> processor(new EditorProcessor(handler));

		boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(9090));
		boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

		//TNonblockingServer s2(processor, protocolFactory, 9080);

		server.reset(new TSimpleServer(processor,
			serverTransport,
			transportFactory,
			protocolFactory));

		server->serve();
	}

	void EditorServer::Stop()
	{
		server->stop();
	}

} }
