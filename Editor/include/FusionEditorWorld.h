/*
*  Copyright (c) 2012-2013 Fusion Project Team
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

#pragma once
#ifndef H_FusionEditorWorld
#define H_FusionEditorWorld

#include "FusionPrerequisites.h"

#include "FusionSystemWorld.h"

namespace Gwen
{
	namespace Controls
	{
		class Canvas;
		class Menu;
	}
	namespace Input { class ClanLib; }
	namespace Renderer { class ClanLib; }
	namespace Skin { class Base; }
}

namespace FusionEngine
{

	class ShapeTool;
	class EditorPolygonTool;
	class EditorRectangleTool;
	class EditorCircleTool;

	class ResourceDatabase;

	class ResourceEditor;

	class InspectorGenerator;

	class OverlayTask;
	class SelectionDrawerTask;

	class EditorWorld : public System::WorldBase
	{
	public:
		EditorWorld(System::ISystem* system);

		void ProcessMessage(Messaging::Message message) override
		{
		}

		std::vector<std::string> GetTypes() const override
		{
			return std::vector<std::string>();
		}

		ComponentPtr InstantiateComponent(const std::string& type) override
		{
			return ComponentPtr();
		}

		void OnActivation(const ComponentPtr& component) override
		{
		}

		void OnDeactivation(const ComponentPtr& component) override
		{
		}

		System::TaskList_t MakeTasksList() const override;

		void SetSelectionBox(const clan::Rectf& box);

		void SelectEntity(const EntityPtr& entity);

		void DeselectEntity(const EntityPtr& entity);

		void SetOffset(const Vector2& offset);

		void ForEachSelected(std::function<void (const EntityPtr&)> fn);

		enum Tool
		{
			None,
			Polygon,
			Line,
			Rectangle,
			Elipse,
			NumTools
		};

	private:
		std::shared_ptr<OverlayTask> m_OverlayTask;
		std::shared_ptr<SelectionDrawerTask> m_SelectionDrawerTask;

		std::shared_ptr<EditorPolygonTool> m_PolygonTool;
		std::shared_ptr<EditorRectangleTool> m_RectangleTool;
		std::shared_ptr<EditorCircleTool> m_CircleTool;

		Tool m_Tool;

		std::array<std::shared_ptr<ShapeTool>, Tool::NumTools> m_ShapeTools;

		std::shared_ptr<ResourceDatabase> m_ResourceDatabase;
		std::map<std::string, std::shared_ptr<ResourceEditor>> m_ResourceEditors;

		void InitInspectorGenerator(InspectorGenerator& generator, const std::function<void (void)>& close_callback);

		void CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback = std::function<void (void)>());
		void CreatePropertiesWindowForSelected();

	};

}

#endif