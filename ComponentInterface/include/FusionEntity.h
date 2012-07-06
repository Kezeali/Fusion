/*
*  Copyright (c) 2007-2012 Fusion Project Team
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

#ifndef H_FusionEntity
#define H_FusionEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include "FusionTypes.h"
#include "FusionCommon.h"
#include "FusionEntityComponent.h"
//#include "FusionCommandQueue.h"
#include "FusionTransformComponent.h"
//#include "FusionPlayerInput.h"
#include "FusionResourcePointer.h"
//#include "FusionTagFlagDictionary.h"
#include "FusionVector2.h"
#include "FusionVectorTypes.h"

#include <tbb/spin_rw_mutex.h>
#include <tbb/mutex.h>

namespace FusionEngine
{
	//! Number of 'domains' - used to group entities
	static const size_t s_EntityDomainCount = 8;

	enum EntityDomains { SYSTEM_DOMAIN, GAME_DOMAIN, SYSTEM_LOCAL_DOMAIN, USER_DOMAIN };

	typedef std::vector<EntityPtr> EntityArray;

	typedef std::shared_ptr<PlayerInput> PlayerInputPtr;

	class StreamedResourceUser;

	class IArchetypeAgent;

	/*!
	 * \brief
	 * In game object base class
	 */
	class Entity : public std::enable_shared_from_this<Entity>, noncopyable
	{
	public:
		//! Constructor
		Entity(EntityRepo* manager, const ComponentPtr& transform_component);
		//! Destructor
		virtual ~Entity();

	public:
		//! Array of StreamedResourceUser objects
		typedef std::vector<StreamedResourceUser*> StreamedResourceArray;
		typedef std::unordered_set<std::string> TagSet;

		//! Sets the search-name of this Entity
		void SetName(const std::string &name);
		//! Gets the search-name of this Entity
		const std::string &GetName() const;

		//! Informs this entity that it has been given a default name
		void _notifyDefaultName(const std::string &name);
		//! Returns true if the name currently assigned to this Entity is default
		/*!
		* If an entity is added to the manager without a search name, it will get a default
		* search name assigned to it. Such names should not be saved to save-game files,
		* so this property allows the saved-game creator to make an informed decision.
		*/
		bool HasDefaultName() const;

		//! Sets the sync ID of this Entity.
		/*!
		* Set to zero to make this a pseudo-entity, i.e. prevent this entity from sync.ing
		*/
		void SetID(ObjectID id);
		//! Returns the sync. ID of this Entity
		ObjectID GetID() const;

		//! Sets the owner id of this entity
		void SetOwnerID(PlayerID owner);
		//! Returns the owner ID of this entity
		PlayerID GetOwnerID() const;

		//! Sets the current authority for this entity
		void SetAuthority(PlayerID authority);
		//! Gets the current authority for this entity
		PlayerID GetAuthority() const;

		//! Marks this entity as terrain (in first, out last when it's cell is activated)
		void SetTerrain(bool is_terrain);
		//! Returns true if this is a terrain entity
		bool IsTerrain() const;

		unsigned int m_SkippedPackets;
		void PacketSkipped() { ++m_SkippedPackets; }
		void AddedToPacket() { m_SkippedPackets = 0; }
		unsigned int GetSkippedPacketsCount() const { return m_SkippedPackets; }

		//! Returns true if this Entity is a synchronised entity
		/*!
		* Synchronised entities have a unique entity-ID, so this returns
		* true when GetID() > 0
		*/
		bool IsSyncedEntity() const;
		//! Returns true if this Entity is a pseudo-entity - an entity which doesn't sync.
		bool IsPseudoEntity() const;

		//! Default implementation
		virtual void EnumReferences(asIScriptEngine *engine)
		{}
		//! Default implementation
		virtual void ReleaseAllReferences(asIScriptEngine *engine)
		{}

		//! Sets the archetype id of this entity
		void SetArchetype(const std::string& id);
		//! Returns the archetype of this entity
		const std::string& GetArchetype() const;

		ComponentPtr GetTransform() const;

		//! Gets position
		const Vector2 &GetPosition();
		//! Gets angle (rotation) value
		float GetAngle() const;

		//! Gets position
		void SetPosition(const Vector2 &position);
		//! Gets angle (rotation) value
		void SetAngle(float angle);

		EntityRepo* GetManager() const { return m_Manager; }


		tbb::mutex m_InRefsMutex;
		std::set<std::weak_ptr<Entity>, std::owner_less<std::weak_ptr<Entity>>> m_ReferencingEntities;
		tbb::spin_rw_mutex m_OutRefsMutex;
		// TODO: support multiple refs to the same entity by using a map with special ref-IDs
		std::map<EntityPtr, size_t> m_ReferencedEntities;
		// Stuff that needs to be activated before this can activate:
		std::vector<std::pair<ObjectID, size_t>> m_UnloadedReferencedEntities;
	
	private:
		//! Notifies this entity that the given entity is referencing it
		void AddReference(EntityPtr entity);
		//! Notifies this entity that the given entity is no longer referencing it
		void RemoveReference(EntityPtr entity);

		tbb::atomic<unsigned int> m_LockingReferences;
		void IncrRefCount() { ++m_LockingReferences; m_GCFlag = false; }
		void DecrRefCount() { --m_LockingReferences; }
		
	public:
		//! Adds a reference from this entity to the given entity
		void HoldReference(EntityPtr toHold);
		//! Removes a reference from this entity to the given entity
		void DropReference(EntityPtr heldEntity);
		
		bool IsReferenced() const { return m_LockingReferences != 0; }
		unsigned int GetNumUsers() const { return m_LockingReferences; }

		tbb::atomic<bool> m_GCFlag;
		void SetGCFlag(bool value) { m_GCFlag = value; }
		bool GetGCFlag() const { return m_GCFlag; }

		//! Sets the archetype agent
		void SetArchetypeAgent(const std::shared_ptr<IArchetypeAgent>& agent) { m_ArchetypeAgent = agent; }
		//! Returns this entity's archetype agent
		std::shared_ptr<IArchetypeAgent> GetArchetypeAgent() const { return m_ArchetypeAgent; } 
		//! Returns true if this entity is archetypal (based on an archetype)
		bool IsArchetypal() const { return (bool)m_ArchetypeAgent; }

		//! Adds the given component
		void AddComponent(const ComponentPtr& component, std::string identifier = std::string());
		//! Removes the given component
		void RemoveComponent(const ComponentPtr& component);
		bool HasComponent(const ComponentPtr& component);

		void OnComponentActivated(const ComponentPtr& component);
		
		typedef std::map<std::string, std::map<std::string, ComponentPtr>> ComInterfaceMap;

		//template <class Interface>
		//void InvokeOnComponent(std::function<void (boost::intrusive_ptr<Interface>)> function)
		//{
		//	auto _where = m_ComponentInterfaces.find(Interface::GetTypeName());
		//	if (_where != m_ComponentInterfaces.end())
		//	{
		//		CallQueue<Interface> *actualQueue = dynamic_cast<CallQueue<Interface>*>(_where->second);
		//		if (actualQueue) actualQueue->Enqueue(function);
		//	}
		//}

		template <class Interface>
		ComponentIPtr<Interface> GetComponent(std::string identifier = std::string()) const
		{
			auto _where = m_ComponentInterfaces.find(Interface::GetTypeName());
			if (_where != m_ComponentInterfaces.end())
			{
				FSN_ASSERT(!_where->second.empty());
				if (identifier.empty())
				{
					return ComponentIPtr<Interface>(_where->second.begin()->second);
				}
				else
				{
					auto implEntry = _where->second.find(identifier);
					if (implEntry != _where->second.end())
						return ComponentIPtr<Interface>(implEntry->second);
				}
			}
			return ComponentIPtr<Interface>();
		}

		ComponentPtr GetComponent(const std::string& type, std::string identifier = std::string()) const;

		const std::vector<ComponentPtr>& GetComponents() const;

		const ComInterfaceMap& GetInterfaces() const;

		//! Create a new copy of this entity
		EntityPtr Clone(ComponentFactory* factory) const;

		void SynchroniseParallelEdits();

		//std::map<std::string, std::string> GetComponentNames() const;

		//! Sets the dictionary used by the EntityManager
		/*!
		 * EntityManager objects use this to pass the current
		 * dictionary to Entities when they are added to them.
		 */
		//void SetTagDictionary(TagFlagDictionaryPtr dictionary);

		//! Adds a Tag
		void AddTag(const std::string &tag);
		//! Removes a Tag
		void RemoveTag(const std::string &tag);
		//! Removes all tags
		void ClearTags();
		//! Checks whether this Entity is tagged with the given Tag
		bool CheckTag(const std::string &tag) const;

		//! Returns this entity's tags
		StringVector GetTags() const;

		void _notifyPausedTag(const std::string &tag);
		void _notifyResumedTag(const std::string &tag);
		void _notifyHiddenTag(const std::string &tag);
		void _notifyShownTag(const std::string &tag);

		const StringSet &GetPausedTags() const;
		const StringSet &GetHiddenTags() const;

		bool IsPausedByTag() const;
		bool IsHiddenByTag() const;

		//! Sets the tag-flags for this entity
		/*!
		 * This is intended for internal use.
		 * Use Entity#AddTag() unless you know you
		 * should be using this.
		 */
		void SetTagFlags(unsigned int flag);
		//! Activates the given tag-flag for this entity
		/*!
		 * This is intended for internal use.
		 * Use AddTag() unless you know you
		 * should be using this.
		 */
		void AddTagFlag(unsigned int flag);
		//! Deactivates the given tag-flag for this entity
		/*!
		 * This is intended for internal use.
		 * Use RemoveTag() unless you know you
		 * should be using this.
		 */
		void RemoveTagFlag(unsigned int flag);
		//! Returns the tag-flags for this entity
		/*!
		 * This is intended for internal use.
		 * Use CheckTag() unless you know you
		 * should be using this.
		 */
		unsigned int GetTagFlags() const;

		//! Sets the update domain for this entity
		void SetDomain(EntityDomain domain_index);
		//! Gets the update domain for this entity
		EntityDomain GetDomain() const;

		void SetLayer(size_t layer);
		size_t GetLayer() const;

		//! Returns true if this Entity has been streamed in
		bool IsStreamedIn() const;

		bool IsActive() const;

		void SetStreamingCellIndex(CellHandle index);
		CellHandle GetStreamingCellIndex() const;

		void SetPaused(bool is_paused);
		bool IsPaused() const;

		void SetHidden(bool is_hidden);
		bool IsHidden() const;

		void SetDepth(int depth);
		int GetDepth() const;

		//! Stops the Entity from being updated for a number of steps
		void SetWait(unsigned int steps);
		//! Returns true when the waiting period is over if called once per step
		bool Wait();

		//! Marks this Entity as one that will be deleted during the next update (effectively constant time)
		void MarkToRemove();
		//! Returns true if this Entity has been marked to delete.
		bool IsMarkedToRemove() const;

		//! Used to remove the entity from the active entity list in effectively constant time
		void MarkToDeactivate();
		//! Undoes MarkToDeactivate()
		void RemoveDeactivateMark();
		//! Returns true if the Entity is marked to deactivate
		bool IsMarkedToDeactivate() const;

		//! Called when a new local player is added
		virtual void OnPlayerAdded(unsigned int local_index, PlayerID net_id) {};

		//! Returns true if the Entity has been spawned
		bool IsSpawned() const;
		//! Makes IsSpawned() return true, without calling OnSpawn()
		void SetSpawned(bool spawned);

		//! Calls OnSpawn, sets the spawned flag to true
		void Spawn();

		//! Streams in resources
		void StreamIn();
		//! Streams out resources
		void StreamOut();

		void _setPlayerInput(const PlayerInputPtr &player_input);

		bool InputIsActive(const std::string &input);
		float GetInputPosition(const std::string &input);

		//! Registers the script Entity type
		static void Register(asIScriptEngine *engine);

	protected:
		std::string m_Type;
		std::string m_Name;
		ObjectID m_Id;
		bool m_HasDefaultName;

		//! The player who owns this entity, 0 for default ownership
		//!  (which falls to the arbitrator, if ownership is needed)
		//!  Most entities have no owner (i.e. this will be set to 0)
		//!  Where there is no owner, the authority (below) is used.
		PlayerID m_OwnerID;
		//! The player who currently has authority over this entity.
		//!  This is only used when OwnerID is zero - otherwise the
		//!  authority is always the owner.
		tbb::atomic<PlayerID> m_Authority;

		bool m_Terrain;

		tbb::spin_rw_mutex m_ComponentsMutex;

		ComponentIPtr<ITransform> m_Transform;

		std::vector<ComponentPtr> m_Components;
		ComInterfaceMap m_ComponentInterfaces;

		PropertySignalingSystem_t *m_PropChangedQueue;

		std::shared_ptr<IArchetypeAgent> m_ArchetypeAgent;

		EntityRepo* m_Manager;

		PlayerInputPtr m_PlayerInput;

		//TagFlagDictionaryPtr m_TagFlagDictionary;

		StringSet m_PausedTags;
		StringSet m_HiddenTags;

		TagSet m_Tags;
		// Markers (flags) for this entity
		// If any of the true bits correspond to true bits
		// in the entity manager, this entity isn't drawn
		// or updated
		unsigned int m_Flags;

		CellHandle m_CellIndex;

		bool m_Spawned;
		bool m_StreamedIn;
		bool m_Paused;
		bool m_Hidden;
		unsigned int m_WaitStepsRemaining;
		bool m_MarkedToRemove;
		tbb::atomic<bool> m_MarkedToDeactivate;

		int m_Depth;

		// EntityManager domain (1-8)
		EntityDomain m_Domain;
		// Renderer layer
		size_t m_Layer;

		StreamedResourceArray m_StreamedResources;

		inline void SetStreamedIn(bool is_streamed_in);
	};

}

#endif
