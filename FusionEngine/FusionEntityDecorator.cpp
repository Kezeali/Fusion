/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionEntityDecorator.h"

#include <Rocket/Core/Element.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/GeometryUtilities.h>
#include <Rocket/Core.h>

#include "FusionEntity.h"
#include "FusionEntityManager.h"
#include "FusionRenderer.h"

namespace FusionEngine
{

	EntityDecorator::EntityDecorator(const EntityPtr& entity, Renderer* renderer)
		: m_Entity(entity),
		m_Renderer(renderer)
	{
	}

	Rocket::Core::DecoratorDataHandle EntityDecorator::GenerateElementData(Rocket::Core::Element* element)
	{
		//CL_Rectf&& bounding_box = m_Entity->CalculateOnScreenAABB();

		//CL_GraphicContext gc = m_Renderer->GetGraphicContext();
		//CL_Texture offtex(gc, bounding_box.get_width(), bounding_box.get_height());

		//CL_FrameBuffer offscreen(gc);
		//offscreen.attach_color_buffer(0, offtex);
		//gc.set_frame_buffer(offscreen);

		//// The main selection box
		//gc.clear(CL_Colorf(0.0f, 0.0f, 0.0f, 0.0f));

		//m_Renderer->DrawEntity(m_Entity);

		//CL_Sprite texture;
		//{
		//	CL_SpriteDescription spriteDesc;
		//	spriteDesc.add_frame(offtex);
		//	texture = CL_Sprite(gc, spriteDesc);
		//}

		//gc.reset_frame_buffer();

		//m_Geometry = new Rocket::Core::Geometry();

		//Rocket::Core::Vertex verticies[4];
		//int indicies[4];
		//Rocket::Core::GeometryUtilities::GenerateQuad(verticies, indicies,
		//	EMP::Core::Vector2f(),
		//	EMP::Core::Vector2f( bounding_box.get_width(), bounding_box.get_height() ),
		//	EMP::Core::Colourb(255, 255, 255, 255),
		//	EMP::Core::Vector2f(0, 0), EMP::Core::Vector2f(bounding_box.get_width(), bounding_box.get_height())
		//	);

		//m_Geometry->GetVertices().assign(&verticies[0], &verticies[3]);
		//m_Geometry->GetIndices().assign(&indicies[0], &indicies[3]);

		return 0;
	}

	void EntityDecorator::ReleaseElementData(Rocket::Core::DecoratorDataHandle element_data)
	{
	}

	void EntityDecorator::RenderElement(Rocket::Core::Element* element, Rocket::Core::DecoratorDataHandle element_data)
	{
		//m_Geometry->Render(element->GetAbsoluteOffset(Rocket::Core::Box::PADDING));
		Rocket::Core::Vector2f offset = element->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
		CL_GraphicContext gc = m_Renderer->GetGraphicContext();
		gc.push_modelview();
		gc.set_translate( offset.x, offset.y );

		m_Renderer->DrawEntity(m_Entity);

		gc.pop_modelview();
	}

	DynamicEntityDecorator::DynamicEntityDecorator(const EntityManager* const manager, Renderer* renderer)
		: m_EntityManager(manager),
		m_Renderer(renderer)
	{
	}

	Rocket::Core::DecoratorDataHandle DynamicEntityDecorator::GenerateElementData(Rocket::Core::Element* element)
	{
		std::string entityName = std::string(element->GetAttribute("entity_name", Rocket::Core::String()).CString());

		EntityPtr entity = m_EntityManager->GetEntity(entityName);
		if (entity)
		{
			entity->addRef();
			return (Rocket::Core::DecoratorDataHandle)entity.get();
		}

		return 0;
	}

	void DynamicEntityDecorator::ReleaseElementData(Rocket::Core::DecoratorDataHandle element_data)
	{
		if (element_data)
			reinterpret_cast<Entity*>(element_data)->release();
	}

	void DynamicEntityDecorator::RenderElement(Rocket::Core::Element* element, Rocket::Core::DecoratorDataHandle element_data)
	{
		Entity* entity = reinterpret_cast<Entity*>(element_data);
		if (entity != nullptr)
		{
			CL_Rectf&& aabb = entity->CalculateOnScreenAABB();
			Rocket::Core::Vector2f offset = element->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
			CL_GraphicContext gc = m_Renderer->GetGraphicContext();
			gc.push_modelview();
			gc.set_translate(offset.x + aabb.get_width() * 0.5f, offset.y + aabb.get_height() * 0.5f);

			m_Renderer->DrawEntity(entity);

			gc.pop_modelview();
		}
	}

}