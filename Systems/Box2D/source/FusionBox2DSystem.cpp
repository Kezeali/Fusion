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

#include "PrecompiledHeaders.h"

#include "FusionBox2DSystem.h"

#include "FusionBox2DComponent.h"
#include "FusionDeltaTime.h"
#include "FusionMaths.h"
#include "FusionBox2DContactListener.h"

// TEMP: For authority contact listener:
#include "FusionEntity.h"
#include "FusionPlayerRegistry.h"
#include "FusionNetworkManager.h"
//#include "FusionRender2DComponent.h"

#include <tbb/parallel_do.h>

namespace FusionEngine
{

	using namespace Maths;

	class Box2DContactListenerDelegator : public b2ContactListener
	{
	public:
		void AddListener(const std::shared_ptr<Box2DContactListener>& listener)
		{
			m_Listeners.insert(listener);
		}

		void RemoveListener(const std::shared_ptr<Box2DContactListener>& listener)
		{
			m_Listeners.erase(listener);
		}

	private:
		void BeginContact(b2Contact* contact)
		{
			for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
			{
				(*it)->BeginContact(contact);
			}
		}
		void EndContact(b2Contact* contact)
		{
			for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
			{
				(*it)->EndContact(contact);
			}
		}
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
		{
			for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
			{
				(*it)->PreSolve(contact, oldManifold);
			}
		}
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
		{
			for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
			{
				(*it)->PostSolve(contact, impulse);
			}
		}

		std::set<std::shared_ptr<Box2DContactListener>> m_Listeners;
	};

	Box2DSystem::Box2DSystem()
	{
	}

	std::shared_ptr<ISystemWorld> Box2DSystem::CreateWorld()
	{
		return std::make_shared<Box2DWorld>(this);
	}

	//class AuthorityContactFilter : public b2ContactFilter
	//{
	//public:
	//	bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
	//	{
	//		Box2DBody* bodyComA = static_cast<Box2DBody*>(fixtureA->GetBody()->GetUserData());
	//		Box2DBody* bodyComB = static_cast<Box2DBody*>(fixtureB->GetBody()->GetUserData());
	//		if (bodyComA && bodyComB)
	//		{
	//			auto ownerA = bodyComA->GetParent()->GetOwnerID(), ownerB = bodyComB->GetParent()->GetOwnerID();
	//			auto authA = bodyComA->GetParent()->GetAuthority(), authB = bodyComB->GetParent()->GetAuthority();
	//			authA = std::max(ownerA, authA); authB = std::max(ownerB, authB);
	//			if (authA != 0 && authB != 0 && !PlayerRegistry::IsLocal(authA) && !PlayerRegistry::IsLocal(authB))
	//			{
	//				return false;
	//			}
	//		}
	//		return true;
	//	}
	//};

#define FSN_PARALLEL_PROC_AUTH

	class TransformPinner : public Box2DContactListener
	{
	public:
		TransformPinner()
		{
		}

		std::map<b2Body*, b2MassData> pinnedBodies;

		// Prevent objects under another authority from jittering as they go to sleep
		//  due to out-of synch collisions
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
		{
			if (!contact->IsTouching())
				return;

			const auto bodyA = contact->GetFixtureA()->GetBody();
			const auto bodyB = contact->GetFixtureB()->GetBody();
			const auto bodyComA = static_cast<Box2DBody*>(bodyA->GetUserData());
			const auto bodyComB = static_cast<Box2DBody*>(bodyB->GetUserData());

			if (bodyComA && bodyComB)
			{
				if (bodyComA->IsPinned())
				//if (bodyComA->GetParent()->GetAuthority() != 0 && !PlayerRegistry::GetPlayer(bodyComA->GetParent()->GetAuthority()).IsLocal())
				{
					if (bodyB->GetLinearVelocity().Length() < b2_linearSleepTolerance)
					{
						contact->SetEnabled(false);
					}
				}
				else if (bodyComB->IsPinned())
				{
					if (bodyA->GetLinearVelocity().Length() < b2_linearSleepTolerance)
					{
						contact->SetEnabled(false);
					}
				}
			}

			//if (bodyComB)
			//{
			//	if (bodyComB->IsPinned())
			//	{
			//		auto manifold = contact->GetManifold();
			//		//manifold->localNormal
			//		for (int32 i = 0; i < manifold->pointCount; ++i)
			//			manifold->points[i].normalImpulse = 0;
			//	}
			//}
		}

		void Unpin()
		{
			for (auto it = pinnedBodies.begin(), end = pinnedBodies.end(); it != end; ++it)
			{
				it->first->SetMassData(&it->second);
			}
		}
	};

	class AuthorityContactManager
	{
	public:
		AuthorityContactManager()
		{
		}

		void WalkInteractions(PlayerID owner, const Box2DBody* bodyCom)
		{
#if FSN_ASSERTS_ENABLED == 1
			unsigned short sanity = 0;
#endif
			auto b2b = bodyCom->Getb2Body();
			std::deque<b2Body*> bodiesStack;
			while (b2b)
			{
				auto contactList = b2b->GetContactList();
				for (auto c = contactList; c != nullptr; c = c->next)
				{
					if (c->contact->IsTouching())
					{
						// AddInteraction(...) returns false if the other body is owned, thus
						//  breaking the interaction chain
						if (AddInteraction(owner, c->other))
							bodiesStack.push_back(c->other);
					}
				}
				if (!bodiesStack.empty())
				{
					b2b = bodiesStack.back();
					bodiesStack.pop_back();
				}
				else
					b2b = nullptr;
#if FSN_ASSERTS_ENABLED == 1
				FSN_ASSERT(++sanity != 0); // Break on rollover
#endif
			}
		}

		void ParseAuthority()
		{
			for (auto authIt = m_Interacting.begin(), authEnd = m_Interacting.end(); authIt != authEnd; ++authIt)
			{
				PlayerID authority = authIt->first;

				for (auto iactIt = authIt->second.begin(), iactEnd = authIt->second.end(); iactIt != iactEnd; ++iactIt)
				{
					const auto& bodyComB = *iactIt;
					PlayerID authB = bodyComB->GetParent()->GetAuthority();

					if (bodyComB->Getb2Body()->IsAwake())
					{
						if (authB == 0) // No current authority: take it
						{
							bodyComB->GetParent()->SetAuthority(authority);
						}
						else
						{
							// If there is a current authority for this body, check whether it is due to
							//  an active interation with a more senior player (if not, take authority)
							const auto& bInteractions = m_Interacting[authB];
							bool activeInteraction = bInteractions.find(bodyComB) != bInteractions.end();

							if (/*!activeInteraction || */NetworkManager::IsSenior(PlayerRegistry::GetPlayer(authority), PlayerRegistry::GetPlayer(authB)))
								bodyComB->GetParent()->SetAuthority(authority);
						}
					}
				}
			}
		}

		void ClearInteractions()
		{
			m_Interacting.clear();
		}

	private:
		std::map<PlayerID, std::set<Box2DBody*>> m_Interacting;

		bool AddInteraction(PlayerID auth, b2Body* otherBody)
		{
			Box2DBody* bodyComB = static_cast<Box2DBody*>(otherBody->GetUserData());
			if (bodyComB)
			{
				PlayerID ownerB = bodyComB->GetParent()->GetOwnerID();
				PlayerID authB = bodyComB->GetParent()->GetAuthority();
				if ((ownerB == 0 || ownerB == auth) && (authB == 0 || authB == auth)) // Break interaction chains at owned bodies
				{
					return m_Interacting[auth].insert(bodyComB).second; // Don't follow paths already checked
				}
			}
			return false;
		}
	};

	Box2DWorld::Box2DWorld(IComponentSystem* system)
		: ISystemWorld(system)
	{
		b2Vec2 gravity(0.0f, 0.0f);
		m_World = new b2World(gravity);

		m_AuthorityContactManager = new AuthorityContactManager();

		m_ContactListenerDelegator = new Box2DContactListenerDelegator();
		m_World->SetContactListener(m_ContactListenerDelegator);

		//m_TransformPinner = std::make_shared<TransformPinner>();
		//m_ContactListenerDelegator->AddListener(m_TransformPinner);

		m_B2DTask = new Box2DTask(this, m_World);
		m_B2DInterpTask = new Box2DInterpolateTask(this);
	}

	Box2DWorld::~Box2DWorld()
	{
		m_ActiveBodies.clear();
		m_BodiesToCreate.clear();
		delete m_B2DInterpTask;
		delete m_B2DTask;
		delete m_World;
		delete m_ContactListenerDelegator;

		delete m_AuthorityContactManager;
	}

	void Box2DWorld::AddContactListener(const std::shared_ptr<Box2DContactListener>& listener)
	{
		m_ContactListenerDelegator->AddListener(listener);
	}

	void Box2DWorld::RemoveContactListener(const std::shared_ptr<Box2DContactListener>& listener)
	{
		m_ContactListenerDelegator->RemoveListener(listener);
	}

	std::vector<std::string> Box2DWorld::GetTypes() const
	{
		//using namespace std::placeholders;
		static const std::string types[] = { "b2RigidBody", "b2Dynamic", "b2Kinematic", "b2Static", "b2Circle", "b2Polygon", "StaticTransform" };
		//std::vector<ISystemWorld::ComponentType> types;
		//types.push_back(ComponentType("b2RigidBody", PositionSerialiser(std::bind(&Box2DBody::SerialisePosition, _1, _2, _3, _4), PositionSerialisationFunctor())));
		//return types;
		return std::vector<std::string>(types, types + 7);
	}

	ComponentPtr Box2DWorld::InstantiateComponent(const std::string& type)
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

		//void SerialiseContinuous(RakNet::BitStream& stream)
		//{
		//	return false;
		//}
		//void DeserialiseContinuous(RakNet::BitStream& stream)
		//{
		//}
		void SerialiseOccasional(RakNet::BitStream& stream)
		{
			//stream.Write(m_Position.x);
			//stream.Write(m_Position.y);
			stream.Write(m_Angle);
			
			stream.Write(m_Depth);
		}
		void DeserialiseOccasional(RakNet::BitStream& stream)
		{
			//stream.Read(m_Position.x);
			//stream.Read(m_Position.y);
			stream.Read(m_Angle);

			stream.Read(m_Depth);
		}

		bool HasContinuousPosition() const { return false; }

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

	ComponentPtr Box2DWorld::InstantiateComponent(const std::string& type, const Vector2& pos, float angle)
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

			auto com = new Box2DBody(def);
			//com->SetInterpolate(def.type != b2_staticBody);
			return com;
		}
		else if (type == "b2Circle")
		{
			return new Box2DCircleFixture();
		}
		else if (type == "b2Polygon")
		{
			return new Box2DPolygonFixture();
		}
		else if (type == "StaticTransform")
		{
			return new StaticTransform(pos, angle);
		}
		return ComponentPtr();
	}

	void Box2DWorld::OnActivation(const ComponentPtr& component)
	{
		if (auto bodyComponent = boost::dynamic_pointer_cast<Box2DBody>(component))
		{
			if (bodyComponent->m_Body != nullptr)
				m_ActiveBodies.push_back(bodyComponent);
			else
				m_BodiesToCreate.push_back(bodyComponent); // make sure to create the b2Body before trying to update it!
			//b2Component->SetActive(true);
		}
		else if (auto polygonComponent = boost::dynamic_pointer_cast<Box2DPolygonFixture>(component))
		{
			// TODO: maybe do this instead? polygonComponent->SetVertsChangedCallback(bind(onvertschanged, this, _1)); onvertschanged: enqueue to re-create the fixture
			m_PolygonFixtures.push_back(polygonComponent);
		}
	}

	void Box2DWorld::OnDeactivation(const ComponentPtr& component)
	{
		if (auto b2Component = boost::dynamic_pointer_cast<Box2DBody>(component))
		{
			b2Component->DestructBody(m_World);
			// Deactivate the body in the simulation
			//b2Component->SetActive(false);
			
			// Find and remove the deactivated body (from the Active Bodies list)
			bool removed = false;
			{
				auto _where = std::find(m_ActiveBodies.begin(), m_ActiveBodies.end(), b2Component);
				if (_where != m_ActiveBodies.end())
				{
					m_ActiveBodies.erase(_where);
					removed = true;
				}
			}
			if (!removed)
			{
				auto _where = std::find(m_BodiesToCreate.begin(), m_BodiesToCreate.end(), b2Component);
				if (_where != m_BodiesToCreate.end())
				{
					m_BodiesToCreate.erase(_where);
				}
			}
		}
		else if (auto polygonComponent = boost::dynamic_pointer_cast<Box2DPolygonFixture>(component))
		{
			auto _where = std::find(m_PolygonFixtures.begin(), m_PolygonFixtures.end(), polygonComponent);
			if (_where != m_PolygonFixtures.end())
				m_PolygonFixtures.erase(_where);
		}
	}

	std::vector<ISystemTask*> Box2DWorld::GetTasks()
	{
		std::vector<ISystemTask*> tasks(2);
		tasks[0] = m_B2DTask;
		tasks[1] = m_B2DInterpTask;
		return tasks;
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

	void Box2DWorld::InitialiseActiveComponents()
	{
		for (auto it = m_BodiesToCreate.begin(), end = m_BodiesToCreate.end(); it != end; ++it)
		{
			auto& body = *it;
			body->ConstructBody(m_World, this->shared_from_this());
		}

		// Copy the newly-created bodies into the active list:
		m_ActiveBodies.insert(m_ActiveBodies.end(), m_BodiesToCreate.begin(), m_BodiesToCreate.end());
		m_BodiesToCreate.clear();

		for (auto it = m_PolygonFixtures.begin(), end = m_PolygonFixtures.end(); it != end; ++it)
		{
			auto& fixture = *it;
			fixture->Update();
		}
	}

	void Box2DTask::Update(const float delta)
	{
		// Late initialisation
		m_B2DSysWorld->InitialiseActiveComponents();

		const auto& activeBodies = m_B2DSysWorld->m_ActiveBodies;

		// Update interpolation and mass data
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
			}

			// Update the mass data based on any fixture changes
			body->CleanMassData();
		}

		// I'm glad I don't have to worry about what goes on in here :)
		m_World->Step(delta, 8, 8);
		m_World->ClearForces();

		// Setup property synch by marking them as changed and
		//  process interactions which affect authority (network sync stuff)
		m_B2DSysWorld->m_AuthorityContactManager->ClearInteractions();
		for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		{
			auto body = *it;
			const bool awake = body->IsAwake();
			const bool staticBody = body->GetBodyType() == IRigidBody::Static;
			if (!staticBody)
			{
				body->CompressState();

				if (awake != body->Awake.Get())
				{
					body->Awake.MarkChanged();
					//body->m_DeltaSerialisationHelper.markChanged(Box2DBody::PropsIdx::Awake);
				}
				if (awake)
				{
					body->Position.MarkChanged();
					body->Angle.MarkChanged();
					body->Velocity.MarkChanged();
					body->AngularVelocity.MarkChanged();

					const PlayerID authority = /*body->GetParent()->GetOwnerID();*/((body->GetParent()->GetOwnerID() != 0) ? body->GetParent()->GetOwnerID() : body->GetParent()->GetAuthority());
					if (authority != 0)
					{
						m_B2DSysWorld->m_AuthorityContactManager->WalkInteractions(authority, body.get());
					}
				}
				else
				{
					// Remove authority on sleeping bodies
					//if (!body->IsInteractingWithPlayer())
					{
						auto owner = body->GetParent()->GetOwnerID();
						auto auth = body->GetParent()->GetAuthority();

						if (owner == 0 && auth != 0)
						{
							body->GetParent()->SetAuthority(0);
						}
					}
				}
			}
		}
		// Clear pinning
		for (auto it = activeBodies.begin(), end = activeBodies.end(); it != end; ++it)
		{
			auto& body = *it;
			body->m_PinTransform = false;
		}
		//m_B2DSysWorld->m_TransformPinner->Unpin();

		m_B2DSysWorld->m_AuthorityContactManager->ParseAuthority();
		m_B2DSysWorld->m_AuthorityContactManager->ClearInteractions();
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
	//void Lerp(T& out, const T& start, const T& end, float alpha)
	//{
	//	out = start * (1 - alpha) + end * alpha;
	//}

	//static void Wrap(float& value, float lower, float upper)
	//{ 
	//	float distance = upper - lower;
	//	float times = std::floor((value - lower) / distance);
	//	value -= (times * distance);
	//} 

	//static void AngleInterpB(float& out, float start, float velocity, float accel, float dt, float alpha)
	//{
	//	//    x0    +      v * time        +  0.5f *  a * time ^ 2
	//	out = start + velocity * dt * alpha + 0.5f * accel * dt * dt * alpha;
	//}

	//static void AngleInterpB(float& out, float start, float end, float vel, float alpha)
	//{
	//	float diff = end - start;
	//	if (vel * diff < 0.f)
	//		diff = b2_pi * 2.f - diff;
	//	out = start + diff * alpha;
	//}

	//static void AngleInterp(float& out, float start, float end, float alpha)
	//{
	//	if (std::abs(end - start) < b2_pi)
	//	{
	//		Lerp(out, start, end, alpha);
	//		return;
	//	}

	//	if (start < end)
	//		start += b2_pi * 2.f;
	//	else
	//		end += b2_pi * 2.f;
	//	
	//	Lerp(out, start, end, alpha);
	//}

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
