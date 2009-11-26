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

#ifndef Header_FusionEngine_PhysicalEntity
#define Header_FusionEngine_PhysicalEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionEntity.h"


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

	class PhysicalEntity : public Entity
	{
	public:
		PhysicalEntity();
		PhysicalEntity(const std::string &name);

		virtual ~PhysicalEntity();

		virtual void ApplyForce(const Vector2 &force, const Vector2 &point);
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

		bool IsPhysicsEnabled() const;

		b2Body *GetBody() const;
		void _setBody(b2Body *body);

		//! Sets the slot object that will destroy the b2Body when the PhysicalEntity is destroyed
		void SetBodyDestroyer(const BodyDestroyerPtr &destroyer);

		//! Save state to buffer
		virtual void SerialiseState(SerialisedData &state, bool local) const;
		//! Read state from buffer
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser);

	protected:
		//b2BodyDef m_BodyDef;
		b2Body *m_Body;

		//DestructorSignal m_DestructorSignal;
		BodyDestroyerPtr m_BodyDestroyer;

		// Vector2 format
		Vector2 m_Position;
		Vector2 m_Velocity;

		float m_Angle;
		float m_AngularVelocity;
	};

	typedef std::tr1::shared_ptr<PhysicalEntity> PhysicalEntityPtr;

}

#endif