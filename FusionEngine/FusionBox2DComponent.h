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

#ifndef H_FusionBox2DComponent
#define H_FusionBox2DComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionPhysicalComponent.h"

#include <Box2D/Box2D.h>
#include <tbb/atomic.h>

namespace FusionEngine
{

	class Box2DBody : public IComponent, public IPhysicalProperties, public IPhysicalMethods
	{
		friend class Box2DSystem;
	public:
		typedef boost::mpl::vector<IPhysicalProperties, IPhysicalMethods>::type Interfaces;

	private:
		Box2DBody(b2Body* body)
			: m_Body(body)
		{
		}

		b2Body* m_Body;

		// IComponent
		std::string GetType() const { return "Box2DBody"; }

		void SynchroniseParallelEdits()
		{
			SynchTransform();

			Vector2 velocity;
			if (m_Velocity.GetWrittenValue(velocity))
				m_Body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
			if (m_AngularVelocityChanged)
				m_Body->SetAngularVelocity(m_AngularVelocity);
		}

		// Threadsafe interface
		ValueBuffer<Vector2> m_Position;
		const Vector2& GetPosition() const { return m_Position.Get(); }
		void SetPosition(const Vector2& position) { m_Position.Set(position); }

		tbb::atomic<float> m_Angle;
		tbb::atomic<bool> m_AngleChanged;
		float GetAngle() const { return m_Angle; }
		void SetAngle(float angle) { m_Angle = angle; m_AngleChanged = true; }

		void SynchTransform()
		{
			Vector2 position;
			if (m_AngleChanged && m_Position.GetWrittenValue(position))
				m_Body->SetTransform(b2Vec2(position.x, position.y), m_Angle);
			m_AngleChanged = false;
		}

		ValueBuffer<Vector2> m_Velocity;
		const Vector2&  GetVelocity() const { return m_Velocity.Get(); }
		void SetVelocity(const Vector2& vel) { m_Velocity.Set(vel); }

		tbb::atomic<float> m_AngularVelocity;
		tbb::atomic<bool> m_AngularVelocityChanged;
		float GetAngularVelocity() const { return m_AngularVelocity; }
		void SetAngularVelocity(float vel) { m_AngularVelocity = vel; m_AngularVelocityChanged = true; }

		// Non-threadsafe interface
		void ApplyForce(const Vector2& force, const Vector2& point)
		{
			m_Body->ApplyForce(b2Vec2(force.x, force.y), b2Vec2(point.x, point.y));
		}

		void ApplyForce(const Vector2& force)
		{
			m_Body->ApplyForce(b2Vec2(force.x, force.y), m_Body->GetLocalCenter());
		}
	};

	class Box2DFixture : public IComponent, public IFixtureProperties
	{
		friend class Box2DSystem;
	public:
		typedef boost::mpl::vector<IFixtureProperties>::type Interfaces;

	private:

		b2Fixture* m_Fixture;
	};

}

#endif
