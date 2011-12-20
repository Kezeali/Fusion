/*
Copyright (c) 2006-2007 Fusion Project Team
 
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
*/

#ifndef Header_FusionEngine_FusionNode
#define Header_FusionEngine_FusionNode

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionEntity.h"

namespace FusionEngine
{

	typedef std::tr1::shared_ptr<Node> NodePtr;

	/*!
	 * \brief
	 * Node in the EntityManager graph.
	 *
	 * Each Node represents a level in the entity hirachy. Each node
	 * contains a list of child Nodes, which can be Entities or
	 * basic Nodes. If a Node is deleted, all of its child Nodes will
	 * be deleted.
	 *
	 * \see
	 * EntityManager | Entity
	 */
	class Node
	{
	public:
		/*!
		 * \brief
		 * Constructor.
		 */
		Node();
		//! Constructor. Names the node.
		Node(const std::string& name);

		//! Virtual destructor.
		virtual ~Node();

	public:
		typedef std::vector<NodePtr> ChildNodeList;
		typedef std::vector<EntityPtr> AttachedEntityList;

		/*!
		 * (mainly)Internal method for setting whether the node is in the scene graph.
		 */
		void _setInSceneGraph(bool inSceneGraph);

		/*!
		 * \brief
		 * Notifies this SceneNode that it is the root scene node. 
		 *
		 * Although this is simmilar in functionality to _setInSceneGraph(), is preffered when
		 * simply notifying the root node as it assumes no sub-nodes to be attached, and thus
		 * has a smaller overhead.
		 *
		 * \remarks
		 * Only the Scene manager should call this!
		 */
		virtual void _notifyRootNode() { m_InSceneGraph = true; }

		/*!
		 * \brief
		 * Determines whether this node is in the scene graph.
		 *
		 * ie. Whether it's ulitimate ancestor is the root scene node.
		 */
		bool IsInSceneGraph() const;

		/*!
		 * \brief
		 * Creates a node below this one in the tree.
		 * 
		 * \param[in] name
		 * The name of the node.
		 *
		 * \returns
		 * A pointer to the node created.
		 */
		NodePtr CreateChildNode(const std::string &name);

		/*!
		 * \brief
		 * Change the node's parent. Used internally.
		 * 
		 * \param parent
		 * A pointer to the desired parent.
		 */
		void _setParent(NodePtr parent);

		//! Gets this node's parent (NULL if this is the root).
		NodePtr GetParent() const;

		//! Self explanitory.
		ChildNodeList GetChildren() const;

		/*!
		 * \brief
		 * Add a child node to this node.
		 * 
		 * \param child
		 * A pointer to the child node.
		 */
		void AdoptChild(NodePtr child);

		/*!
		 * \brief
		 * Removes a child node.
		 * 
		 * \param child
		 * A pointer to the child to remove.
		 */
		void RemoveChild(NodePtr child);

		/*!
		 * \brief
		 * Removes and destroys all children of this node. 
		 */
		void ClearChildren();

		void AttachEntity(EntityPtr entity);
		void DetachEntity(EntityPtr entity);

	private:
		std::string m_Name;
		//! See definition of FusionNode::IsInSceneGraph().
		bool m_InSceneGraph;
		//! The parent of this node.
		NodePtr m_Parent;
		//! Children direcly decending this node.
		ChildNodeList m_Children;

		AttachedEntityList m_AttachedEntities;

		bool m_Update;
		bool m_Draw;

	};

}

#endif
