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

#ifndef Header_FusionEngine_EntityManager
#define Header_FusionEngine_EntityManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"
#include "FusionPacketHandler.h"
#include "FusionPlayerInput.h"
#include "FusionInputHandler.h"
#include "FusionRenderer.h"

#include "FusionNetwork.h"

#include <boost/bimap.hpp>

namespace FusionEngine
{

	//! Updates input states for each player (local and remote)
	class ConsolidatedInput
	{
	public:
		typedef std::tr1::unordered_map<ObjectID, PlayerInputPtr> PlayerInputsMap;

	public:
		ConsolidatedInput(InputManager *input_manager);
		~ConsolidatedInput();

		void SetState(ObjectID player, const std::string input, bool active, float position);
		PlayerInputPtr GetInputsForPlayer(ObjectID player);

		const PlayerInputsMap &GetPlayerInputs() const;

		unsigned short ChangedCount() const;
		void ChangesRecorded();

		ObjectID LocalToNetPlayer(unsigned int local);

	protected:
		InputManager *m_LocalManager;

		void onInputChanged(const InputEvent &event);

		unsigned short m_ChangedCount;

		PlayerInputsMap m_PlayerInputs;

		bsig2::connection m_InputChangedConnection;

	};

	static const BitSize_t s_MaxEntityPacketSize = 512;

	class EntitySynchroniser : public PacketHandler
	{
	public:
		struct InstanceDefinition
		{
			std::string Type;
			std::string Name;
			ObjectID ID;
			ObjectID Owner;
			SerialisedData State;
		};
		typedef std::vector<InstanceDefinition> InstanceDefinitionArray;

		EntitySynchroniser(InputManager *input_manager, Network *network);

		const InstanceDefinitionArray &GetReceivedEntities() const;

		void BeginPacket();
		void EndPacket();

		void Send(ObjectID player);

		void OnEntityAdded(EntityPtr &entity);

		// Returns true if the entity should be updated
		bool ReceiveSync(EntityPtr &entity, const EntityDeserialiser &entity_deserialiser);
		// Returns true if the entity state was written to the packet
		bool SendSync(EntityPtr &entity);

		void HandlePacket(IPacket *packet);

	protected:
		ConsolidatedInput *m_PlayerInputs;
		InputManager *m_InputManager;

		Network *m_Network;

		InstanceDefinitionArray m_ReceivedEntities;

		typedef std::tr1::unordered_map<ObjectID, SerialisedData> ReceivedStatesMap;
		ReceivedStatesMap m_ReceivedStates;

		bool m_ImportantMove;
		//std::string m_PacketData;
		RakNet::BitStream m_PacketData;
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
	class EntityManager
	{
	protected:
		typedef std::set<std::string> BlockedTagSet;
		typedef std::map<std::string, bool> BlockingChangeMap;

		typedef std::set<EntityPtr> EntitySet;

	public:
		//! Constructor
		EntityManager(Renderer *renderer, InputManager *input_manager, EntitySynchroniser *entity_synchroniser);
		//! Destructor
		virtual ~EntityManager();

	public:
		typedef std::tr1::weak_ptr<Entity> EntityWPtr;
		//! A list of nodes
		typedef std::tr1::unordered_map<std::string, EntityPtr> NameEntityMap;
		typedef std::map<ObjectID, EntityPtr> IDEntityMap;

		//typedef boost::bimap<std::string, unsigned int> TagFlagMap;
		//typedef TagFlagMap::value_type TagDef;

		//! Gives you access to the root node
		//virtual EntityPtr GetRootNode() const;

		//! Creates a new node with the given name
		//virtual EntityPtr CreatNode(const std::string& name);
		//virtual void AddNode(EntityPtr node);
		//virtual void RemoveNode(EntityPtr node);
		//virtual void RemoveNode(const std::string& name);

		//void AttachToNode(const std::string& node_name, EntityPtr entity);

		//! Creates an entity of the given type and adds it to the manager
		EntityPtr InstanceEntity(const std::string &type, const std::string &name = "default");

		EntityFactory *GetFactory() const;

		IDTranslator MakeIDTranslator() const;

		void CompressIDs();

		void AddEntity(EntityPtr entity);
		void RemoveEntity(EntityPtr entity);
		void RemoveEntityNamed(const std::string &name);
		void RemoveEntityById(ObjectID id);

		void AddPseudoEntity(EntityPtr pseudo_entity);

		void ReplaceEntity(ObjectID id, EntityPtr entity);

		//! Returns the Entity with the given name
		/*!
		* \param[in] name
		* Name of the desired Entity
		*
		* \param[in] throwIfNotFound
		* Throw rather than returning NULL
		*/
		EntityPtr GetEntity(const std::string &name, bool throwIfNotFound = true);
		//! Returns the Entity with the given ID
		/*!
		* Obviously can't return Pseudo-Entities, since they have no ID
		*
		* \param[in] id
		* ID of the desired Entity
		*
		* \param[in] throwIfNotFound
		* Throw rather than returning NULL
		*/
		EntityPtr GetEntity(ObjectID id, bool throwIfNotFound = true);

		//! Returns Entities
		/*!
		* Doesn't return Pseudo-Entities.
		*/
		const IDEntityMap &GetEntities() const;

		//! Returns Pseudo-Entities
		const EntitySet &GetPseudoEntities() const;

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
		* Removes and deletes all nodes in the scene.
		*/
		virtual void Clear();

		//! Updates nodes
		virtual void Update(float split);

		//! Draws nodes.
		virtual void Draw();

	protected:
		//unsigned int getTagFlag(const std::string &tag, bool generate);

		ObjectID getFreeID();

		std::string generateName(EntityPtr entity);

		void updateTags(EntityPtr &tag) const;

		//void updateEntity(EntityPtr entity, float split);

	protected:
		Renderer *m_Renderer;
		InputManager *m_InputManager;
		EntitySynchroniser *m_EntitySynchroniser;

		EntityFactory *m_EntityFactory;

		ObjectID m_NextId;
		typedef std::vector<ObjectID> ObjectIDStack;
		// Lists IDs between 0 and m_NextId that have been freed by Entity removal
		ObjectIDStack m_UnusedIds;

		// Used to quickly find entities by name (all entities, pseudo/non-pseudo are listed here)
		NameEntityMap m_EntitiesByName;

		// All non-pseudo-entities
		IDEntityMap m_Entities;
		// All pseudo-entities
		EntitySet m_PseudoEntities;

		// Entities with no paused tags
		EntityArray m_EntitiesToUpdate;

		typedef std::map<int, EntityPtr> EntityDepthMap;
		// Entities with no hidden tags
		EntityDepthMap m_EntitiesToDraw;

		// Bool part indicates whether the entity is a Pseudo-Entity
		typedef std::pair<EntityPtr, bool> EntityToAdd;
		typedef std::vector<EntityToAdd> EntityToAddArray;
		EntityToAddArray m_EntitiesToAdd;
		EntityArray m_EntitiesToRemove;
		//! The node to which all other nodes are children
		//EntityPtr m_RootNode;
		bool m_EntitiesLocked;

		TagFlagDictionaryPtr m_TagFlagDictionary;

		unsigned int m_ToDeleteFlags;
		unsigned int m_UpdateBlockedFlags;
		unsigned int m_DrawBlockedFlags;

		BlockingChangeMap m_ChangedUpdateStateTags;
		BlockedTagSet m_UpdateBlockedTags;

		BlockingChangeMap m_ChangedDrawStateTags;
		BlockedTagSet m_DrawBlockedTags;

	};

}

#endif
