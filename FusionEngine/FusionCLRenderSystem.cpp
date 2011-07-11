/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionCLRenderSystem.h"

#include "FusionCLRenderComponent.h"
#include "FusionRenderer.h"

#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>

namespace FusionEngine
{
	CLRenderSystem::CLRenderSystem(const CL_GraphicContext& gc)
		: m_GraphicContext(gc)
	{
	}

	ISystemWorld* CLRenderSystem::CreateWorld()
	{
		return new CLRenderWorld(this, m_GraphicContext);
	}

	CLRenderWorld::CLRenderWorld(IComponentSystem* system, const CL_GraphicContext& gc)
		: ISystemWorld(system)
	{
		m_Renderer = new Renderer(gc);
		m_RenderTask = new CLRenderTask(this, m_Renderer);
	}

	CLRenderWorld::~CLRenderWorld()
	{
		delete m_RenderTask;
		delete m_Renderer;
	}

	void CLRenderWorld::AddViewport(const ViewportPtr& viewport)
	{
		m_Viewports.push_back(viewport);
	}

	void CLRenderWorld::RemoveViewport(const ViewportPtr& viewport)
	{
		m_Viewports.erase(std::find(m_Viewports.begin(), m_Viewports.end(), viewport));
	}

	std::vector<std::string> CLRenderWorld::GetTypes() const
	{
		static const std::string types[] = { "CLSprite", "" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	std::shared_ptr<IComponent> CLRenderWorld::InstantiateComponent(const std::string& type)
	{
		std::shared_ptr<IComponent> com;
		if (type == "CLSprite")
		{
			com = std::make_shared<CLSprite>();
		}
		return com;
	}

	void CLRenderWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		if (component->GetType() == "CLSprite")
		{
			auto drawable = std::dynamic_pointer_cast<IDrawable>(component);
			if (drawable)
			{
				m_Drawables.push_back(drawable);
			}
		}
	}

	void CLRenderWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto drawable = std::dynamic_pointer_cast<IDrawable>(component);
		if (drawable)
		{
			auto _where = std::find(m_Drawables.begin(), m_Drawables.end(), drawable);
			if (_where != m_Drawables.end())
			{
				_where->swap(m_Drawables.back());
				m_Drawables.pop_back();
			}
		}
	}

	void CLRenderWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		if (type == "CLSprite")
		{
			CLSprite::DeltaSerialiser_t::copyChanges(result, current_data, new_data);
		}
	}

	ISystemTask* CLRenderWorld::GetTask()
	{
		return m_RenderTask;
	}

	CLRenderTask::CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer)
		: ISystemTask(sysworld),
		m_RenderWorld(sysworld),
		m_Renderer(renderer)
	{
	}

	CLRenderTask::~CLRenderTask()
	{
	}

	void CLRenderTask::Update(const float delta)
	{
		CL_GraphicContext gc = m_Renderer->GetGraphicContext();

		auto& drawables = m_RenderWorld->GetDrawables();

		auto depthSort = [](std::shared_ptr<IDrawable>& first, std::shared_ptr<IDrawable>& second)->bool
		{
			if (first->GetParent() == second->GetParent())
			{
				if (first->GetLocalDepth() < second->GetLocalDepth())
					return true;
			}
			else if (first->GetEntityDepth() == second->GetEntityDepth())
			{
				if (first->GetParent() < second->GetParent())
					return true;
			}
			else if (first->GetEntityDepth() < second->GetEntityDepth())
				return true;

			return false;
		};
		tbb::parallel_sort(drawables.begin(), drawables.end(), depthSort);

		//tbb::parallel_for(tbb::blocked_range<size_t>(0, drawables.size()), [drawables, delta](const tbb::blocked_range<size_t>& r)
		//{
		//	for (auto it = r.begin(), end = r.end(); it != end; ++it)
		//	{
		//		drawables[it]->Update(delta);
		//	}
		//});
		
		
		for (auto it = drawables.begin(); it != drawables.end(); ++it)
		{
			auto& drawable = *it;
			drawable->Update(delta);
			// Bubblesort
			//if (it != drawables.begin())
			//{
			//	auto& previous = *(it - 1);
			//	if (depthSort(previous, drawable))
			//		previous.swap(drawable);
			//}
		}

		for (auto it = m_RenderWorld->GetViewports().begin(), end = m_RenderWorld->GetViewports().end(); it != end; ++it)
		{
			const auto& camera = (*it)->GetCamera();

			CL_Rectf drawArea;
			m_Renderer->SetupDraw(*it, &drawArea);
			
			for (auto dit = drawables.begin(), dend = drawables.end(); dit != dend; ++dit)
			{
				auto& drawable = *dit;
				if (!drawable->HasAABB() || drawArea.is_overlapped(drawable->GetAABB()))
				{
					Vector2 camera_pos(camera->GetPosition().x, camera->GetPosition().y);
					drawable->Draw(gc, camera_pos);
				}
			}
		}
	}

}
