#ifndef Header_FusionEngine_FusionScene
#define Header_FusionEngine_FusionScene

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"
#include "FusionInputHandler.h"

#include <boost/bimap.hpp>

namespace FusionEngine
{

	//class EntityMap
	//{
	//public:
	//	typedef std::map<ObjectID, EntityPtr> IDEntityMap;
	//	typedef std::tr1::unordered_map<std::string, EntityPtr> NameEntityMap;

	//	const IDEntityMap &by_id() const;
	//	const NameEntityMap &by_name() const;

	//	bool insert(EntityPtr entity);

	//protected:
	//	IDEntityMap m_EntitiesByID;
	//	NameEntityMap m_EntitiesByName;
	//};

	typedef std::vector<EntityPtr> EntityArray;

	/*!
	 * \brief
	 * Updates / draws entity objects.
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
		EntityManager();
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
		const EntityArray &GetPseudoEntities() const;

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

		//void updateEntity(EntityPtr entity, float split);

	protected:
		InputManager *m_InputManager;

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
		// Entities with no hidden tags
		EntityArray m_EntitiesToDraw;

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
