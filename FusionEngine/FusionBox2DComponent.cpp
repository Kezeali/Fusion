/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionBox2DComponent.h"

namespace FusionEngine
{

	void Box2DBody::SynchTransform()
	{
		Vector2 position;
		bool positionWritten = m_Position.ClearWrittenValue(position);

		if (m_AngleWritten && positionWritten)
		{
			m_Body->SetTransform(ToSim(position), m_Angle);
		}
		else if (positionWritten)
		{
			m_Body->SetTransform(ToSim(position), m_Body->GetAngle());
			m_Angle = m_Body->GetAngle();
		}
		else if (m_AngleWritten)
		{
			m_Body->SetTransform(m_Body->GetPosition(), m_Angle);
			m_Position.SetReadValue(ToRender(m_Body->GetPosition()));
		}
		
		m_AngleWritten = false;
	}

	void Box2DBody::SynchroniseParallelEdits()
	{
		SynchTransform();

		Vector2 velocity;
		if (m_Velocity.ClearWrittenValue(velocity))
			m_Body->SetLinearVelocity(velocity);
		else
			m_Velocity.SetReadValue(ToRender(m_Body->GetLinearVelocity()));

		if (m_AngularVelocityWritten)
		{
			m_Body->SetAngularVelocity(m_AngularVelocity);
			m_AngularVelocityWritten = false;
		}
		else
			m_AngularVelocity = m_Body->GetAngularVelocity();

		if (m_LinearDampingWritten)
		{
			m_Body->SetLinearDamping(ToSim(m_LinearDamping));
			m_LinearDampingWritten = false;
		}

		if (m_AngularDampingWritten)
		{
			m_Body->SetAngularDamping(m_AngularDamping);
			m_AngularDampingWritten = false;
		}

		if (m_GravityScaleWritten)
		{
			m_Body->SetGravityScale(m_GravityScale);
			m_GravityScaleWritten = false;
		}

		// Copy flags that have been set on the threadsafe wrapper
		if (m_Active != m_Body->IsActive())
			m_Body->SetActive(m_Active);

		if (m_SleepingAllowed != m_Body->IsSleepingAllowed())
			m_Body->SetSleepingAllowed(m_SleepingAllowed);

		if (m_Bullet != m_Body->IsBullet())
			m_Body->SetBullet(m_Bullet);

		if (m_FixedRotation != m_Body->IsFixedRotation())
			m_Body->SetFixedRotation(m_FixedRotation);

		// Copy the value of IsAwake
		m_Awake = m_Body->IsAwake();
	}

	bool Box2DBody::SerialiseContinuous(RakNet::BitStream& stream)
	{
		const Vector2& pos = GetPosition();
		stream.Write(pos.x);
		stream.Write(pos.y);
		stream.Write(GetAngle());

		const Vector2& vel = GetVelocity();
		stream.Write(vel.x);
		stream.Write(vel.y);
		stream.Write(GetAngularVelocity());

		return true;
	}

	void Box2DBody::DeserialiseContinuous(RakNet::BitStream& stream)
	{
		Vector2 position;
		float angle;
		stream.Read(position.x);
		stream.Read(position.y);
		stream.Read(angle);

		SetPosition(position);
		SetAngle(angle);

		Vector2 linearVelocity;
		float angularVelocity;
		stream.Read(linearVelocity.x);
		stream.Read(linearVelocity.x);
		stream.Read(angularVelocity);

		SetVelocity(linearVelocity);
		SetAngularVelocity(angularVelocity);
	}

	template <typename T>
	static inline bool writeChange(bool force_all, RakNet::BitStream& stream, T& old_value, const T& new_value)
	{
		if (force_all || new_value != old_value)
		{
			if (!force_all)
				stream.Write1();
			stream.Write(new_value);
			old_value = new_value;
			return true;
		}
		else
		{
			stream.Write0();
			return false;
		}
	}
	
	template <>
	static inline bool writeChange(bool force_all, RakNet::BitStream& stream, bool& old_value, const bool& new_value)
	{
		stream.Write(new_value);
		if (force_all || new_value != old_value)
		{
			old_value = new_value;
			return true;
		}
		else
			return false;
	}

	template <typename T>
	static inline bool readChange(bool all, RakNet::BitStream& stream, T& new_value)
	{
		if (all || stream.ReadBit())
		{
			stream.Read(new_value);
			return true;
		}
		else
			return false;
	}

	template <>
	static inline bool readChange(bool all, RakNet::BitStream& stream, bool& new_value)
	{
		stream.Read(new_value);
		return true;
	}

	template <typename T>
	static void copyChange(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		T value;
		if (readChange(false, new_data, value))
		{
			stream.Write(value);
			current_data.IgnoreBytes(sizeof(T));
		}
		else if (readChange(true, current_data, value))
			stream.Write(value);
	}

	bool Box2DBody::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		//uint16_t occasionalChangeFlags;

		bool changeWritten = false;

		changeWritten |= writeChange(force_all, stream, m_SerialisedActiveValue, IsActive());
		changeWritten |= writeChange(force_all, stream, m_SerialisedActiveValue, IsSleepingAllowed());
		changeWritten |= writeChange(force_all, stream, m_SerialisedAwakeValue, IsAwake());
		changeWritten |= writeChange(force_all, stream, m_SerialisedBulletValue, IsBullet());
		changeWritten |= writeChange(force_all, stream, m_SerialisedFixedRotationValue, IsFixedRotation());

		changeWritten |= writeChange(force_all, stream, m_SerialisedLinearDampingValue, GetLinearDamping());
		changeWritten |= writeChange(force_all, stream, m_SerialisedAngularDampingValue, GetAngularDamping());
		changeWritten |= writeChange(force_all, stream, m_SerialisedGravityScaleValue, GetGravityScale());

		return changeWritten;
	}

	void Box2DBody::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		if (readChange(all, stream, m_SerialisedActiveValue))
			m_Body->SetActive(m_SerialisedActiveValue);
		if (readChange(all, stream, m_SerialisedSleepingAllowedValue))
			m_Body->SetSleepingAllowed(m_SerialisedSleepingAllowedValue);
		if (readChange(all, stream, m_SerialisedAwakeValue))
			m_Body->SetAwake(m_SerialisedAwakeValue);
		if (readChange(all, stream, m_SerialisedBulletValue))
			m_Body->SetBullet(m_SerialisedBulletValue);
		if (readChange(all, stream, m_SerialisedFixedRotationValue))
			m_Body->SetFixedRotation(m_SerialisedFixedRotationValue);

		if (readChange(all, stream, m_SerialisedLinearDampingValue))
			m_Body->SetLinearDamping(m_SerialisedLinearDampingValue);
		if (readChange(all, stream, m_SerialisedAngularDampingValue))
			m_Body->SetAngularDamping(m_SerialisedAngularDampingValue);
		if (readChange(all, stream, m_SerialisedGravityScaleValue))
			m_Body->SetGravityScale(m_SerialisedGravityScaleValue);
	}

	void Box2DBody::MergeDelta(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
	{
		copyChange<bool>(result, current_data, new_data); // Active
		copyChange<bool>(result, current_data, new_data); // Sleeping allowed
		copyChange<bool>(result, current_data, new_data); // Awake
		copyChange<bool>(result, current_data, new_data); // Bullet
		copyChange<bool>(result, current_data, new_data); // FixedRotation

		copyChange<float>(result, current_data, new_data); // LinearDamping
		copyChange<float>(result, current_data, new_data); // AngularDamping
		copyChange<float>(result, current_data, new_data); // GravityScale
	}

	void Box2DFixture::SynchroniseParallelEdits()
	{
		if (m_SensorProp.written != m_Fixture->IsSensor())
			m_Fixture->SetSensor(m_SensorProp.written);

		if (m_DensityProp.changed)
		{
			m_Fixture->SetDensity(m_DensityProp.written);
			m_DensityProp.changed = false;
		}

		if (m_FrictionProp.changed)
		{
			m_Fixture->SetFriction(m_FrictionProp.written);
			m_FrictionProp.changed = false;
		}

		if (m_RestitutionProp.changed)
		{
			m_Fixture->SetRestitution(m_RestitutionProp.written);
			m_RestitutionProp.changed = false;
		}
	}

	bool Box2DFixture::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return false;
	}

	void Box2DFixture::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool Box2DFixture::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		bool changeWritten = false;
		changeWritten = writeChange(force_all, stream, m_SensorProp.serialised, m_Fixture->IsSensor());
		changeWritten = writeChange(force_all, stream, m_DensityProp.serialised, m_Fixture->GetDensity());
		changeWritten = writeChange(force_all, stream, m_FrictionProp.serialised, m_Fixture->GetFriction());
		changeWritten = writeChange(force_all, stream, m_RestitutionProp.serialised, m_Fixture->GetRestitution());

		return changeWritten;
	}

	void Box2DFixture::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		if (readChange(all, stream, m_SensorProp.serialised))
			m_Fixture->SetSensor(m_SensorProp.serialised);
		if (readChange(all, stream, m_DensityProp.serialised))
			m_Fixture->SetDensity(m_DensityProp.serialised);
	}

}