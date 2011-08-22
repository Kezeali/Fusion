/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionEntityManager
#define H_FusionEntityManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Bitstream.h>
#include <RakNetTypes.h>

#include "FusionEntity.h"
#include "FusionIDStack.h"
#include "FusionInputHandler.h"
#include "FusionPacketHandler.h"
#include "FusionPlayerInput.h"
#include "FusionStreamingManager.h"
#include "FusionViewport.h"

#include <tbb/spin_rw_mutex.h>

//#include <boost/bimap.hpp>

namespace FusionEngine
{

	class ISystemWorld;

	//! Updates input states for each player (local and remote)
	class ConsolidatedInput
	{
	public:
		typedef std::tr1::unordered_map<PlayerID, PlayerInputPtr> PlayerInputsMap;

	public:
		ConsolidatedInput(InputManager *input_manager);
		~ConsolidatedInput();

		void SetState(PlayerID player, const std::string input, bool active, float position);
		PlayerInputPtr GetInputsForPlayer(PlayerID player);

		const PlayerInputsMap &GetPlayerInputs() const;

		unsigned short ChangedCount() const;
		void ChangesRecorded();

		PlayerID LocalToNetPlayer(unsigned int local);

	protected:
		InputManager *m_LocalManager;

		void onInputChanged(const InputEvent &event);

		unsigned short m_ChangedCount;

		PlayerInputsMap m_PlayerInputs;

		boost::signals2::connection m_InputChangedConnection;

	};

	static const unsigned int s_EntitiesPerPacket = 8;
	static const unsigned int s_BodiesPerPacket = 12;

	static const RakNet::BitSize_t s_MaxEntityData = 256;

	class EntitySynchroniser : public PacketHandler
	{
	public:
		EntitySynchroniser(InputManager *input_manager);
		~EntitySynchroniser();

		const EntityArray &GetReceivedEntities() const;

		void BeginPacket();
		void EndPacket();

		//! Sends data
		void Send();

		void OnEntityAdded(EntityPtr &entity);

		// Returns true if the entity should be updated
		bool ReceiveSync(EntityPtr &entity, const EntityDeserialiser &entity_deserialiser);
		// Returns true if the entity state was written to the packet
		bool AddToPacket(EntityPtr &entity);

		void HandlePacket(RakNet::Packet *packet);

	protected:
		ConsolidatedInput *m_PlayerInputs;
		InputManager *m_InputManager;

		RakNetwork *m_Network;

		//typedef std::tr1::unordered_map<ObjectID, EntityArray> EntityPreparedInstancesMap;
		//EntityPreparedInstancesMap m_EntityPreparedInstances;
		boost::signals2::connection m_EntityInstancedCnx;

		EntityArray m_ReceivedEntities;

		typedef std::tr1::unordered_map<ObjectID, SerialisedData> ObjectStatesMap;
		ObjectStatesMap m_ReceivedStates;

		ObjectStatesMap m_SentStates;

		struct EntityPacketData
		{
			ObjectID ID;
			SerialisedData State;
		};

		typedef std::map<unsigned int, EntityPtr, std::greater<unsigned int>> EntityPriorityMap;
		EntityPriorityMap m_EntityPriorityQueue;

		size_t m_EntityDataUsed;

		struct SystemPriority
		{
			RakNet::RakNetGUID System;
			float Distance; // Average Entity distance
			unsigned int SkippedSteps;
		};

		typedef std::vector<SystemPriority> SystemArray;
		SystemArray m_PacketDestinations;

		bool m_ImportantMove;
		//std::string m_PacketData;
		RakNet::BitStream m_PacketData;
	};

	enum DomainState
	{
		DS_INACTIVE = 0x0,
		DS_STREAMING = 0x1,
		DS_SYNCH = 0x2,
		DS_ENTITYUPDATE = 0x4,
		DS_ALL = DS_STREAMING | DS_SYNCH | DS_ENTITYUPDATE
	};

	/*!
	 * \brief
	 * Updates / draws entity objects.
	 *
	 * EntityManager uses a tag system rather than a more traditional
	 * scene-graph (tree based) system for managing hidden / paused
	 * entities because it is easier to understand & use for the end
	 * user. Furthermore, an entity tree can easily be implemented
	 * at a high level by entities themselves.
	 *
	 * \see
	 * Entity
	 */
	class EntityManager : public IEntityRepo
	{
	protected:
		typedef std::set<std::string> BlockedTagSet;
		typedef std::map<std::string, bool> BlockingChangeMap;

		typedef std::set<EntityPtr> EntitySet;

	public:
		//! Constructor
		EntityManager(InputManager *input_manager, EntitySynchroniser *entity_synchroniser, StreamingManager *streaming);
		//! Destructor
		virtual ~EntityManager();

	public:
		typedef std::tr1::weak_ptr<Entity> EntityWPtr;
		//! A list of nodes
		typedef std::tr1::unordered_map<std::string, EntityPtr> NameEntityMap;
		typedef std::map<ObjectID, EntityPtr> IDEntityMap;

		//typedef boost::bimap<std::string, unsigned int> TagFlagMap;
		//typedef TagFlagMap::value_type TagDef;

		//! Makes all the entity IDs sequential (so there are no gaps)
		void CompressIDs();

		//! Set to true to give entities named "default" a unique name when they are passed to AddEntity
		void EnableDefaultNameGeneration(bool enabled);
		//! Returns the value set by EnableDefaultNameGeneration
		bool IsGeneratingDefaultNames() const;

		//! Adds a created entity
		void AddEntity(EntityPtr &entity);

		//! Removes the given entity
		void RemoveEntity(const EntityPtr &entity);
		//! Removes the entity with the givcen name
		void RemoveEntityNamed(const std::string &name);
		//! Removes the entity with the given ID
		void RemoveEntityById(ObjectID id);

		//! Removes the Entity with the given ID and adds the new Entity
		void ReplaceEntity(ObjectID id, EntityPtr &entity);

		//! Finds the Entity which currently has the given name and changes it to the given new name
		void RenameEntity(const std::string &current_name, const std::string &new_name);
		//! Changes the given Entity's search name
		void RenameEntity(EntityPtr &entity, const std::string &new_name);

		//! Returns the Entity with the given name
		/*!
		* \param[in] name
		* Name of the desired Entity
		*
		* \param[in] throwIfNotFound
		* Throw rather than returning NULL
		*/
		EntityPtr GetEntity(const std::string &name, bool throwIfNotFound = false) const;
		//! Returns the Entity with the given ID
		/*!
		* Obviously can't return pseudo-Entities, since they have no ID
		*
		* \param[in] id
		* ID of the desired Entity
		*
		* \param[in] throwIfNotFound
		* Throw rather than returning NULL
		*/
		EntityPtr GetEntity(ObjectID id, bool throwIfNotFound) const;
		//! Implementation of IEntityRepo
		EntityPtr GetEntity(ObjectID id) const { return GetEntity(id, false); }

		//! Returns Entities
		/*!
		* Doesn't return Pseudo-Entities.
		*/
		const IDEntityMap &GetEntities() const;

		//! Returns Pseudo-Entities
		const EntitySet &GetPseudoEntities() const;

		//! Returns the EntityArray for the given domain
		//EntityArray &GetDomain(EntityDomain domain_index);

		bool AddTag(const std::string &entity_name, const std::string &tag);
		bool AddTag(EntityPtr entity, const std::string &tag);
		void RemoveTag(const std::string &entity_name, const std::string &tag);
		void RemoveTag(EntityPtr entity, const std::string &tag);

		bool CheckTag(const std::string &entity_name, const std::string &tag);
		//StringVector GetTags() const;

		inline bool IsBlocked(EntityPtr entity, const BlockingChangeMap &tags);

		void PauseEntitiesWithTag(const std::string &tag);
		void ResumeEntitiesWithTag(const std::string &tag);
		void HideEntitiesWithTag(const std::string &tag);
		void ShowEntitiesWithTag(const std::string &tag);
		void RemoveEntitiesWithTag(const std::string &tag);

		/*!
		* \brief
		* Removes all entities from the manager.
		*/
		void Clear();

		/*!
		* \brief
		* Removes synchronised (non-pseudo) entities from the manager
		*/
		void ClearSyncedEntities();
		
		//! Removes all entities in the given domain
		void ClearDomain(EntityDomain domain_index);

		void ProcessActivationQueues();

		void ProcessActiveEntities(float split);

		void UpdateActiveRegions();

		//! Draws entities
		void Draw(Renderer *renderer, const ViewportPtr &viewport, size_t layer);
		//! Returns active entities
		EntityArray& GetActiveEntities();

		//! Sets the given domain to active/inactive
		void SetDomainState(EntityDomain domain_index, char active_modes);
		//! Returns true if the given domain is active
		bool CheckState(EntityDomain domain_index, DomainState mode) const;
		//! Returns the state of the given domain
		char GetDomainState(EntityDomain domain_index) const;

		void SetModule(ModulePtr module);

		void OnModuleRebuild(BuildModuleEvent& ev);

		//! Registers EntityManager script methods
		static void Register(asIScriptEngine *engine);

		void OnComponentAdded(EntityPtr& entity, std::shared_ptr<IComponent>& component);

		void OnActivationEvent(const ActivationEvent& ev);

		// Will notify entities that the given player was added next time Update is called 
		void OnPlayerAdded(unsigned int local_index, PlayerID net_id);

		tbb::concurrent_queue<IComponentProperty*> m_PropChangedQueue;

	protected:
		//! Updates the entities that have been added to the active-entities list
		void updateEntities(EntityArray &entities, float split);

		//! \param real_only Only remove non-pseudo entities (used before loading save-games, for example)
		void clearEntities(bool real_only);

		void queueEntityToActivate(const EntityPtr& entity);
		//! returns true if all components are ready to activate
		bool prepareEntity(const EntityPtr &entity);
		//! returns true if all components were ready and thus activated
		bool attemptToActivateEntity(const EntityPtr &entity);
		bool attemptToActivateComponent(ISystemWorld* world, const std::shared_ptr<IComponent>& component);
		void activateEntity(const EntityPtr &entity);

		void deactivateEntity(const EntityPtr& entity);

		//! Generates a unique name for the given entity
		std::string generateName(const EntityPtr &entity);

		void updateTags(EntityPtr &tag) const;

		bool m_GenerateDefaultNames;

		InputManager *m_InputManager;

		EntitySynchroniser *m_EntitySynchroniser;
		StreamingManager *m_StreamingManager;

	public:
		EntityFactory *m_EntityFactory;
	protected:
		ObjectIDStack m_UnusedIds;

		mutable tbb::spin_rw_mutex m_EntityListsMutex;

		// Used to quickly find entities by name (all entities, pseudo/non-pseudo are listed here)
		NameEntityMap m_EntitiesByName;

		// All non-pseudo-entities
		IDEntityMap m_Entities;
		// All pseudo-entities
		EntitySet m_PseudoEntities;

		tbb::concurrent_queue<std::pair<EntityPtr, std::shared_ptr<IComponent>>> m_ComponentsToAdd;
		tbb::concurrent_queue<EntityPtr> m_NewEntitiesToActivate;

		std::vector<std::pair<EntityPtr, std::shared_ptr<IComponent>>> m_ComponentsToActivate;
		std::vector<EntityPtr> m_EntitiesToActivate;
		std::vector<EntityPtr> m_EntitiesToDeactivate;
		EntityArray m_ActiveEntities;

		// Entities to be updated - 8 domains
		//EntityArray m_EntitiesToUpdate[s_EntityDomainCount];
		// Active status of each domain
		char m_DomainState[s_EntityDomainCount];

		bool m_EntitiesLocked;
		bool m_ClearWhenAble;

		std::list<std::pair<unsigned int, PlayerID>> m_PlayerAddedEvents;

		TagFlagDictionaryPtr m_TagFlagDictionary;

		unsigned int m_ToDeleteFlags;
		unsigned int m_UpdateBlockedFlags;
		unsigned int m_DrawBlockedFlags;

		BlockingChangeMap m_ChangedUpdateStateTags;
		BlockedTagSet m_UpdateBlockedTags;

		BlockingChangeMap m_ChangedDrawStateTags;
		BlockedTagSet m_DrawBlockedTags;

		boost::signals2::connection m_ModuleConnection;

	};

}

#endif
