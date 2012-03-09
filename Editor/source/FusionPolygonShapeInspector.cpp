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
		AddTextInput("PolygonFile",
			StringSetter_t([](std::string value, ComponentIPtr<IPolygonShape> component) { component->PolygonFile.Set(value); }),
			StringGetter_t([](ComponentIPtr<IPolygonShape> component)->std::string { return component->PolygonFile.Get(); })
			);
		Rocket::Core::Factory::InstanceElementText(this, "Edit polygon");
		AddButtonInput("Edit polygon", "Edit",
			StringSetter_t([this](std::string, ComponentIPtr<IPolygonShape> component)
		{
			auto verts = component->Verts.Get();
			auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();
			for (auto it = verts.begin(), end = verts.end(); it != end; ++it)
				*it += offset;
			this->m_PolygonToolExecutor(verts, [component](const std::vector<Vector2>& verts)
			{
				auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();
				auto offsetVerts = verts;
				for (auto it = offsetVerts.begin(), end = offsetVerts.end(); it != end; ++it)
					*it - offset;
				component->Verts.Set(verts);
			});
		}));
		Rocket::Core::Factory::InstanceElementText(this, "Edit as rectangle");
		AddButtonInput("Edit Polygon as Rectangle", "EditRect",
			StringSetter_t([this](std::string, ComponentIPtr<IPolygonShape> component)
		{
			Vector2 hsize(1.f, 1.f);
			Vector2 center;
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
		}));
	}

} }
