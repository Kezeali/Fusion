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

	//! 'Input' callback for the Edit Circle button
	//class EditVertsButtonFunctor
	//{
	//public:
	//	EditCircleButtonFunctor(GenericInspector<ComponentT>* executor_, FloatSetter_t x_setter_, FloatGetter_t x_getter_, FloatSetter_t y_setter_, FloatGetter_t y_getter_, FloatSetter_t radius_setter_, FloatGetter_t radius_getter_)
	//		: executor(executor_),
	//		x_setter(x_setter_), x_getter(x_getter_), y_setter(y_setter_), y_getter(y_getter_), radius_setter(radius_setter_), radius_getter(radius_getter_)
	//	{
	//	}

	//	//EditCircleButtonFunctor(CircleToolExecutor_t executor_, FloatSetter_t x_setter_, FloatGetter_t x_getter_, FloatSetter_t y_setter_, FloatGetter_t y_getter_, FloatSetter_t radius_setter_, FloatGetter_t radius_getter_)
	//	//	: executor(executor_),
	//	//	x_setter(x_setter_), x_getter(x_getter_), y_setter(y_setter_), y_getter(y_getter_), radius_setter(radius_setter_), radius_getter(radius_getter_)
	//	//{
	//	//}

	//	void operator() (bool, ComponentIPtr<ComponentT> component)
	//	{
	//		auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();

	//		const Vector2 c(ToRenderUnits(x_getter(component) + offset.x), ToRenderUnits(y_getter(component) + offset.y));
	//		const float r = ToRenderUnits(radius_getter(component));
	//		executor->m_CircleToolExecutor(c, r, [this, component](const Vector2& c, float r)
	//		{
	//			auto offset = dynamic_cast<IComponent*>(component.get())->GetParent()->GetPosition();
	//			x_setter(ToSimUnits(c.x) - offset.x, component);
	//			y_setter(ToSimUnits(c.y) - offset.y, component);
	//			radius_setter(ToSimUnits(r), component);
	//		});
	//	}

	//	GenericInspector<ComponentT>* executor;
	//	//CircleToolExecutor_t executor;
	//	FloatSetter_t x_setter, y_setter, radius_setter;
	//	FloatGetter_t x_getter, y_getter, radius_getter;
	//	Vector2 offset;
	//};

	void PolygonShapeInspector::InitUI()
	{
		AddTextInput("PolygonFile",
			StringSetter_t([](std::string value, ComponentIPtr<IPolygonShape> component) { component->PolygonFile.Set(value); }),
			StringGetter_t([](ComponentIPtr<IPolygonShape> component)->std::string { return component->PolygonFile.Get(); })
			);
		Rocket::Core::Factory::InstanceElementText(this, "Edit polygon");
		//AddButtonInput("Edit",
		//	StringSetter_t([](std::string value, ComponentIPtr<IPolygonShape> component) { component->PolygonVerts.Set(value); }),
		//	);
		Rocket::Core::Factory::InstanceElementText(this, "Edit as rectangle");
		AddButtonInput("EditRect",
			StringSetter_t([this](std::string value, ComponentIPtr<IPolygonShape> component)
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
				//component->PolygonVerts.Set(verts);
			});
		}));
	}

} }
