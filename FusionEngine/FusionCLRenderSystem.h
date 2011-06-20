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

#ifndef H_FusionCLRenderSystem
#define H_FusionCLRenderSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"

#include "FusionComponentSystem.h"
#include "FusionViewport.h"

#include <ClanLib/display.h>

namespace FusionEngine
{
	// forward decl.
	class IDrawable;
	class CLRenderTask;

	class CLRenderSystem : public IComponentSystem
	{
	public:
		SystemType GetType() const { return SystemType::Rendering; }

		std::string GetName() const { return "Box2DSystem"; }

		CLRenderSystem(const CL_GraphicContext& gc);

		ISystemWorld* CreateWorld();

	private:
		CL_GraphicContext m_GraphicContext;
	};

	class CLRenderWorld : public ISystemWorld
	{
	public:
		CLRenderWorld(const CL_GraphicContext& gc);
		virtual ~CLRenderWorld();

		const std::vector<ViewportPtr>& GetViewports() const { return m_Viewports; }
		void AddViewport(const ViewportPtr& viewport);
		void RemoveViewport(const ViewportPtr& viewport);

		const std::vector<std::shared_ptr<IDrawable>>& GetDrawables() const { return m_Drawables; }
		std::vector<std::shared_ptr<IDrawable>>& GetDrawables() { return m_Drawables; }

	private:
		std::vector<std::string> GetTypes() const;

		void MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data);

		std::shared_ptr<IComponent> InstantiateComponent(const std::string& type, const Vector2& pos, float angle);
		void OnActivation(const std::shared_ptr<IComponent>& component);
		void OnDeactivation(const std::shared_ptr<IComponent>& component);

		ISystemTask* GetTask();

		CLRenderTask* m_RenderTask;

		std::vector<std::shared_ptr<IDrawable>> m_Drawables;

		std::vector<ViewportPtr> m_Viewports;

		Renderer* m_Renderer;
	};

	class CLRenderTask : public ISystemTask
	{
	public:
		CLRenderTask(CLRenderWorld* sysworld, Renderer* const renderer);
		~CLRenderTask();

		void Update(const float delta);

		bool IsPrimaryThreadOnly() const
		{
			return true;
		}

	private:
		CLRenderWorld* m_RenderWorld;
		Renderer* const m_Renderer;
	};

}

#endif
