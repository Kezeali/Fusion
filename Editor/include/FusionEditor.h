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

#ifndef H_FusionEditor
#define H_FusionEditor

#include "FusionPrerequisites.h"

#include "FusionEngineExtension.h"

#include "FusionTypes.h"

#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <memory>

namespace Rocket { namespace Core {
	class Context;
} }

namespace FusionEngine
{

	class ISystemWorld;
	class AngelScriptWorld;
	class CLRenderWorld;

	class GUIDialog;

	class WorldSaver;

	class Editor : public EngineExtension
	{
	public:
		Editor(const std::vector<CL_String> &args);
		virtual ~Editor();

		void SetDisplay(const CL_DisplayWindow& display);
		void SetComponentFactory(const std::shared_ptr<ComponentFactory>& factory) { m_ComponentFactory = factory; }
		void SetEntityInstantiator(const std::shared_ptr<EntityInstantiator>& instantiator) { m_EntityInstantiator = instantiator; }
		void SetEntityManager(const std::shared_ptr<EntityManager>& manager) { m_EntityManager = manager; }
		// TODO: ? interface MapLoader with Save and Load methods
		void SetMapLoader(const std::shared_ptr<RegionMapLoader>& map_loader);
		void SetStreamingManager(const std::shared_ptr<StreamingManager>& manager) { m_StreamingManager = manager; }
		void SetWorldSaver(WorldSaver* saver) { m_Saver = saver; }

		void OnWorldCreated(const std::shared_ptr<ISystemWorld>& world);

		void SetAngelScriptWorld(const std::shared_ptr<AngelScriptWorld>& asw) { m_AngelScriptWorld = asw; }

		void Update(float time, float dt);

		std::vector<std::shared_ptr<RendererExtension>> MakeRendererExtensions() const;

	private:
		std::shared_ptr<Camera> m_EditCam;

		CL_DisplayWindow m_DisplayWindow;
		std::shared_ptr<AngelScriptWorld> m_AngelScriptWorld;
		std::shared_ptr<CLRenderWorld> m_RenderWorld;

		std::shared_ptr<RegionMapLoader> m_MapLoader;
		std::shared_ptr<StreamingManager> m_StreamingManager;

		std::shared_ptr<ComponentFactory> m_ComponentFactory;
		std::shared_ptr<EntityInstantiator> m_EntityInstantiator;
		std::shared_ptr<EntityManager> m_EntityManager;

		WorldSaver* m_Saver;

		std::vector<EntityPtr> m_NonStreamedEntities;

		CL_Slot m_KeyUpSlot;

		bool m_RebuildScripts;
		bool m_CompileMap;
		bool m_SaveMap;
		bool m_LoadMap;

		Rocket::Core::Context* m_GUIContext;

		std::string m_SaveName;

		std::shared_ptr<GUIDialog> m_Dialog;

		void ShowSaveDialog();
		void ShowLoadDialog();

		void Save();
		void Load();

		void onKeyUp(const CL_InputEvent& ev, const CL_InputState& state);

	};

}

#endif
