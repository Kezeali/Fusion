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

#include "FusionBox2DSystem.h"

#include "FusionBox2DComponent.h"

namespace FusionEngine
{

	Box2DSystem::Box2DSystem()
	{
	}

	std::shared_ptr<ISystemWorld> Box2DSystem::CreateWorld()
	{
		return std::make_shared<Box2DWorld>(this);
	}

	Box2DWorld::Box2DWorld(IComponentSystem* system)
		: ISystemWorld(system)
	{
		b2Vec2 gravity(0.0f, 0.0f);
		m_World = new b2World(gravity, true);

		m_B2DTask = new Box2DTask(this, m_World);
		m_B2DInterpTask = new Box2DInterpolateTask(this);
	}

	Box2DWorld::~Box2DWorld()
	{
		delete m_B2DInterpTask;
		delete m_B2DTask;
		delete m_World;
	}

	std::vector<std::string> Box2DWorld::GetTypes() const
	{
		static const std::string types[] = { "b2RigidBody", "b2Dynamic", "b2Kinematic", "b2Static", "b2Circle", "b2Polygon", "StaticTransform" };
		return std::vector<std::string>(types, types + 7);
	}

	std::shared_ptr<IComponent> Box2DWorld::InstantiateComponent(const std::string& type)
	{
		return InstantiateComponent(type, Vector2::zero(), 0.f);
	}

	class StaticTransform : public IComponent, public ITransform
	{
	public:
		FSN_LIST_INTERFACES((ITransform))

		StaticTransform()
			: m_Angle(0.0f),
			m_Depth(0)
		{}

		StaticTransform(const Vector2& pos, float angle)
			: m_Position(pos),
			m_Angle(angle),
			m_Depth(0)
		{}

	private:
		std::string GetType() const { return "StaticTransform"; }

		//bool SerialiseContinuous(RakNet::BitStream& stream)
		//{
		//	return false;
		//}
		//void DeserialiseContinuous(RakNet::BitStream& stream)
		//{
		//}
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
		{
			stream.Write(m_Position.x);
			stream.Write(m_Position.y);
			stream.Write(m_Angle);
			
			stream.Write(m_Depth);
			return true;
		}
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
		{
			stream.Read(m_Position.x);
			stream.Read(m_Position.y);
			stream.Read(m_Angle);

			stream.Read(m_Depth);

			//Position.MarkChanged();
			//Angle.MarkChanged();
		}

		Vector2 GetPosition() const { return m_Position; }
		void SetPosition(const Vector2& position) { m_Position = position; }

		float GetAngle() const { return m_Angle; }
		void SetAngle(float angle) { m_Angle = angle; }

		int GetDepth() const { return m_Depth; }
		void SetDepth(int depth) { m_Depth = depth; }

		Vector2 m_Position;
		float m_Angle;
		int m_Depth;
	};

	std::shared_ptr<IComponent> Box2DWorld::InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
	{
		if (type == "b2RigidBody" 
			|| type == "b2Dynamic"
			|| type == "b2Kinematic"
			|| type == "b2Static")
		{
			b2BodyDef def;

			if (type == "b2Kinematic")
				def.type = b2_kinematicBody;
			else if (type == "b2Static")
				def.type = b2_staticBody;
			else
				def.type = b2_dynamicBody;

			def.position.Set(pos.x, pos.y);
			def.angle = angle;

			auto com = std::make_shared<Box2DBody>(def);
			com->SetInterpolate(def.type != b2_staticBody);
			return com;
		}
		else if (type == "b2Circle")
		{
			return std::make_shared<Box2DCircleFixture>();
		}
		else if (type == "b2Polygon")
		{
			return std::make_shared<Box2DPolygonFixture>();
		}
		else if (type == "StaticTransform")
		{
			return std::make_shared<StaticTransform>(pos, angle);
		}
		return std::shared_ptr<IComponent>();
	}

	void Box2DWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		auto b2Component = std::dynamic_pointer_cast<Box2DBody>(component);
		if (b2Component)
		{
			if (b2Component->m_Body != nullptr)
				m_ActiveBodies.push_back(b2Component);
			else
				m_BodiesToCreate.push_back(b2Component); // make sure to create the b2Body before trying to update it!
			b2Component->SetActive(true);
		}
	}

	void Box2DWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto b2Component = std::dynamic_pointer_cast<Box2DBody>(component);
		if (b2Component)
		{
			// Deactivate the body in the simulation
			b2Component->SetActive(false);
			// Find and remove the deactivated body (from the Active Bodies list)
			{
			auto _where = std::find(m_ActiveBodies.begin(), m_ActiveBodies.end(), b2Component);
			if (_where != m_ActiveBodies.end())
			{
				_where->swap(m_ActiveBodies.back());
				m_ActiveBodies.pop_back();
				return;
			}
			}
			{
			auto _where = std::find(m_BodiesToCreate.begin(), m_BodiesToCreate.end(), b2Component);
			if (_where != m_BodiesToCreate.end())
			{
				_where->swap(m_BodiesToCreate.back());
				m_BodiesToCreate.pop_back();
			}
			}
		}
	}

	std::vector<ISystemTask*> Box2DWorld::GetTasks()
	{
		std::vector<ISystemTask*> tasks(2);
		tasks[0] = m_B2DTask;
		tasks[1] = m_B2DInterpTask;
		return tasks;
	}

	void Box2DWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
		if (type == "b2RigidBody" 
			|| type == "b2Dynamic"
			|| type == "b2Kinematic"
			|| type == "b2Static")
		{
			Box2DBody::DeltaSerialiser_t::copyChanges(result, current_data, delta);
		}
		else if (type == "b2Circle")
		{
			Box2DCircleFixture::CopyChanges(result, current_data, delta);
		}
	}

	Box2DTask::Box2DTask(Box2DWorld* sysworld, b2World* const world)
		: ISystemTask(sysworld),
		m_B2DSysWorld(sysworld),
		m_World(world)
	{
	}

	Box2DTask::~Box2DTask()
	{
	}

	void Box2DTask::Update(const float delta)
	{
		auto& toCreate = m_B2DSysWorld->m_BodiesToCreate;
		for (auto it = toCreate.begin(), end = toCreate.end(); it != end; ++it)
		{
			auto body = *it;
			body->ConstructBody(m_World);
		}

		auto& activeBodies = m_B2DSysWorld->m_ActiveBodies;
		// Copy the newly-created bodies into the active list:
		activeBodies.insert(activeBodies.end(), toCreate.begin(), toCreate.end());
		toCreate.clear();

		for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		{
			auto body = *it;

			const bool awake = body->IsAwake();
			const bool staticBody = body->GetBodyType() == IRigidBody::Static;
			if (!staticBody && awake && body->m_Interpolate)
			{
						const auto& tf = body->m_Body->GetTransform();
						body->m_LastPosition = b2v2(tf.p);
						body->m_LastAngle = tf.q.GetAngle();
						body->m_LastAngularVelocity = body->m_Body->GetAngularVelocity();
			}
		}

		m_World->Step(delta, 8, 8);
		m_World->ClearForces();
		//auto& activeBodies = m_B2DSysWorld->m_ActiveBodies;
		for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		{
			auto body = *it;
			const bool awake = body->IsAwake();
			const bool staticBody = body->GetBodyType() == IRigidBody::Static;
			if (!staticBody)
			{
				if (awake != body->Awake.Get())
				{
					body->Awake.MarkChanged();
					body->m_DeltaSerialisationHelper.markChanged(Box2DBody::PropsIdx::Awake);
				}
				if (awake)
				{
					//if (body->m_Interpolate)
					//{
					//	const auto& tf = body->m_Body->GetTransform();
					//	body->m_LastPosition = b2v2(tf.p);
					//	body->m_LastAngle = tf.q.GetAngle();
					//}
					
					body->Position.MarkChanged();
					body->Angle.MarkChanged();
					body->Velocity.MarkChanged();
					body->AngularVelocity.MarkChanged();
				}
			}

			body->CleanMassData();
		}
	}

	Box2DInterpolateTask::Box2DInterpolateTask(Box2DWorld* sysworld)
		: ISystemTask(sysworld),
		m_B2DSysWorld(sysworld)
	{
	}

	Box2DInterpolateTask::~Box2DInterpolateTask()
	{
	}

	//template <typename T>
	//void Lerp(T& out, const T& start, const T& end, float m)
	//{
	//	out = start + m * (end - start);
	//}

	template <typename T>
	void Lerp(T& out, const T& start, const T& end, float alpha)
	{
		out = start * (1 - alpha) + end * alpha;
	}

	static void Wrap(float& value, float lower, float upper)
	{ 
		float distance = upper - lower;
		float times = std::floor((value - lower) / distance);
		value -= (times * distance);
	} 

	static void AngleInterpB(float& out, float start, float velocity, float accel, float dt, float alpha)
	{
		//    x0    +      v * time        +  0.5f *  a * time ^ 2
		out = start + velocity * dt * alpha + 0.5f * accel * dt * dt * alpha;
	}

	static void AngleInterpB(float& out, float start, float end, float vel, float alpha)
	{
		float diff = end - start;
		if (vel * diff < 0.f)
			diff = b2_pi * 2.f - diff;
		out = start + diff * alpha;
	}

	static void AngleInterp(float& out, float start, float end, float alpha)
	{
		if (std::abs(end - start) < b2_pi)
		{
			Lerp(out, start, end, alpha);
			return;
		}

		if (start < end)
			start += b2_pi * 2.f;
		else
			end += b2_pi * 2.f;
		
		Lerp(out, start, end, alpha);
	}

	void Box2DInterpolateTask::Update(const float delta)
	{
		FSN_ASSERT(DeltaTime::GetInterpolationAlpha() <= 1.0f);
		auto& activeBodies = m_B2DSysWorld->m_ActiveBodies;
		for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		{
			auto body = *it;
			const bool awake = body->IsAwake();
			const bool staticBody = body->GetBodyType() == IRigidBody::Static;
			const bool interpolate = body->GetInterpolate();
			if (!staticBody && awake && interpolate)
			{
				const auto& tf = body->m_Body->GetTransform();
				const float angularVelocity = body->m_Body->GetAngularVelocity();

				Lerp(body->m_InterpPosition, body->m_LastPosition, b2v2(tf.p), DeltaTime::GetInterpolationAlpha());
				AngleInterp(body->m_InterpAngle, body->m_LastAngle, tf.q.GetAngle(), DeltaTime::GetInterpolationAlpha());

				body->Position.MarkChanged();
				body->Angle.MarkChanged();
			}
		}
	}

}
