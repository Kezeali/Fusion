/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionEditorExtension
#define H_FusionEditorExtension

#include "FusionPrerequisites.h"

#include <ClanLib/display.h>

#include <deque>
#include <memory>
#include <vector>

class asIScriptEngine;

namespace FusionEngine
{

	class RendererExtension;

	namespace System
	{
		class WorldBase;
		class ISystem;
	}

	class ArchetypeFactory;
	class ComponentFactory;
	class EntityInstantiator;
	class EntityManager;
	class RegionCellArchivist;
	class StreamingManager;

	class SaveDataArchive;

	class WorldSaver;

	class EngineExtension
	{
	public:
		EngineExtension() : m_Quit(false) {}
		virtual ~EngineExtension() {}

		virtual std::string GetName() const = 0;

		virtual void CleanUp() = 0;

		virtual void Activate() = 0;
		virtual void Deactivate() = 0;

		virtual bool IsEditor() const  { return false; }

		virtual void SetOptions(const ClientOptions& options) = 0;
		// If I change the implementation such that CLRenderSystem creates the display,
		//  this interface can be removed and getting the display can be done in OnWorldCreated
		//  if an extension requires it
		virtual void SetDisplay(const clan::DisplayWindow& display) {}
		virtual void SetComponentFactory(const std::shared_ptr<ComponentFactory>& factory) {}
		virtual void SetEntityInstantiator(const std::shared_ptr<EntityInstantiator>& instantiator) {}
		virtual void SetEntityManager(const std::shared_ptr<EntityManager>& manager) {}
		virtual void SetArchetypeFactory(const std::shared_ptr<ArchetypeFactory>& factory) {}
		virtual void SetMapLoader(const std::shared_ptr<RegionCellArchivist>& map_loader) {}
		virtual void SetStreamingManager(const std::shared_ptr<StreamingManager>& manager) {}
		virtual void SetDataArchiver(const std::shared_ptr<SaveDataArchive>& archiver) {}
		virtual void SetWorldSaver(WorldSaver* saver) {}

		virtual void RegisterScriptType(asIScriptEngine* engine) {}

		virtual void OnWorldCreated(const std::shared_ptr<System::WorldBase>& world) = 0;

		virtual void Update(float time, float dt) = 0;

		virtual System::ISystem* GetSystem() const = 0;

		void RequestQuit() { m_Quit = true; }
		bool HasRequestedQuit() const { return m_Quit; }

		enum MessageType
		{
			None,
			Save,
			Load,
			SwitchToEditMode,
			SwitchToPlayMode
		};

		void PushMessage(MessageType type, const std::string& data)
		{
			m_Messages.push_back(std::make_pair(type, data));
		}

		std::pair<MessageType, std::string> PopMessage()
		{
			if (m_Messages.empty())
				return std::make_pair(MessageType::None, std::string());
			else
			{
				auto message = m_Messages.front();
				m_Messages.pop_front();
				return std::move(message);
			}
		}

		virtual std::vector<std::shared_ptr<RendererExtension>> MakeRendererExtensions() const = 0;

	private:
		bool m_Quit;

		std::deque<std::pair<MessageType, std::string>> m_Messages;

	};

}

#endif
