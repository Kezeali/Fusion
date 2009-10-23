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


namespace FusionEngine
{

	class Fixture : public RefCounted
	{
		friend class PhysicsBody;
	public:
		Fixture();
		Fixture(b2Fixture *inner);

		virtual ~Fixture();

		b2Fixture *GetInner() const;

	protected:
		b2Fixture *m_Inner;

		//! Called when Body dies
		void Invalidate();
	};

	typedef boost::intrusive_ptr<Fixture> FixturePtr;

	class FixtureDefinition// : public RefCounted
	{
	public:
		typedef std::tr1::shared_ptr<b2Shape> ShapePtr;
		ShapePtr shape;
		b2FixtureDef definition;

		FixtureDefinition()
		{
		}

		~FixtureDefinition()
		{
		}

		void SetShape(const ShapePtr &_shape)
		{
			shape = _shape;
			definition.shape = shape.get();
		}
	};

	typedef boost::intrusive_ptr<FixtureDefinition> FixtureDefinitionPtr;

	FixtureDefinition DefineCircleFixture(float radius, const Vector2& local_position = Vector2::zero(), float friction = -1.f, float restitution = 0.0f, float density = 0.0f);

}

#endif
