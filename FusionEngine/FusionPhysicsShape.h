/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_PhysicsShape
#define Header_FusionEngine_PhysicsShape 

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{

	//class IAttachable
	//{
	//public:
	//	virtual void Attach(cpSpace* space) = 0;
	//	virtual void Detach(cpSpace* space) = 0;
	//};

	//! Shape class
	class Shape
	{
	public:
		Shape(PhysicsBody* body);
		Shape(PhysicsBody* body, const Vector2& offset);
		~Shape();

	public:
		virtual cpShape* GetShape() const;

		virtual cpFloat GetInertia(float mass = 0) const = 0;

		void SetBody(PhysicsBody* body)
		{
			m_Body = body;
			GetShape()->data = m_Body;
		}

		PhysicsBody* GetBody() const
		{
			return m_Body;
		}

		bool IsStatic() const;

		Vector2 GetPosition() const;

		void SetOffset(const Vector2& offset)
		{
			m_Offset.x = offset.x;
			m_Offset.y = offset.y;
		}

		const Vector2& GetOffset() const
		{
			return m_Offset;
		}

	protected:
		PhysicsBody* m_Body;
		Vector2 m_Offset;
		cpShape* m_Shape;
	};

	//! PolyShape Shape implementations
	class PolyShape : public Shape
	{
	public:
		PolyShape(PhysicsBody* body, int num, cpVect* verts, const Vector2& offset);
		~PolyShape();

	public:
		//cpShape* GetShape() const;
		virtual cpFloat GetInertia(float mass) const;

	protected:
		//cpPolyShape* m_Poly;
	};

	//! CircleShape Shape implementations
	class CircleShape : public Shape
	{
	public:
		CircleShape(PhysicsBody* body, float centreRad, float outerRad, const Vector2& offset);
		CircleShape(PhysicsBody* body, float radius, const Vector2& offset);
		~CircleShape();

	public:
		//cpShape* GetShape() const;
		virtual cpFloat GetInertia(float mass) const;

		void SetHoop(float centreRad, float outerRad);

	protected:
		//cpCircleShape* m_Shape;
		float m_Centre;
		float m_Radius;
	};

}

#endif
