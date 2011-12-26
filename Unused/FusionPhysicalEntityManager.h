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

#ifndef H_FusionEngine_PhysicsManager
#define H_FusionEngine_PhysicsManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicalEntity.h"
#include "FusionPhysicsCallback.h"
#include "FusionPhysicsDebugDraw.h"

#include "FusionRefCounted.h"

#include "FusionSingleton.h"


namespace FusionEngine
{

	static const int s_PhysicsVelocityIterations = 10;
	static const int s_PhysicsPositionIterations = 8;
	
	class ContactListener : public b2ContactListener
	{
	public:
		ContactListener();

		virtual void BeginContact(b2Contact* contact);
		virtual void EndContact(b2Contact* contact);
		virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		virtual void PostSolve(const b2Contact* contact, const b2ContactImpulse* impulse);
	};

	class ContactFilter : public b2ContactFilter
	{
	public:
		ContactFilter(PhysicalWorld *world);

		virtual bool ShouldCollide(b2Fixture *shape1, b2Fixture *shape2);
		virtual bool RayCollide(void *userData, b2Fixture *shape);

	protected:
		PhysicalWorld *m_Manager;
	};

	class PhysicalWorld : public Singleton<PhysicalWorld>
	{
	public:
		PhysicalWorld();

		~PhysicalWorld();

		//! Sets the GC for the debug draw object to use
		void SetGraphicContext(const CL_GraphicContext &context);
		//! Sets the viewport within which the debug info. should be drawn
		void SetDebugDrawViewport(const ViewportPtr &viewport);
		//! Adds a debug draw impl. object to the b2World
		void EnableDebugDraw(bool enable = true);

		b2World *GetB2World() const;

		void Step(float split);

		void OnEntityDestruction(b2Body *body);

		void PrepareEntity(PhysicalEntity *entity);
		//void RemoveEntity(const PhysicalEntityPtr &entity);

		//void Clear();

		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnBeginContact(b2Contact *contact);
		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnEndContact(b2Contact *contact);
		//! Called via contact listener
		void OnPreSolve(b2Contact* contact, const b2Manifold* oldManifold);
		//! Called automatically by Box2D during RunSimulation (via a contact listener)
		void OnPostSolve(const b2Contact *contact, const b2ContactImpulse *impulse);

	protected:
		b2World *m_World;

		int m_VelocityIterations;
		int m_PositionIterations;

		ContactListener *m_ContactListener;
		B2DebugDraw *m_DebugDraw;

		//typedef std::vector<boost::signals2::connection> ConnectionList;

		//ConnectionList m_Connections;

		BodyDestroyerPtr m_BodyDestroyer;

		void initialise();
	};

}

#endif