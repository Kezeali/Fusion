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

	/*!
	 * \brief
	 * Updates / draws entity objects.
	 *
	 * \see
	 * Entity
	 */
	class EntityManager
	{
	public:
		//! Constructor
		EntityManager();
		//! Destructor
		virtual ~EntityManager();

	public:
		typedef std::tr1::weak_ptr<Entity> EntityWPtr;
		//! A list of nodes
		typedef std::tr1::unordered_map<std::string, EntityPtr> EntityMap;
		typedef std::vector<EntityPtr> EntityList;

		typedef boost::bimap<std::string, unsigned int> TagFlagMap;
		typedef TagFlagMap::value_type TagDef;

		//! Gives you access to the root node
		//virtual EntityPtr GetRootNode() const;

		//! Creates a new node with the given name
		//virtual EntityPtr CreatNode(const std::string& name);
		//virtual void AddNode(EntityPtr node);
		//virtual void RemoveNode(EntityPtr node);
		//virtual void RemoveNode(const std::string& name);

		//void AttachToNode(const std::string& node_name, EntityPtr entity);

		void AddEntity(EntityPtr entity);
		void RemoveEntity(EntityPtr entity);
		void RemoveEntityNamed(const std::string &name);

		EntityPtr GetEntity(const std::string &name);

		bool AddTag(const std::string &entity_name, const std::string &tag);
		bool AddTag(EntityPtr, const std::string &tag);
		void RemoveTag(const std::string &entity_name, const std::string &tag);
		void RemoveTag(EntityPtr, const std::string &tag);

		bool CheckTag(EntityPtr entity, const std::string &tag);
		StringVector GetTags(EntityPtr entity);

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
		unsigned int getTagFlag(const std::string &tag, bool generate);

	protected:
		InputManager *m_InputManager;

		//! Used to quickly find entities by name
		EntityMap m_Entities;
		EntityList m_EntitiesToRemove;
		//! The node to which all other nodes are children
		//EntityPtr m_RootNode;
		bool m_EntitiesLocked;

		unsigned int m_ToDeleteFlags;
		unsigned int m_UpdateBlockedTags;
		unsigned int m_DrawBlockedTags;

	};

}

#endif
