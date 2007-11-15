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

	Shape::Shape(PhysicsBody* body)
		: m_Body(body)
	{
	}

	Shape::Shape(PhysicsBody* body, const Vector2& offset)
		: m_Body(body),
		m_Offset(offset)
	{
	}

	Shape::~Shape()
	{
		// This is fine as is, and here's why:
		//  if the world is deleting shapes, they will be automatically removed
		//  from their bodies here; if a body is clearing it's shapes, they don't
		//  get deleted so what goes on here doesn't matter; and if the
		//  world is deleting shapes which have been cleared from bodies, m_Body
		//  will be null
		if (m_Body)
			m_Body->DetachShape(this);
	}

	bool Shape::IsStatic() const
	{
		if (m_Body)
			return m_Body->IsStatic();
		else
			return false;
	}

	Vector2 Shape::GetPosition() const
	{
		Vector2 v;
		v2Add(GetOffset(), m_Body->GetPosition(), v);
		return v;
	}

	cpShape* Shape::GetShape() const
	{
		return m_Shape;
	}

	PolyShape::PolyShape(PhysicsBody* body, int num, cpVect* verts, const Vector2& offset)
		: Shape(body, offset)
	{
		m_Shape = cpPolyShapeNew(body->GetChipBody(), num, verts, cpv(offset.x, offset.y) );
	}

	PolyShape::~PolyShape()
	{
		cpShapeFree((cpShape*)m_Shape);
	}

	//cpShape* PolyShape::GetShape() const
	//{
	//	return (cpPolyShape*)m_Poly;
	//}

	cpFloat PolyShape::GetInertia(float mass) const
	{
		cpPolyShape* poly = (cpPolyShape*)m_Shape;
		if (m_Body != NULL)
			mass += m_Body->GetMass();
		return cpMomentForPoly(mass, poly->numVerts, poly->verts, cpv(m_Offset.x, m_Offset.y));
	}

	CircleShape::CircleShape(PhysicsBody* body, float centerRad, float outerRad, const Vector2& offset)
		: Shape(body, offset),
		m_Centre(centerRad),
		m_Radius(outerRad)
	{
		m_Shape = cpCircleShapeNew(body->GetChipBody(), m_Radius, cpv(offset.x, offset.y) );
	}

	CircleShape::CircleShape(PhysicsBody* body, float radius, const Vector2& offset)
		: Shape(body, offset),
		m_Centre(0.f),
		m_Radius(radius)
	{
		m_Shape = cpCircleShapeNew(body->GetChipBody(), m_Radius, cpv(offset.x, offset.y) );
	}

	CircleShape::~CircleShape()
	{
		cpShapeFree((cpShape*)m_Shape);
	}

	//cpShape* CircleShape::GetShape() const
	//{
	//	return (cpShape*)m_Shape;
	//}

	void CircleShape::SetHoop(float centerRad, float outerRad)
	{
		m_Centre = centerRad;
		m_Radius = outerRad;
	}

	cpFloat CircleShape::GetInertia(float mass) const
	{
		if (m_Body != NULL)
			mass += m_Body->GetMass();
		return cpMomentForCircle(mass, m_Centre, m_Radius, cpv(m_Offset.x, m_Offset.y));
	}

};