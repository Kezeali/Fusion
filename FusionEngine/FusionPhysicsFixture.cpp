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

#include "FusionStableHeaders.h"

#include "FusionPhysicsFixture.h"


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

	void Fixture::SetTag(const std::string &tag)
	{
		m_Tag = tag;
	}

	const std::string &Fixture::GetTag() const
	{
		return m_Tag;
	}

	void Fixture::SetUserData(const FixtureUserDataPtr &user_data)
	{
		m_UserData = user_data;
	}

	const FixtureUserDataPtr &Fixture::GetUserData() const
	{
		return m_UserData;
	}

	void Fixture::Invalidate()
	{
		m_Inner = NULL;
	}

	FixturePtr Fixture::GetWrapper(b2Fixture *fixture)
	{
		Fixture *data = static_cast<Fixture*>( fixture->GetUserData() );
		if (data != NULL)
			return FixturePtr( data );
		else
			return FixturePtr();
	}

	//////////////
	// Fixture Utilities
	////////////////
	FixtureDefinition DefineCircleFixture(float radius, const Vector2& local_position, float friction, float restitution, float density)
	{
		FixtureDefinition def;
		std::tr1::shared_ptr<b2CircleShape> shape(new b2CircleShape());

		shape->m_radius = radius;
		shape->m_p.Set(local_position.x, local_position.y);

		def.SetShape(shape);

		if (!fe_fequal(friction, -1.f))
			def.definition.friction = friction;
		def.definition.restitution = restitution;
		def.definition.density = density;

		return def;
	}

}
