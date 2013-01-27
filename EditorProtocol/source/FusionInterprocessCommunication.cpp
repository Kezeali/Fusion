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

#include <boost/tokenizer.hpp>

#include "FusionEngineManager.h"
#include "FusionEditor.h"
#include "FusionEntity.h"
#include "FusionViewport.h"
#include "FusionPhysFS.h"

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

	void EditorServiceHandler::GetUserDataDirectory(std::string & _return)
	{
		_return = PHYSFS_getWriteDir();
	}

	void EditorServiceHandler::GetDataDirectory(std::string & _return)
	{
		_return = PHYSFS_getBaseDir();
	}

	void EditorServiceHandler::MakeResourceList(std::vector<ResourceFile>& out, const std::vector<std::string>& files)
	{
		for (auto file : files)
		{
			ResourceFile resource;
			resource.filename = file;
			resource.directory = PHYSFS_isDirectory(file.c_str()) != 0;
			if (!resource.directory)
				resource.type = editor->GetResourceType(file);
			out.push_back(resource);
		}
	}
	
	void EditorServiceHandler::GetResources(std::vector<ResourceFile> & _return, const std::string& path)
	{
		auto files = PhysFSHelp::list_content(path);
		MakeResourceList(_return, files);
	}

	void EditorServiceHandler::GetResourcesRecursive(std::vector<ResourceFile> & _return, const std::string& path)
	{
		auto files = PhysFSHelp::list_content(path, true);
		MakeResourceList(_return, files);
	}

	void EditorServiceHandler::GetResourceType(std::string & _return, const std::string& path)
	{
		editor->GetResourceType(path);
	}

	namespace 
	{
		void copy_file_even_if_dest_nonexist(const std::string& source, const std::string& destination)
		{
			for (size_t c = destination.find('/'); c != std::string::npos; c = destination.find('/', c + 1))
			{
				auto newPath = destination.substr(0, c);
				if (PHYSFS_isDirectory(newPath.c_str()) == 0)
				{
					PHYSFS_mkdir(newPath.c_str());
				}
			}
			PhysFSHelp::copy_file(source, destination);
		}
	}

	void EditorServiceHandler::CopyResource(const std::string& source, const std::string& destination)
	{
		try
		{
			copy_file_even_if_dest_nonexist(source, destination);
		}
		catch (FusionEngine::Exception& ex)
		{
			SendToConsole("Failed to copy resource\n" + source + "\n-> " + destination + ":\n" + ex.GetDescription());
		}
	}
	
	void EditorServiceHandler::MoveResource(const std::string& source, const std::string& destination)
	{
		try
		{
			copy_file_even_if_dest_nonexist(source, destination);
			PHYSFS_delete(source.c_str());
		}
		catch (FusionEngine::Exception& ex)
		{
			SendToConsole("Failed to move resource " + source + " -> " + destination + ": " + ex.GetDescription());
		}
	}

	void EditorServiceHandler::DeleteResource(const std::string& path)
	{
		if (PHYSFS_isDirectory(path.c_str()))
			PhysFSHelp::clear_folder(path);
		PHYSFS_delete(path.c_str());
	}

	void EditorServiceHandler::InterpretConsoleCommand(const std::string& command)
	{
		Console::getSingleton().Interpret(command);
	}

	void EditorServiceHandler::FindConsoleCommandSuggestions(std::vector<std::string> & _return, const std::string& command)
	{
		if (command.find(' ') != std::string::npos)
			_return = Console::getSingleton().ListPossibleCompletions(command);
		else
			Console::getSingleton().ListPrefixedCommands(command, _return, 20);
	}

	void EditorServiceHandler::CompleteCommand(std::string& _return, const std::string& command, const std::string& completion)
	{
		_return = Console::getSingleton().Autocomplete(command, completion);
	}

	void EditorServiceHandler::GetConsoleCommandHelp(ConsoleCommandHelpData& _return, const std::string& command)
	{
		auto commandHelp = Console::getSingleton().GetCommandHelp(command);
		_return.helpText = commandHelp.helpText;
		_return.argumentNames = commandHelp.argumentNames;
	}

	void EditorServiceHandler::GetSelectedEntities(std::vector<EntityData> & _return)
	{
		editor->ForEachSelected([&_return](const EntityPtr& entity)
		{
			EntityData data;
			data.id = entity->GetID();
			data.owner = entity->GetOwnerID();
			data.name = entity->GetName();

			for (auto component : entity->GetComponents())
			{
				try
				{
					EntityComponentData componentData;
					componentData.type = component->GetType();
					componentData.name = component->GetIdentifier();
					RakNet::BitStream stream;
					component->SerialiseEditable(stream);
					componentData.state.assign(reinterpret_cast<char*>(stream.GetData()), stream.GetNumberOfBytesUsed());
				}
				catch (FusionEngine::Exception&)
				{}
			}
			_return.push_back(data);

			return true;
		});
	}

	void EditorServiceHandler::SelectEntity(const int32_t id)
	{
		editor->SelectEntityWithID(ObjectID(id));
	}
	
	void EditorServiceHandler::FocusOnEntity(const int32_t id)
	{
		editor->GoToEntityWithID(id);
	}

	bool EditorServiceHandler::CreateEntity(const std::string& transformType, const bool synced, const bool streamed)
	{
		auto camera = editor->GetViewport()->GetCamera();
		auto entity = editor->CreateEntity(transformType, camera->GetSimPosition(), camera->GetAngle(), synced, streamed);
		if (entity)
			return true;
		else
			return false;
	}

	void EditorServiceHandler::Stop()
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
