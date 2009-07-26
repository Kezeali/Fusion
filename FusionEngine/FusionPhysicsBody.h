/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#ifndef Header_FusionEngine_PhysicsBody
#define Header_FusionEngine_PhysicsBody

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionRefCounted.h"
/// Fusion
#include "FusionPhysicsShape.h"
#include "FusionPhysicsCallback.h"


namespace FusionEngine
{
	//! Collision type flags
	/*!
	 * Flags used by FusionPhysicsWorld to decide what type of collision
	 * detection/response to use.
	 */
	//enum CollisionFlags
	//{
	//	//! (0000)
	//	C_NONE = 0,
	//	//! (0001)
	//	C_STATIC = 1,
	//	//! (0010)
	//	C_BOUNCE = 2
	//};

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

	static float g_PhysStaticMass = 0.f;
	static int g_PhysBodyCpCollisionType = 1;

	class PhysicsBody;
	typedef boost::intrusive_ptr<PhysicsBody> PhysicsBodyPtr;


	/*!
	 * \brief
	 * The basis for movable/colliding objects.
	 *
	 * \see
	 * PhysicsWorld.
	 */
	class PhysicsBody : public RefCounted
	{
		friend class PhysicsWorld;

		//typedef boost::ptr_list<Shape> ShapeList;
		typedef std::list<ShapePtr> ShapeList;
		//typedef std::list<Shape*> ShapeList;

		typedef std::vector<FixturePtr> FixturePtrArray;

	public:
		PhysicsBody();
		//PhysicsBody();
		//! Constructor.
		/*!
		 * \param world
		 * The world in which this body resides.
		 */
		//PhysicsBody(PhysicsWorld *world);

		//! Constructor with handler.
		/*!
		 * \param[in] world
		 * The world in which this body resides.
		 *
		 * \param[in] handler
		 * The collision response object.
		 */
		//PhysicsBody(PhysicsWorld *world, ICollisionHandler *handler);

		//! Destructor
		~PhysicsBody();

	protected:
		//! Called by world when this body is substantiated
		//void Initialize(PhysicsWorld *world);
	public:
		//! Insert the set body properties into the Box2D objects
		void CommitProperties();

		b2BodyDef *GetBodyDef() const;

		void _setB2Body(b2Body *b2body);
		b2Body *GetB2Body() const;

		void SetWorld(PhysicsWorld* world);
		//! Sets the type ID for this object.
		void SetType(int type);
		//! Does what you think it does.
		void SetMass(float mass);
		//! Sets the allowSleep property of the BodyDef
		void SetCanSleep(bool canSleep);
		//! Sets teh isSleeping property of the BodyDef
		void SetInitialSleepState(bool isSleeping);
		
		//virtual void RecalculateInertia();
		//! Sets the radius used for torque equations.
		/*
		 * There are no torque equations ATM, but may be in the future...
		 */
		//virtual void SetRadius(float radius);

		//! Gets the type of this object
		int GetType();
		//! Gets the mass of this object
		float GetMass();
		//! Gets one over mass
		virtual float GetInverseMass();
		//! Gets the radius of this object
		virtual float GetRadius();

		//! Adds a force to the body
		virtual void ApplyForce(const Vector2 &force);
		//! Applies force based on the current orientation.
		/*!
		 * <p>
		 * This allows a force to be applied relative to the ships facing over
		 * the next step - such as a thrust force.
		 * </p>
		 */
		virtual void ApplyForceRelative(const Vector2 &force);
		//! Applies relative scalar force
		void ApplyForceRelative(float force);

		//! Applies an impulse
		virtual void ApplyImpulse(const Vector2 &force);
		//! Applies an impulse based on the current orientation.
		/*!
		 * <p>
		 * This allows an impulse to be applied relative to the ships facing over
		 * the next step - such as a thrust force.
		 * </p>
		 */
		void ApplyImpulseRelative(const Vector2 &force);
		//! Applies (scalar) force based on the current orientation and rotational velocity.
		void ApplyImpulseRelative(float force);

		//! Applies torque
		virtual void ApplyTorque(float torque);

		//! Sets the constant used to apply damping to the body's movement.
		//virtual void SetFriction(float damping);
		//! Sets the constant used to apply bounce to the body's collisions.
		//virtual void SetRestitution(float bounce);
		//! We don't care about that talk.
		virtual void SetAngularVelocityRad(float velocity);
		//! We don't care about all this 'torque' business.
		virtual void SetAngularVelocityDeg(float velocity);

		//! \brief Huh? What? Oh, 'torque'. I see how it is-
		/*!
		 * No, no, I'm sure you think it's very important.
		 */
		void SetAngularVelocity(float velocity);

		//! \name Collision Properties
		//@{
		//virtual void SetColBitmask(FusionBitmask *bitmask);
		//virtual void SetColAABB(float width, float height);
		//virtual SetColAABB(const CL_Rectf &bbox);
		//virtual void SetColDist(float dist);
		//@}

		b2Body* GetInternalBody() const;

		FixturePtrArray m_Fixtures;

		//! Wrapper that gets the b2FixtureDef* from a FixtureDefinition
		/*!
		* \see FixtureDefinition
		*/
		FixturePtr CreateFixture(const FixtureDefinition definition);
		//! Creates (and adds) a fixture from the given definition
		FixturePtr CreateFixture(const b2FixtureDef *def);
		//! Removes the given fixture
		void RemoveFixture(FixturePtr fixture);

		//void AttachShape(ShapePtr shape);
		//void DetachShape(ShapePtr shape);
		//void ClearShapes();

		//const ShapeList &GetShapes();

		//ShapePtr GetShape(b2Shape *internalShape);
		//ShapePtr GetShape(b2Shape *internalShape) const;
		//ShapePtr GetShape(const std::string &name);
		//ShapePtr GetShape(const std::string &name) const;

		void AddJoint(b2Joint* joint);
		void RemoveJoint(b2Joint* joint);
		void ClearJoints();

		void Clear();

		//void SetAllShapesElasticity(float e);
		//void SetAllShapesFriction(float u);

		//virtual void CacheBB();
		//! \name Collision property retreival
		//@{
		//virtual FusionBitmask *GetColBitmask() const;
		virtual CL_Rectf GetAABB() const;

		//! Returns true if the given point is solid.
		/*!
		 * <p>
		 * Quick access to the bitmask function FusionBitmask#GetBit()
		 * </p>
		 * If 'auto_offset' is true, the function will automatically offset the given
		 * 'point' relative to this body's current position, and scale it to match
		 * the the bitmask's PPB setting. <br>
		 * This is oftern required, because GetBit isn't designed to act as a collision
		 * detection function.
		 *
		 * \param point
		 * The point on the bitmask to check, starting at (0,0).
		 * \param auto_offset
		 * Automatically offset the 'point' param.
		 */
		//virtual bool GetColPoint(const CL_Point &point, bool auto_offset = true) const;
		//@}

		/*!
		 * \name Collision mode properties
		 *
		 * I think these are self explanatory.
		 */
		//@{
		//virtual void SetUsePixelCollisions(bool usePixel);
		//virtual void SetUseAABBCollisions(bool useAABB);
		//virtual void SetUseDistCollisions(bool useDist);
		//@}

		/*!
		 * \name Collision mode retrieval
		 *
		 * I think these are self explanatory.
		 */
		//@{
		//virtual bool GetUsePixelCollisions() const;
		//virtual bool GetUseAABBCollisions() const;
		//virtual bool GetUseDistCollisions() const;
		//@}

		//! Sets the user data
		/*!
		 * Usually a pointer to the game object this physical entity is associated with.
		 */
		void SetUserData(void *userdata);
		//! Retrives user data
		void *GetUserData() const;

		//! Sets a collision response callback
		//void SetCollisionCallback(const CollisionCallback &callback);

		//! Sets a collision handler
		void SetCollisionHandler(ICollisionHandler *handler);

		//! Returns true if the given body can experiance a collision with this one.
		bool CanCollideWith(PhysicsBodyPtr other);
		//! Calls the collision response (if this body has one.)
		void ContactBegin(const Contact &contact);
		//! Called while a contact persists
		void ContactPersist(const Contact &contact);
		//! Called when a collision ends
		void ContactEnd(const Contact &contact);

		//! [depreciated] Use CollisionWith()
		//void CollisionResponse(PhysicsBody *other, const std::vector<Contact> &collision_point);

		//! Returns true if this is a static body (infinate mass & inertia)
		bool IsStatic() const;

		//! Returns the current collision config.
		//int GetCollisionFlags();
		//! Returns the current collision config.
		//int GetCollisionFlags() const;
		//! Returns true if the given flag is set.
		//bool CheckCollisionFlag(int flag);
		//! Returns true if the given flag is set.
		//bool CheckCollisionFlag(int flag) const;
		//! Allows the collision flags to be set manually
		/*!
		 * This isn't usually necessary, as collision flags get set by relavant
		 * methods (e.g. If SetMass(g_PhysStaticMass) is called, the C_STATIC flag will be set.)
		 */
		//void _setCollisionFlags(int flags);

		//! \name State retreival.
		//@{
		//! Returns the current position (as used by the simulation)
		virtual const Vector2 &GetSimulationPosition() const;
		//! Returns the current position transformed to screen (1 unit = 1pixel) co-ords
		virtual const Vector2 &GetPosition() const;
		//! Integer point used as that makes this eaisier to pass to FusionBitmask.
		CL_Point GetPositionPoint() const;

		//! Gets the net constant (i.e. not 'relative') force applied to the body.
		//const Vector2 &GetForce();
		//! Gets the net relative force applied to the body.
		//virtual const Vector2& GetRelativeForce() const;

		//const Vector2 &GetAcceleration() const;
		const Vector2 &GetVelocity();

		//! Guess
		//virtual float GetFriction() const;
		//! Yep
		//virtual float GetRestitution() const;

		virtual float GetAngleRad() const;
		virtual float GetAngleDeg() const;

		virtual float GetAngularVelocityRad() const;
		virtual float GetAngularVelocityDeg() const;

		float GetAngle() const;
		float GetAngularVelocity() const;

		//! ISceneNodeController implementation
		//virtual float GetFacing() const;
		//@}

		//! Returns true if this object is active.
		/*!
		 * Objects which haven't had calls to any property modifying methods in a while
		 * will become in-active, and will be ignored during FusionPhysicsWorld#RunSimulation()
		 */
		bool IsActive() const;

		//! Sets m_Active to true.
		/*!
		 * m_Active will be set to true, and m_DeactivationCounter will be set to
		 * m_DeactivationPeriod. 
		 * <br>
		 * Allows FusionPhysicsWorld to activate bodies.
		 */
		void _activate();

		//! Sets m_Active to false.
		/*!
		 * Allows FusionPhysicsWorld to deactivate bodies.
		 */
		void _deactivate();

		//! Sets m_Active to false when m_DeactivationCounter reaches zero.
		/*!
		 * Allows FusionPhysicsWorld to deactivate bodies after a period.
		 *
		 * \param[in] split
		 * The amount of time since the last step (i.e. the amount to decrease the
		 * deactivation counter by.)
		 */
		//void _deactivateAfterCountdown(unsigned int split);

		//! Sets the value of m_DeactivationCounter.
		/*!
		 * Generally, _deactivateAfterCountdown() should be used rather than this.
		 */
		//void _setDeactivationCount(unsigned int count);

		//! Returns the value of m_DeactivationCounter.
		/*!
		 * Returns the ammount of time left before this body deactivates.
		 */
		unsigned int GetDeactivationCount() const;

		//! Sets m_DeactivationPeriod.
		/*!
		 * Sets the deactivation period (the period used to set the deactivation time.)
		 *
		 * \param[in] period
		 * The value to set to.
		 */
		//void SetDeactivationPeriod(unsigned int period);

		//! Gets m_DeactivationPeriod.
		/*!
		 * Returns the period used to set the deactivation counter.
		 */
		//unsigned int GetDeactivationPeriod() const;

		//! Sets the position without transforming from game co-ords.
		void _setSimulationPosition(const Vector2 &position);

		//! Sets the position.
		/*!
		* Co-ords passed are assumed to be in game units.
		*/
		void _setPosition(const Vector2 &position);

		//! Sets the force.
		//virtual void _setForce(const Vector2 &force);
		//! Sets the force.
		/*!
		  * \param force
		 * The force to set.
		 *
		 * \param direction
		 * [not implemented] The direction relative to which the force should be set.
		 */
		//virtual void _setRelativeForce(const Vector2 &force, float direction =0);

		//! Sets the acceleration.
		//virtual void _setAcceleration(const Vector2 &acceleration);
		//! Sets the velocity.
		virtual void _setVelocity(const Vector2 &velocity);

		//! Sets the rotation.
		virtual void _setAngleRad(float rotation);
		//! Sets the rotation (in degrees).
		virtual void _setAngleDeg(float rotation);

		virtual void _setAngle(float rotation);

		//! [removed] Adds the given body to the collision list
		void _notifyCollisionWith(PhysicsBody *other) {};
		//! [removed] Checks for the given body on the collision list
		bool IsCollidingWith(PhysicsBody *other) const { return false; };
		//! [removed] Clears the collision list
		void ClearCollisions() {};

		////! Used internally by CollisionGrid
		////@{
		////! Stores the Collision Grid Pos; the position on the grid.
		//void _setCGPos(int ind);
		////! Retreives the Collision Grid Pos - required for sorting etc.
		//int _getCGPos() const;
		////! [depreciated] Stores the Collision Cell Index; the position within the cell.
		//void _setCCIndex(int ind);
		////! Retreives the Collision Cell Index
		//int _getCCIndex() const;

		////! Sets m_GotCGUpdate to true.
		///*!
		// * Allows the collision grid to tell this body that it's update request has been noted.
		// */
		//void _notifyCGwillUpdate();
		////! Returns m_GotCGUpdate.
		///*!
		// * Allows the collision grid to see if this body has already requested an update.
		// */
		//bool _CGwillUpdate() const;
		////! Sets m_GotCGUpdate to false.
		///*!
		// * Allows the collision grid to tell this body that it's previously requested 
		// * update has been completed.
		// */
		//void _notifyCGUpdated();
		////@}

	protected:
		b2Body *m_BxBody;
		b2BodyDef *m_BxBodyDef;

		ShapeList m_Shapes;
		////! \name Used internally by CollisionGrid
		////@{
		////! Collision Grid Index
		//int m_CGPos;
		////! Collision Grid Index
		//int m_CCIndex;

		////! True if CollisionGrid#_updateThis(thisobject) has been called.
		//bool m_GotCGUpdate;
		////@}

		//! Containing world
		PhysicsWorld *m_World;

		//! \see FusionPhysicsCallback.h
		//CollisionCallback m_CollisionResponse;
		//! Collsion handler
		ICollisionHandler *m_CollisionHandler;

		//! Data which may be useful for collision responses, etc.
		void *m_UserData;
		//! Collision flags, such as C_STATIC
		//int m_CollisionFlags;

		//! The ID for the current object's type
		/*!
		 * This is used to identify objects in collision responses
		 */
		int m_Type;

		//! True when the body is active.
		bool m_Active;

		//! The time left before this body deactivates, if nothing happens to it.
		int m_DeactivationCounter;

		//! The period used to set m_DeactivationCounter.
		unsigned int m_DeactivationPeriod;

		//! bitmask
		//FusionBitmask *m_Bitmask;
		//! aabb
		CL_Rectf m_AABB;
		//! dist
		float m_Radius;

		//! Lists the bodies which have signaled that they are colliding with this one
		//BodyList m_CollidingBodies;

		//! Bitmask collisions
		bool m_UsesPixel;
		//! Bounding Box collisions
		bool m_UsesAABB;
		//! Distance (circle) based collisions
		bool m_UsesDist;

		Vector2 m_InitialVelocity;
		bool m_Bullet;

		//@{
		//! "State" stuff.
		float m_Mass;
		float m_InverseMass;
		//! Linear damping, a.k.a. coefficient of friction
		float m_LinearDamping;
		//! Bounce, a.k.a. coefficient of restitution
		float m_Bounce;

		Vector2 m_AppliedForce;
		Vector2 m_AppliedRelativeForce;

		Vector2 m_Acceleration;
		Vector2 m_CachedVelocity;
		//std::vector<Vector2> m_DisplacementPath;
		Vector2 m_Displacement;

		mutable Vector2 m_CachedPosition;
		mutable Vector2 m_CachedGamePosition;

		float m_RotationDeg;
		float m_Rotation;

		float m_RotationalVelocityDeg;
		//! Current velocity of rotation.
		/*!
		 * \remarks
		 * (ShipResourceBundle has RotationVelocity [no 'al'], that being the <i>maximum</i>
		 * velocity of rotation.)
		 */
		float m_RotationalVelocity;
		//@}

	};

	//static int bodyCollFunc(b2Shape *a, b2Shape *b, b2ContactPoint *contacts, int numContacts, cpFloat normal_coef, void *data)
	//{
	//	if (a->data == NULL || b->data == NULL)
	//		return 1;

	//	PhysicsBody* aBody = (PhysicsBody*)a->data;
	//	PhysicsBody* bBody = (PhysicsBody*)b->data;

	//	std::vector<Contact> contactList;
	//	for (int i = 0; i < numContacts; i++)
	//	{
	//		contactList.push_back( Contact(contacts[i]) );
	//	}

	//	aBody->CollisionWith(bBody, contactList);
	//	bBody->CollisionWith(aBody, contactList);

	//	if (aBody->CanCollideWith(bBody) && bBody->CanCollideWith(aBody))
	//		return 1;

	//	return 0;
	//}

}

#endif
