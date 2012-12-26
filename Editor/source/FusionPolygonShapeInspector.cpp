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

#include "PrecompiledHeaders.h"

#include "FusionPolygonShapeInspector.h"

namespace FusionEngine { namespace Inspectors
{

	void PolygonShapeInspector::InitUI()
	{
		AddProperty("PolygonFile", AddTextInput("PolygonFile",
			StringSetter_t([](std::string value, ComponentIPtr<IPolygonShape> component) { component->PolygonFile.Set(value); }),
			StringGetter_t([](ComponentIPtr<IPolygonShape> component)->std::string { return component->PolygonFile.Get(); })
			));
		/*AddProperty("Verts", */AddButtonInput("Edit polygon", "Edit",
			StringSetter_t([this](std::string, ComponentIPtr<IPolygonShape> component)
		{
			if (!component->PolygonFile.Get().empty())
			{
				auto offset = dynamic_cast<EntityComponent*>(component.get())->GetParent()->GetPosition();
				// Translate offset to render-units, because that's what the polygon tool uses
				offset.x = ToRenderUnits(offset.x); offset.y = ToRenderUnits(offset.y);
				this->m_ResourceEditors->StartResourceEditor(component->PolygonFile.Get(), offset);
			}
			else
			{
				auto verts = component->Verts.Get();
				auto offset = dynamic_cast<EntityComponent*>(component.get())->GetParent()->GetPosition();
				for (auto it = verts.begin(), end = verts.end(); it != end; ++it)
				{
					*it += offset;
					it->x = ToRenderUnits(it->x);
					it->y = ToRenderUnits(it->y);
				}
				this->m_PolygonToolExecutor(verts, [component](const std::vector<Vector2>& verts)
				{
					auto offset = dynamic_cast<EntityComponent*>(component.get())->GetParent()->GetPosition();
					auto localVerts = verts;
					for (auto it = localVerts.begin(), end = localVerts.end(); it != end; ++it)
					{
						it->x = ToSimUnits(it->x);
						it->y = ToSimUnits(it->y);
						*it -= offset;
					}
					component->Verts.Set(localVerts);
				});
			}
		}))/*)*/;
		/*AddProperty("Verts", */AddButtonInput("Edit Polygon as Rectangle", "EditRect",
			StringSetter_t([this](std::string, ComponentIPtr<IPolygonShape> component)
		{
			Vector2 hsize(10.f, 10.f);
			Vector2 center = dynamic_cast<EntityComponent*>(component.get())->GetParent()->GetPosition();
			center.x = ToRenderUnits(center.x); center.y = ToRenderUnits(center.y);
			float angle = 0.0f;
			this->m_RectangleToolExecutor(hsize, center, angle, [component](const Vector2& hs, const Vector2& c, float a)
			{
				b2PolygonShape shape;
				shape.SetAsBox(ToSimUnits(hs.x), ToSimUnits(hs.y), b2Vec2(ToSimUnits(c.x), ToSimUnits(c.y)), a);
				std::vector<Vector2> verts; verts.reserve(shape.GetVertexCount());
				for (int i = 0; i < shape.GetVertexCount(); ++i)
				{
					const auto& v = shape.GetVertex(i);
					verts.push_back(Vector2(v.x, v.y));
				}
				component->Verts.Set(verts);
			});
		}))/*)*/;
	}

} }
