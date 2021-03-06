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

#if _MSC_VER > 1000
#pragma once
#endif

#include <thrift/server/TServer.h>

#include "../gen-cpp/Editor.h"

#include <functional>
#include <unordered_map>
#include <string>
#include "FusionEditor.h"

namespace FusionEngine
{

	class EngineManager;
	class Editor;

}

namespace FusionEngine { namespace Interprocess
{

	class EditorServiceHandler : public EditorIf
	{
	public:
		EditorServiceHandler(FusionEngine::EngineManager* manager, FusionEngine::Editor* editor);

		void SaveMap(const std::string& name) final;

		void LoadMap(const std::string& name) final;

		void CompileMap(const std::string& name) final;

		void PopDialogRequest(DialogRequest& _return) final;
		
		void CompleteDialogRequest(const DialogRequest& request, const bool success) final;

		void GetUserDataDirectory(std::string& _return) final;

		void GetDataDirectory(std::string& _return) final;

		void GetResources(std::vector<ResourceFile> & _return, const std::string& path) final;

		void GetResourcesRecursive(std::vector<ResourceFile> & _return, const std::string& path) final;

		void GetResourceType(std::string& _return, const std::string& path) final;

		void StartResourceEditor(const std::string& path) final;
		
		void RefreshResources() final;
		
		void CopyResource(const std::string& source, const std::string& destination) final;

		void MoveResource(const std::string& source, const std::string& destination) final;

		void DeleteResource(const std::string& path) final;

		DragDropAction::type SetDragDropData(const DragDropData& data) final;

		bool DragDrop(const DragDropData& data) final;

		void InterpretConsoleCommand(const std::string& command) final;

		void FindConsoleCommandSuggestions(std::vector<std::string> & _return, const std::string& command) final;

		void CompleteCommand(std::string& _return, const std::string& command, const std::string& completion) final;

		void GetConsoleCommandHelp(ConsoleCommandHelpData& _return, const std::string& command) final;

		void GetSelectedEntities(std::vector<EntityData> & _return) final;

		void SelectEntity(const int32_t id) final;

		void FocusOnEntity(const int32_t id) final;

		bool CreateEntity(const std::string& transformType, const bool synced, const bool streamed) final;

		void Stop() final;

	private:
		FusionEngine::EngineManager* engineManager;
		FusionEngine::Editor* editor;

		struct DialogRequestData
		{
			FusionEngine::Editor::FileBrowserOverrideResultFn_t resultFn;
			DialogRequest request;

			DialogRequestData(FusionEngine::Editor::FileBrowserOverrideResultFn_t resultFn, DialogRequest request)
				: resultFn(resultFn),
				request(request)
			{
			}
		};

		std::deque<DialogRequestData> dialogRequests;

		std::map<std::int32_t, FusionEngine::Editor::FileBrowserOverrideResultFn_t> activeDialogRequests;

		std::int32_t nextId;

		void MakeResourceList(std::vector<ResourceFile>& out, const std::vector<std::string>& files);
	};

	class EditorServer
	{
	public:
		EditorServer();

		void Serve(FusionEngine::EngineManager* manager, FusionEngine::Editor* editor);

		void Stop();

	private:
		//FusionEngine::EngineManager* engineManager;

		std::unique_ptr<apache::thrift::server::TServer> server;
	};

} }

