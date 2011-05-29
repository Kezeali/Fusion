/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef H_FusionEngine_PhysicalEntity
#define H_FusionEngine_PhysicalEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <functional>
#include <Box2D/Box2D.h>

#include "FusionEntity.h"
#include "FusionPhysicsFixture.h"

namespace FusionEngine
{

	class BodyDestroyer
	{
	public:
		typedef std::tr1::function<void (b2Body*)> CallbackFn;

		BodyDestroyer();
		BodyDestroyer(const CallbackFn &fn);

		void Destroy(b2Body *body) const;

		void SetCallback(const CallbackFn &fn);
		void ClearCallback();

		bool IsConnected() const;

	protected:
		CallbackFn m_Callback;
	};

	typedef std::tr1::shared_ptr<BodyDestroyer> BodyDestroyerPtr;

	//! Returns the game co-ords transformation of the sim-position
	Vector2 ToGameUnits(const Vector2 &sim_position);
	//! Returns the game co-ords transformation of the game-position
	void ToGameUnits(Vector2 &out, const Vector2 &sim_position);
	//! Transforms the given sim-position to a game-position
	void TransformToGameUnits(Vector2 &sim_position);

	//! Returns the given game position as a sim-position
	Vector2 ToSimUnits(const Vector2 &game_position);
	//! Returns the given game position as a sim-position
	void ToSimUnits(Vector2 &out, const Vector2 &game_position);
	//! Transforms the given game-position to a sim-position
	void TransformToSimUnits(Vector2 &game_position);

	//! Physically simulated Entity
	class PhysicalEntity : public Entity
	{
	public:
		//! CTOR
		PhysicalEntity();
		//! CTOR
		PhysicalEntity(const std::string &name);

		//! DTOR
		virtual ~PhysicalEntity();

		//! Applies force
		virtual void ApplyForce(const Vector2 &force, const Vector2 &point);
		//! Applies force
		virtual void ApplyTorque(float torque);

		//! Gets the position of the physical body
		virtual const Vector2 &GetPosition();
		//! Gets angle (rotation) value
		virtual float GetAngle() const;
		//! Gets linear velocity
		virtual const Vector2 &GetVelocity();
		//! Gets angular (rotational) velocity
		virtual float GetAngularVelocity() const;

		//! Gets position
		virtual void SetPosition(const Vector2 &position);
		//! Gets angle (rotation) value
		virtual void SetAngle(float angle);
		//! Gets linear velocity
		virtual void SetVelocity(const Vector2 &velocity);
		//! Gets angular (rotational) velocity
		virtual void SetAngularVelocity(float angular_vel);

		// TODO: damping, allowSleep, fixedRotation, isBullet

		//! Returns true if this entity has a b2Body object
		bool IsPhysicsEnabled() const;

		//! Returns the held simulation object (if it is enabled)
		b2Body *GetBody() const;
		//! Sets the sim. obj.
		void _setBody(b2Body *body);

		//! Creates a fixture
		/*!
		* The FixturePtr (wrapper) will be added to the fixture list held by
		* this entity
		*/
		FixturePtr CreateFixture(const b2FixtureDef *fixture_definition, const FixtureUserDataPtr &user_data = FixtureUserDataPtr());
		//! Creates a fixture
		/*!
		* The FixturePtr (wrapper) will be added to the fixture list held by
		* this entity
		*/
		FixturePtr CreateFixture(const b2FixtureDef *fixture_definition, const std::string &tag, const FixtureUserDataPtr &user_data = FixtureUserDataPtr());
		//! Destroys the given fixture
		/*!
		* The wrapper for the given fixture will also be removed from the list
		* held by this object
		*/
		void DestroyFixture(b2Fixture *inner);
		//! Destroys the given fixture
		/*!
		* The given wrapper will be removed, and it the inner b2Fixture will be
		* destroyed
		*/
		void DestroyFixture(const FixturePtr &wrapper);

		//! Sets the slot object that will destroy the b2Body when the PhysicalEntity is destroyed
		void SetBodyDestroyer(const BodyDestroyerPtr &destroyer);

		//! Save state to buffer
		virtual void SerialiseState(SerialisedData &state, bool local) const;
		//! Read state from buffer
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser);

	protected:
		b2Body *m_Body;

		typedef std::vector<FixturePtr> FixtureArray;
		FixtureArray m_Fixtures;

		//DestructorSignal m_DestructorSignal;
		BodyDestroyerPtr m_BodyDestroyer;

		// Vector2 format
		Vector2 m_Position;
		Vector2 m_Velocity;

		float m_Angle;
		float m_AngularVelocity;
	};

}

#endif