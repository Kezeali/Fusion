/*
  Copyright (c) 2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#include "FusionPhysicsShape.h"


namespace FusionEngine
{
	/////////
	// Fixture
	/////
	Fixture::Fixture()
		: m_Inner(NULL)
	{}

	Fixture::Fixture(b2Fixture *inner)
		: m_Inner(inner)
	{
		// Point the user data to this wrapper
		m_Inner->SetUserData(this);
	}

	Fixture::~Fixture()
	{
	}

	b2Fixture *Fixture::GetInner() const
	{
		return m_Inner;
	}

	void Fixture::Invalidate()
	{
		m_Inner = NULL;
	}

	//////////////
	// Fixture Utilities
	////////////////
	FixtureDefinition DefineCircleFixture(float radius, const Vector2& local_position, float friction, float restitution, float density)
	{
		b2CircleDef *definition = new b2CircleDef;

		definition->radius = radius;
		definition->localPosition.Set(local_position.x, local_position.y);

		if (!fe_fequal(friction, -1.f))
			definition->friction = friction;
		definition->restitution = restitution;
		definition->density = density;

		return FixtureDefinition(definition);
	}


	////////
	//// Shape
	//////////
	//Shape::Shape()
	//	: m_BxBody(NULL),
	//	m_BxShape(NULL),
	//	m_Offset(0, 0),
	//	m_Mass(0),
	//	m_Friction(0.1f),
	//	m_Name("")
	//{
	//}
	//Shape::Shape(b2Body* body)
	//	: m_BxBody(body),
	//	m_BxShape(NULL),
	//	m_Offset(0,0),
	//	m_Mass(0),
	//	m_Friction(0.1f),
	//	m_Name("")
	//{
	//}

	//Shape::Shape(const Vector2& offset)
	//	: m_BxBody(NULL),
	//	m_BxShape(NULL),
	//	m_Offset(offset),
	//	m_Mass(0),
	//	m_Friction(0.1f),
	//	m_Name("")
	//{
	//}

	//Shape::~Shape()
	//{
	//	// This is fine as is, and here's why:
	//	//  if the world is deleting shapes, they will be automatically removed
	//	//  from their bodies here; if a body is clearing it's shapes, they don't
	//	//  get deleted so what goes on here doesn't matter; and if the
	//	//  world is deleting shapes which have been cleared from bodies, m_BxBody
	//	//  will be null
	//	if (m_BxBody && m_BxShape)
	//		m_BxBody->DestroyShape(m_BxShape);
	//}

	//const b2Shape* Shape::GetShape() const
	//{
	//	return m_BxShape;
	//}

	//void Shape::SetBody(b2Body* body)
	//{
	//	m_BxBody = body;
	//}

	//const b2Body* Shape::GetBody() const
	//{
	//	return m_BxBody;
	//}

	//const b2FilterData& Shape::GetFilter() const
	//{
	//	return m_Filter;
	//}

	//void Shape::SetFilter(b2FilterData filter)
	//{
	//	m_Filter = filter;
	//}

	//void Shape::SetFriction(double friction)
	//{
	//	m_Friction = friction;
	//}

	//void Shape::SetMass(int mass)
	//{
	//	m_Mass = mass;
	//}

	//void Shape::SetName(const std::string &name)
	//{
	//	m_Name = name;
	//}

	//const std::string &Shape::GetName() const
	//{
	//	return m_Name;
	//}

	//void Shape::SetOffset(const Vector2& offset)
	//{
	//	m_Offset.x = offset.x;
	//	m_Offset.y = offset.y;
	//}

	//const Vector2& Shape::GetOffset() const
	//{
	//	return m_Offset;
	//}

	//void Shape::SetOffset_Abs(const Vector2& position)
	//{
	//	Vector2 v;
	//	v2Subtract(position, b2v2(m_BxBody->GetPosition()), v);
	//	m_Offset.x = v.x;
	//	m_Offset.y = v.y;
	//}

	//Vector2 Shape::GetPosition() const
	//{
	//	Vector2 v;
	//	v2Add(GetOffset(), b2v2(m_BxBody->GetPosition()), v);
	//	return v;
	//}

	//bool Shape::IsStatic() const
	//{
	//	if (m_BxBody)
	//		return m_BxBody->IsStatic();
	//	else
	//		return true;
	//}

	////cpFloat Shape::GetInertia(float mass) const
	////{
	////	if (m_Shape->type == CP_POLY_SHAPE)
	////		return ((PolyShape*)this)->GetInertia(mass);
	////	else if (m_Shape->type == CP_CIRCLE_SHAPE)
	////		return ((CircleShape*)this)->GetInertia(mass);
	////	else
	////		return 0;
	////}


	////////
	//// PolyShape
	//////////
	//PolyShape::PolyShape()
	//	: Shape()
	//{
	//}

	//PolyShape::PolyShape(std::vector<Vector2>& verts, const Vector2& offset)
	//	: Shape(offset)
	//{
	//	m_PointList.insert(m_PointList.begin(), verts.begin(), verts.end());
	//}

	//PolyShape::~PolyShape()
	//{
	//}

	//void PolyShape::AddPoint(Vector2 point)
	//{
	//	m_PointList.push_back(point);
	//}

	//void PolyShape::Clear()
	//{
	//	m_PointList.clear();
	//}

	//void PolyShape::Generate()
	//{
	//	if (m_BxShape)
	//	{
	//		m_BxBody->DestroyShape(m_BxShape);
	//		m_BxShape = NULL;
	//	}

	//	b2PolygonDef shapeDef;
	//	shapeDef.vertexCount = m_PointList.size();

	//	for (unsigned int i = 0; i < m_PointList.size(); i++)
	//	{
	//		shapeDef.vertices[i].Set(m_PointList[i].x, m_PointList[i].y);
	//	}

	//	shapeDef.density = 1.0f;
	//	shapeDef.friction = m_Friction;
	//	shapeDef.restitution = 0.1f;
	//	shapeDef.filter.categoryBits = m_Filter.categoryBits;
	//	shapeDef.filter.maskBits = m_Filter.maskBits;
	//	shapeDef.filter.groupIndex = m_Filter.groupIndex;
	//	m_BxShape = m_BxBody->CreateShape(&shapeDef);

	//	b2MassData massData;
	//	massData.mass = m_Mass;
	//	massData.center.SetZero();
	//	massData.I = 0.0f;
	//	m_BxBody->SetMass(&massData);
	//}

	//double PolyShape::GetInitialWidth() const
	//{
	//	FSN_ASSERT(m_PointList.size() > 0);

	//	double width = 0;
	//	double minx = m_PointList[0].x;
	//	double maxx = m_PointList[0].x;

	//	for (unsigned int i = 1; i < m_PointList.size(); i++) {

	//		if (m_PointList[i].x > maxx)
	//			maxx = m_PointList[i].x;

	//		if (m_PointList[i].x < minx)
	//			minx = m_PointList[i].x;
	//	}
	//	width = maxx - minx;
	//	return width;
	//}

	//double PolyShape::GetInitialHeight() const
	//{
	//	FSN_ASSERT(m_PointList.size() > 0);

	//	double height = 0;
	//	double miny = m_PointList[0].y;
	//	double maxy = m_PointList[0].y;

	//	for (unsigned int i = 1; i < m_PointList.size(); i++) {

	//		if (m_PointList[i].y > maxy)
	//			maxy = m_PointList[i].y;

	//		if (m_PointList[i].y < miny)
	//			miny = m_PointList[i].y;
	//	}
	//	height = maxy - miny;
	//	return height;
	//}

	//double PolyShape::GetCurrentWidth() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;

	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double width = 0;
	//	double minx = verts[0].x;
	//	double maxx = verts[0].x;

	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].x > maxx)
	//			maxx = verts[i].x;

	//		if (verts[i].x < minx)
	//			minx = verts[i].x;
	//	}
	//	width = maxx - minx;
	//	return width;
	//}

	//double PolyShape::GetCurrentHeight() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;

	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double height = 0;
	//	double miny = verts[0].y;
	//	double maxy = verts[0].y;

	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].y > maxy)
	//			maxy = verts[i].y;

	//		if (verts[i].y < miny)
	//			miny = verts[i].y;
	//	}
	//	height = maxy - miny;
	//	return height;
	//}

	//double PolyShape::GetCurrentMinX() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;
	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double minx = verts[0].x;
	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].x < minx)
	//			minx = verts[i].x;
	//	}

	//	return m_BxBody->GetPosition().x + minx;
	//}

	//double PolyShape::GetCurrentMaxX() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;
	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double maxx = verts[0].x;
	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].x > maxx)
	//			maxx = verts[i].x;
	//	}

	//	return m_BxBody->GetPosition().x + maxx;
	//}

	//double PolyShape::GetCurrentMinY() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;
	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double miny = verts[0].y;
	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].y < miny)
	//			miny = verts[i].y;
	//	}

	//	return m_BxBody->GetPosition().y + miny;
	//}

	//double PolyShape::GetCurrentMaxY() const
	//{
	//	b2PolygonShape* polygon = (b2PolygonShape*)m_BxShape;
	//	FSN_ASSERT(polygon->GetVertexCount() > 0);

	//	int32 vertCount = polygon->GetVertexCount();
	//	const b2Vec2 *verts = polygon->GetVertices();

	//	double maxy = verts[0].y;
	//	for (unsigned int i = 1; i < unsigned int(vertCount); i++) {

	//		if (verts[i].y > maxy)
	//			maxy = verts[i].y;
	//	}

	//	return m_BxBody->GetPosition().y + maxy;
	//}

	////cpShape* PolyShape::GetShape() const
	////{
	////	return (cpPolyShape*)m_Poly;
	////}

	////cpFloat PolyShape::GetInertia(float mass) const
	////{
	////	cpPolyShape* poly = (cpPolyShape*)m_Shape;
	////	if (m_Body != NULL)
	////		mass += m_Body->GetMass();
	////	return cpMomentForPoly(mass, poly->numVerts, poly->verts, cpv(m_Offset.x, m_Offset.y));
	////}

	//RectangleShape::RectangleShape(double width, double height)
	//	: PolyShape(),
	//	m_Width(width), m_Height(height)
	//{
	//}

	//void RectangleShape::Generate()
	//{
	//	m_PointList.clear();
	//	AddPoint(Vector2(0, 0));
	//	AddPoint(Vector2(m_Width, 0));
	//	AddPoint(Vector2(m_Width, m_Height));
	//	AddPoint(Vector2(0, m_Height));

	//	PolyShape::Generate();
	//}


	////////
	//// CircleShape
	//////////
	//CircleShape::CircleShape()
	//	: Shape(),
	//	m_Radius(0)
	//{
	//}

	//CircleShape::CircleShape(float radius, const Vector2& offset)
	//	: Shape(offset),
	//	m_Radius(radius)
	//{
	//}

	//CircleShape::~CircleShape()
	//{
	//}

	//void CircleShape::Generate()
	//{
	//	if (m_BxShape)
	//	{
	//		m_BxBody->DestroyShape(m_BxShape);
	//		m_BxShape = NULL;
	//	}

	//	b2CircleDef shapeDef;
	//	shapeDef.radius = m_Radius;
	//	shapeDef.localPosition.Set(m_Offset.x, m_Offset.y);
	//	shapeDef.density = 1.0f;
	//	shapeDef.friction = m_Friction;
	//	shapeDef.restitution = 0.1f;
	//	shapeDef.filter.categoryBits = m_Filter.categoryBits;
	//	shapeDef.filter.maskBits = m_Filter.maskBits;
	//	shapeDef.filter.groupIndex = m_Filter.groupIndex;
	//	m_BxShape = m_BxBody->CreateShape(&shapeDef);

	//	b2MassData massData;
	//	massData.mass = m_Mass;
	//	massData.center.SetZero();
	//	massData.I = 0.0f;

	//	m_BxBody->SetMass(&massData);
	//}

	//double CircleShape::GetInitialWidth() const
	//{
	//	return GetCurrentWidth();
	//}

	//double CircleShape::GetInitialHeight() const
	//{
	//	return GetCurrentHeight();
	//}

	//double CircleShape::GetCurrentWidth() const
	//{
	//	return m_Radius*2;
	//}

	//double CircleShape::GetCurrentHeight() const
	//{
	//	return m_Radius*2;
	//}

	//double CircleShape::GetCurrentMinX() const
	//{
	//	return m_BxBody->GetPosition().x + m_Offset.x - m_Radius;
	//}

	//double CircleShape::GetCurrentMaxX() const
	//{
	//	return m_BxBody->GetPosition().x + m_Offset.x + m_Radius;
	//}

	//double CircleShape::GetCurrentMinY() const
	//{
	//	return m_BxBody->GetPosition().y + m_Offset.y - m_Radius;
	//}

	//double CircleShape::GetCurrentMaxY() const
	//{
	//	return m_BxBody->GetPosition().y + m_Offset.y + m_Radius;
	//}

	//cpShape* CircleShape::GetShape() const
	//{
	//	return (cpShape*)m_Shape;
	//}

	//void CircleShape::SetHoop(float centerRad, float outerRad)
	//{
	//	m_Centre = centerRad;
	//	m_Radius = outerRad;
	//}

	//cpFloat CircleShape::GetInertia(float mass) const
	//{
	//	if (m_Body != NULL)
	//		mass += m_Body->GetMass();
	//	return cpMomentForCircle(mass, m_Centre, m_Radius, cpv(m_Offset.x, m_Offset.y));
	//}

}
