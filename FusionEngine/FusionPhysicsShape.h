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

#include "FusionRefCounted.h"
//#include "FusionPhysicsBody.h"


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
		Shape();
		Shape(b2Body* body);
		Shape(const Vector2& offset);
		virtual ~Shape();

	public:
		// Should this be CommitProperties to match PhysicsBody#CommitProperties()?
		virtual void Generate() = 0;

		const b2Shape* GetShape() const;

		void SetBody(b2Body* body);
		const b2Body* GetBody() const;

		void SetFilter(b2FilterData filter);
		const b2FilterData& GetFilter() const;
		void SetFriction(double friction);
		void SetMass(int mass);
		void SetName(const std::string &name);
		const std::string &GetName() const;

		void SetOffset(const Vector2& offset);
		const Vector2& GetOffset() const;

		//! Set the offset to make the position the given world co-ord
		void SetOffset_Abs(const Vector2 &position);

		Vector2 GetPosition() const;

		bool IsStatic() const;

		virtual double GetInitialWidth() const = 0;
		virtual double GetInitialHeight() const = 0;
		// returns current max width (taking angle into account)
		virtual double GetCurrentWidth() const = 0;
		// returns current max height (taking angle into account)
		virtual double GetCurrentHeight() const = 0;

		virtual double GetCurrentMinX() const = 0;
		virtual double GetCurrentMaxX() const = 0;
		virtual double GetCurrentMinY() const = 0;
		virtual double GetCurrentMaxY() const = 0;

		//virtual cpFloat GetInertia(float mass = 0) const = 0;
		//float32 GetInertia(float mass = 0) const;

	protected:
		b2FilterData m_Filter;
		double m_Friction;
		int m_Mass;
		std::string m_Name;
		Vector2 m_Offset;

		b2Body* m_BxBody;
		b2Shape *m_BxShape;
	};

	//! Shape Smart Ptr type
	typedef std::tr1::shared_ptr<Shape> ShapePtr;

	//! PolyShape Shape implementations
	class PolyShape : public Shape 
	{
	public:
		PolyShape();
		PolyShape(std::vector<Vector2>& verts, const Vector2& offset = Vector2::zero());
		~PolyShape();

	public:
		void AddPoint(Vector2 point);
		void Clear();

		virtual void Generate();

		virtual double GetInitialWidth() const;
		virtual double GetInitialHeight() const;
		virtual double GetCurrentWidth() const;
		virtual double GetCurrentHeight() const;

		virtual double GetCurrentMinX() const;
		virtual double GetCurrentMaxX() const;
		virtual double GetCurrentMinY() const;
		virtual double GetCurrentMaxY() const;
		//cpShape* GetShape() const;
		//float32 GetInertia(float mass) const;

	protected:
		//cpPolyShape* m_Poly;
		std::vector<Vector2> m_PointList;
	};

	//! A prefab PolyShape with four points
	class RectangleShape : public PolyShape
	{
	public:
		RectangleShape(double width = 10.0, double height = 10.0);

		virtual void Generate();

	protected:
		double m_Width;
		double m_Height;
	};

	//! CircleShape Shape implementations
	class CircleShape : public Shape
	{
	public:
		CircleShape();
		CircleShape(float radius, const Vector2& offset = Vector2::zero());
		~CircleShape();

	public:
		virtual void Generate();

		virtual double GetCurrentWidth() const;
		virtual double GetCurrentHeight() const;
		virtual double GetInitialWidth() const;
		virtual double GetInitialHeight() const;

		virtual double GetCurrentMinX() const;
		virtual double GetCurrentMaxX() const;
		virtual double GetCurrentMinY() const;
		virtual double GetCurrentMaxY() const;
		//cpShape* GetShape() const;
		//float32 GetInertia(float mass) const;

		//void SetHoop(float centreRad, float outerRad);

	protected:
		//cpCircleShape* m_Shape;
		//float m_Centre;
		float m_Radius;
	};

}

#endif
