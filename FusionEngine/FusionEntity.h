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

#ifndef Header_FusionEngine_Entity
#define Header_FusionEngine_Entity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionScriptingEngine.h"
//#include "FusionCommand.h"

/// Inherited
#include "FusionPhysicsCallback.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Input structure for entities
	 *
	 * \todo Perhaps this should be implemented entirely at a script level?
	 *
	 * \todo Rename this as Command
	 */
	class EntityInput
	{
	public:
		//! Adds a variable to the input structure
		void AddKey(const std::string& name) {}//;
		//! Check a var
		bool GetInput(const std::string& name) {}//;

		char* Serialize() {}//;
		void Deserialize(char* data) {}//;
	};

	/*!
	 * \brief
	 * In game object base class
	 */
	class Enitity : public SceneNode, public ICollisionHandler
	{
	public:
		//! A list of ObjectIDs
		typedef std::vector<ObjectID> ObjectList;

		//! List of properties
		typedef std::map<std::string, PropertyPointer> PropertyList;

	public:
		//! Constructor
		Enitity();
		//! Constructor. W/O inputs.
		Enitity(const std::string& name);
		//! Destructor
		~Enitity();

	public:
		//! Sets Body velocity
		void SetVelocity(const Vector2 &velocity);
		//! Sets Body pos
		void SetPosition(const Vector2 &position);
		//! Sets Body rv
		void SetRotationalVelocity(float velocity);
		//! Sets Body rotation
		void SetRotation(float rotation);

		//! Ship state
		void SetProperties(const PropertyList& props);
		//! Input state
		void SetInput(const PropertyList& input);

		//! Guess
		const PropertyList &GetProperties() const;
		//! Self explainatory
		const PropertyList &GetInput() const;

		//EntityState GetState() const;
		//void SetState(EntityState state);
		//! Validates the given state (this will become the sync point)
		/*!
		 * Only after ValidateState has been called should irreversable changes be made,
		 * e.g. terrain destruction
		 */
		//void ValidateState(StateID state);

		//! Adds a simulation state
		/*!
		 * Sets the state to the stored state (history) corresponding to the given ID, and runs
		 * the Simulate script for this entity.
		 */
		virtual void Simulate();

		PropertyPointer GetProperty(const std::string& name);

		virtual std::string SerializeState() const;
		virtual void DeserializeState(const std::string& state);

		//! Returns a human-readable string
		virtual std::string ToString() const;

		////! Scene node
		//void SetSceneNode(FusionNode *input);
		////! Self explainatory
		//const FusionNode *GetSceneNode() const;
		//! Physics body
		void SetPhysicalBody(FusionPhysicsBody *input);
		//! Self explainatory
		const FusionPhysicsBody *GetPhysicalBody() const;

		//! Returns true if the input have changed recently.
		/*!
		 * Returns true if the input data stored here has been changed since the last
		 * call to _inputSynced().
		 */
		bool InputHasChanged() const;
		//! Returns true if the state have changed recently.
		/*!
		 * Returns true if the state stored here has been changed since the last
		 * call to _stateSynced().
		 */
		bool StateHasChanged() const;

		//! Makes InputHasChanged return false.
		/*!
		 * This should be called after input data has been read from this 'ship'
		 * (that is to say, this 'player data storeage location') so that no
		 * unnecessary packets will be sent.
		 *
		 * \sa ClientEnvironment#send() | ServerEnvironment#send()
		 */
		bool _inputSynced() const;
		//! Makes StateHasChanged return false.
		/*!
		 * This should be called after state data has been read from this 'ship'
		 * (that is to say, this 'player data storeage location') so that no
		 * unnecessary packets will be sent.
		 */
		bool _stateSynced() const;

		//! Reverts all state data
		void RevertToInitialState();

		//! Implementation of ICollisionHandler#CanCollideWith()
		bool CanCollideWith(const FusionPhysicsBody *other);

		//! Implementation of ICollisionHandler#CollisionWith()
		void CollisionWith(const FusionPhysicsBody *other, const Vector2 &collision_point);


	protected:
		// Is this needed? - 2006/09/08: no
		//ClientEnvironment *m_Environment;

		//! Body
		FusionPhysicsBody *m_Body;

		PropertyList m_Properties;

		EntityList m_Children;

		//! Weapons available to this ship
		/*!
		 * This is kept localy on client and server (notice it isn't
		 * in ShipState) and occasionally sync-ed with MTID_HELDWEAPONS.
		 *
		 * \remarks
		 * For non-powerup gamemodes, this will simply be ResourceManager->LoadedWeapons.
		 */
		WeaponList m_HeldWeapons;

		//! Projectiles owned by this ship
		/*!
		 * Useful for scripting and debug
		 */
		ObjectList m_Projectiles;

		//! Will be set to true if theis entity has been marked for detination (to allow chain detonations)
		bool m_Detonated;

		//! Input state
		ShipInput m_Input;
		//! Current spacial attributes
		ShipState m_CurrentState;
		//! Default (initial) spacial attributes
		ShipState m_InitialState;

		//! True if the input state has been modified
		bool m_InputChanged;
		//! True if the ShipState has been modified
		bool m_StateChanged;
	};

}

#endif
