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

#include "PrecompiledHeaders.h"

#include "FusionEditorWorld.h"

#include "FusionSystemTask.h"

#include "FusionCamera.h"
#include "Visual/FusionDebugDraw.h"
#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionEntity.h"

namespace FusionEngine
{

	class OverlayTask : public System::TaskBase
	{
	public:
		friend class EditorWorld;

		OverlayTask(EditorWorld* world, const std::shared_ptr<EditorPolygonTool>& poly_tool, const std::shared_ptr<EditorRectangleTool>& rect_tool, const std::shared_ptr<EditorCircleTool>& circle_tool);

		void Update() override;

	private:
		DebugDraw m_DebugDraw;

		clan::Colorf GetColour(const std::set<EntityPtr>::const_iterator& it) const
		{
			const auto fraction = std::distance(it, m_Selected.end()) / (float)m_Selected.size();
			return clan::ColorHSVf(fraction * 360.f, 0.8f, 0.8f, 1.0f);
		}

		clan::Colorf GetColour(const EntityPtr& entity) const
		{
			return GetColour(m_Selected.find(entity));
		}

		std::set<EntityPtr> m_Selected;
		Vector2 m_Offset;

		std::shared_ptr<Camera> m_EditCam;
		float m_CamRange;

		std::shared_ptr<EditorPolygonTool> m_PolygonTool;
		std::shared_ptr<EditorRectangleTool> m_RectangleTool;
		std::shared_ptr<EditorCircleTool> m_CircleTool;
	};

	OverlayTask::OverlayTask(EditorWorld* world, const std::shared_ptr<EditorPolygonTool>& poly_tool, const std::shared_ptr<EditorRectangleTool>& rect_tool, const std::shared_ptr<EditorCircleTool>& circle_tool)
		: System::TaskBase(world, "Editor"),
		m_PolygonTool(poly_tool),
		m_RectangleTool(rect_tool),
		m_CircleTool(circle_tool)
	{
	}

	void OverlayTask::Update()
	{
		for (auto it = m_Selected.begin(), end = m_Selected.end(); it != end; ++it)
		{
			const auto& entity = *it;

			auto pos = entity->GetPosition();
			pos.x = ToRenderUnits(pos.x), pos.y = ToRenderUnits(pos.y);
			pos += m_Offset;

			clan::Rectf box(clan::Sizef(50, 50));
			box.translate(pos.x - box.get_width() * 0.5f, pos.y - box.get_height() * 0.5f);

			m_DebugDraw.DrawRectangle(box, GetColour(it));
		}

		if (m_EditCam)
		{
			clan::Colorf rangeColour(1.0f, 0.6f, 0.6f, 0.95f);
			auto center = m_EditCam->GetPosition();
			auto radius = ToRenderUnits(m_CamRange);
			clan::Rectf camRect(center.x - radius, center.y - radius, center.x + radius, center.y + radius);
			m_DebugDraw.DrawRectangle(camRect, rangeColour);
		}

		if (m_PolygonTool && m_PolygonTool->IsActive())
			m_PolygonTool->Draw();
		if (m_RectangleTool && m_RectangleTool->IsActive())
			m_RectangleTool->Draw();
		if (m_CircleTool && m_CircleTool->IsActive())
			m_CircleTool->Draw();
	}

	class SelectionDrawerTask : public System::TaskBase
	{
	public:
		void Update() override;

		void SetSelectionBox(const clan::Rectf& box) { m_SelectionBox = box; }

		clan::Rectf m_SelectionBox;

		DebugDraw m_DebugDraw;
	};

	void SelectionDrawerTask::Update()
	{
		if (!fe_fzero(m_SelectionBox.get_width()) || !fe_fzero(m_SelectionBox.get_height()))
		{
			auto fillC = clan::Colorf::aquamarine;
			fillC.set_alpha(0.20f);
			m_DebugDraw.DrawRectangle(m_SelectionBox, clan::Colorf::white);
			m_DebugDraw.DrawSolidRectangle(m_SelectionBox, fillC);
		}
	}

	System::TaskList_t EditorWorld::MakeTasksList() const
	{
		System::TaskList_t taskList;
		taskList.push_back(*m_OverlayTask);
		taskList.push_back(*m_SelectionDrawerTask);
		return std::move(taskList);
	}

	void EditorWorld::SetSelectionBox(const clan::Rectf& box)
	{
		m_SelectionDrawerTask->SetSelectionBox(box);
	}
	
	void EditorWorld::SelectEntity(const EntityPtr& entity)
	{
		m_OverlayTask->m_Selected.insert(entity);
	}

	void EditorWorld::DeselectEntity(const EntityPtr& entity)
	{
		m_OverlayTask->m_Selected.erase(entity);
	}

	void EditorWorld::SetOffset(const Vector2& offset)
	{
		m_OverlayTask->m_Offset = offset;
	}

}
