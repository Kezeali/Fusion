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

/// Fusion
#include "FusionDrawable.h"

namespace FusionEngine
{
	class INode : public RefCounted
	{
		virtual void StoreState(Action &state) = 0;
		virtual void LoadState(const Action &state) = 0;

		virtual void Update(unsigned int split) = 0;
		virtual void Draw() = 0;

		/*!
		* \brief
		 * Sets the position of the node relative to its parent.
		 * 
		 * \param position
		 * The relative position to move to.
		 * 
		 * Set position will move a node to the specified position relative to its parent,
		 * and update the positions of all of the nodes children to maintain their relative
		 * positions.
		 */
		virtual void SetPosition(const Vector2 &position) = 0;
		/*!
		 * \brief
		 * Gets the position of the node relative to its parent.
		 * 
		 * \returns
		 * The position relative to the parent.
		 */
		virtual const Vector2 &GetPosition() const = 0;

		/*!
		 * \brief
		 * Sets the facing (degrees) of the node relative to its parent.
		 * 
		 * \param angle
		 * The angle in degrees relative to the objects parent.
		 */
		virtual void SetFacing(float angle) = 0;
		/*!
		 * \brief
		 * Gets the facing (degrees) of the node relative to its parent.
		 * 
		 * \returns
		 * The angle in degrees relative to the objects parent.
		 */
		virtual float GetFacing() const = 0;
	};

	/*!
	 * \brief
	 * FusionNode represents a moveable object with sub-objects.
	 *
	 * Each FusionNode can have multiple child nodes, but only one parent node.
	 * A FusionScene object handles creation and deletion of FusionNodes; as such no
	 * public constructor is available. Each fusion node can also have multiple
	 * FusionDrawables attached to it, which will be drawn by the scene.
	 *
	 * \remarks
	 * Nodes are designed to maintain the positions of child nodes relative to their
 	 * parent nodes. For instance, a 'space ship' node could have 'engine' and 'weapon'
	 * subnodes positioned at each corner of the sprite - when the SetPosition() 
	 * method is called on the 'space ship' node, the 'weapon' and 'engine' nodes are
	 * also moved to maintain their positions at the top and bottom of the sprite. 
	 * 
	 * \see
	 * Scene
	 */
	class SceneNode : public RefCounted
	{
	public:
		/*!
		 * \brief
		 * Constructor. Should only be called by FusionScene.
		 *
		 * \param[in] creator
		 * The scene which will control this node.
		 *
		 * \remarks
		 * This constructor is private and thus can only be called by scenes and other nodes,
		 * which are both friendly classes.
		 * Actually, the constructor is no longer private (I decided using friend classes was
		 * bad form), but that doesn't mean it should be used outside of the FusionScene /
		 * FusionNode based classes!
		 */
		SceneNode(Scene *creator);
		//! Virtual destructor.
		virtual ~SceneNode();

	public:
		typedef std::vector<SceneNode*> ChildNodeList;
		typedef std::vector<Action> StateList;

		virtual void StoreState(Action &state);
		virtual void LoadState(const Action &state);

		//! \todo Set controller
		void SetController(ISceneNodeController *controller);

		//! Updates all dynamic drawables
		virtual void Update(unsigned int split);

		/*!
		* \brief
		 * Sets the position of the node relative to its parent.
		 * 
		 * \param position
		 * The relative position to move to.
		 * 
		 * Set position will move a node to the specified position relative to its parent,
		 * and update the positions of all of the nodes children to maintain their relative
		 * positions.
		 */
		virtual void SetPosition(const Vector2 &position);
		/*!
		 * \brief
		 * Gets the position of the node relative to its parent.
		 * 
		 * \returns
		 * The position relative to the parent.
		 */
		virtual const Vector2 &GetPosition() const;

		/*!
		 * \brief
		 * Sets the facing (degrees) of the node relative to its parent.
		 * 
		 * \param angle
		 * The angle in degrees relative to the objects parent.
		 */
		virtual void SetFacing(float angle);
		/*!
		 * \brief
		 * Gets the facing (degrees) of the node relative to its parent.
		 * 
		 * \returns
		 * The angle in degrees relative to the objects parent.
		 */
		virtual float GetFacing() const;

		//! used internally
		virtual const Vector2 &_getDerivedPosition() const;
		//! used internally
		virtual float _getDerivedFacing() const;

		//! Used when drawing nodes.
		virtual const Vector2 &GetGlobalPosition() const;
		//! Used when drawing nodes.
		virtual float GetGlobalFacing() const;

		//! Saves the current state as the 'initial' state
		//void SetInitialState();
		//! Resets to the state saved as the 'initial' state
		//void ResetToInitialState();

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
		 * \param[in] position
		 * The initial position relative to the parent.
		 * 
		 * \param[in] facing
		 * The initial facing (degrees) relative to the parent.
		 *
		 * \returns
		 * A pointer to the node created.
		 */
		FusionNode *CreateChildNode(
			const Vector2 &position = Vector2::zero(),
			float facing = 0);

		/*!
		 * \brief
		 * Creates a node below this one in the tree.
		 * 
		 * \param[in] position
		 * The initial position relative to the parent.
		 * 
		 * \param[in] facing
		 * The initial facing (degrees) relative to the parent.
		 *
		 * \returns
		 * A pointer to the node created.
		 */
		FusionNode *CreateChildNode(
			const CL_Point &position = CL_Point(0,0),
			float facing = 0);
		
		/*!
		 * \brief
		 * Change the node's parent. Used internally.
		 * 
		 * \param parent
		 * A pointer to the desired parent.
		 */
		void _setParent(FusionNode* parent);

		//! Gets this node's parent (NULL if this is the root).
		FusionNode *GetParent() const;

		/*!
		 * \brief
		 * Add a child node to this node.
		 * 
		 * \param child
		 * A pointer to the child node.
		 */
		void AddChild(SceneNode *child);

		/*!
		 * \brief
		 * Removes a child node.
		 * 
		 * \param child
		 * A pointer to the child to remove.
		 */
		void RemoveChild(SceneNode *child);

		//! Self explanitory.
		ChildNodeList GetChildren() const;

		/*!
		 * \brief
		 * [Deprecated] Remove a child node by uid.
		 * 
		 * \param uid
		 * The unique id of the child.
		 *
		 * \returns
		 * A pointer to the removed child.
		 */
		//FusionNode *RemoveChild(const std::string &uid);

		/*!
		 * \brief
		 * This method removes and destroys the named child and all of its children.
		 * 
		 * \param child
		 * A pointer to the child to remove.
		 */
		void RemoveAndDestroyChild(FusionNode *child);

		/*!
		 * \brief
		 * Removes and destroys all children of this node. 
		 */
		void RemoveAndDestroyAllChildren();

		/*!
		 * \brief
		 * To be called in the event of transform changes to this node that make its derived
		 * position, etc. out of date.
		 *
		 * \remarks
		 * This not only tags the node state as being 'dirty', it also tells it's parent about
		 * it's dirtiness, via _requestUpdate(), so it will get an update next time round.
		 */
		void NeedUpdate();

		/*!
		 * \brief
		 * Allows children to add themselves to the update list.
		 *
		 * Adds a child to the m_ChildrenToUpdate list. Called by children in NeedUpdate().
		 */
		void _requestUpdate(FusionNode *child);

		/*!
		 * \brief
		 * Internal method to update the Node.
		 *
		 * Updates this scene node and any relevant children to incorporate transforms etc.
		 *
		 * \param cascade
		 * Force the node to update its children, even if it thinks it doesn't need to. This
		 * allows the root node to make the _update() cascade down the graph.
		 *
		 * \param parentHasChanged
		 * This flag indicates that the parent form has changed, so the child should
		 * update its absolute attributes even if it hasn't changed itself.
		 */
		void _update(bool cascade, bool parentHasChanged);

		/*!
		 * \brief
	   * Triggers the node to update it's derived transforms.
     */
     virtual void _updateFromParent(void) const;

		 /*!
		 * \brief
	   * Draws all attached FusionDrawables.
		 *
		 * Used internally by FusionScene.
		 *
		 * \param cascade
		 * If this is true, the _draw() will cascade down the graph.
     */
     virtual void _draw(bool cascade) const;

		 /*!
		  * \brief
			* Updates all updateable drawables.
			*
			* Used internally by FusionScene.
			*
			* \param[in] split Time since last update
			* \param[in] cascade
			* If this is true, the _drawupdate() will cascade down the graph.
			*/
		 //virtual void _drawupdate(unsigned int split, bool cascade);

		 /*!
		  * \brief
			* Sorts child nodes by their depth attribute.
			*
			* This sorts nodes so their position in the scene graph reflects their m_Depth
			* attribute, in other words nodes with a lower m_Depth within a branch are drawn
			* first.
			*
			* \remarks
			* <b>On Efficiancy:</b><br>
			* It is more efficiant to use FusionScene#Sort() in FusionScene <b>if</b>
			* drawing is done in flat mode (by calling eachnode->_draw(false);). 
			* <p>
			* <b>Only</b> use this method if you plan on doing something like:
			* <br>
			* myobject->GetNode()->_draw(true);
			* </p>
			*/
		 virtual void _sortChildren(bool cascade);

		 /*!
		 * \brief
		 * Allows children to request a branch resort when their depth changes.
		 *
		 * Sets m_NeedChildSort to true if applicable. Called by children
		 * in NeedUpdate().
		 */
		 virtual void _requestSort(FusionNode *child);

		 //! Gets the value of m_Depth.
		 virtual int GetDepth() const;
		 //! Sets the value of m_Depth.
		 virtual void SetDepth(int depth);

		 /*!
		 * \brief
		 * To be called in the event of depth changes.
		 *
		 * This requests a branch sort from the parent. Sorting is expensive so call sparsely.
		 */
		 virtual void NeedSort();

		 //! Sets m_AllowChildSort to the given value.
		 virtual void SetAllowSort(bool allow, bool cascade);
		 //! Returns the value of m_AllowChildSort.
		 virtual bool GetAllowSort() const;

	private:
		//! See definition of FusionNode::IsInSceneGraph().
		bool m_InSceneGraph;
		//! The scene which created this node. Used to create children.
		FusionScene *m_Creator;
		//! The parent of this node.
		FusionNode *m_Parent;
		//! Children direcly decending this node.
		ChildNodeList m_Children;
		//! Drawables attached to this node.
		DrawableList m_AttachedObjects;
		//! Dynamic Drawables attached to this node.
		DrawableList m_AttachedDynamics;
		//! The depth at which this node is drawn.
		int m_Depth;

		typedef std::set<FusionNode*> ChildUpdateSet;
		/*! 
		 * \brief
		 * List of children which need updating, used if self is not out of date but
		 * children are.
		 */
		mutable ChildUpdateSet m_ChildrenToUpdate;

		/*!
		 * \brief
		 * Flag to indicate own transform from parent is out of date.
		 *
		 * \remarks
		 * Set to true by NeedUpdate() or whenever the derived positions are out-of-date. <br>
		 * Set to false when update is no longer required. ie. After calling
		 * _updateFromParent().
		 */
		mutable bool m_NeedParentUpdate;

		/*!
		 * \brief
		 * Flag indicating that all children need to be updated.
		 *
		 * \remarks
		 * Set to true by NeedUpdate(), which should be called whenever relative positions are
		 * changed. <br>
		 * Set to false by _update(), after children have been told to update.
		 */
		mutable bool m_NeedChildUpdate;

		/*!
		 * \brief
		 * Flag indicating that parent has been notified about update request.
		 *
		 * \remarks
		 * Set to true when _requestUpdate() has been called on the parent. <br>
		 * Set to false when parent has forfilled the request (ie. in _update()), or update is
		 * no longer required (eg. in CancelUpdate().)
		 */
		mutable bool m_ParentNotified;

		/*!
		 * \brief
		 * Flag indicating that children should be sorted.
		 *
		 * \remarks
		 * Set to true in FusionNode::_requestSort() and FusionNode::SetAllowSort(). <br>
		 * Set to false when children have been sorted. The normal place for this to happen is
		 * the _update() method.
		 */
		mutable bool m_NeedChildSort;

		/*!
		 * \brief
		 * Flag indicating that child sorting is allowd.
		 *
		 * Allows FusionNode::NeedSort() to set m_NeedChildSort to true. Also makes
		 * FusionNode::AddChild() insert new children in the correct order.
		 */
		bool m_AllowChildSort;

		//! The position of the node relative to its parent.
		Vector2 m_Position;
		//! The facing (degrees) of the node relative to its parent.
		float m_Facing;

		//! The absolute position of the node (used internally).
		mutable Vector2 m_DerivedPosition;
		//! The absolute facing (degrees) of the node (used internally).
		mutable float m_DerivedFacing;

		//! The identity position of the node relative to its parent.
		Vector2 m_InitialPosition;
		//! The identity facing of the node relative to its parent.
		float m_InitialFacing;

	};

	//! Returns whether one is less than two. \see std::sort()
	bool DepthIsLess(FusionNode *one, FusionNode *two)
	{
		return (one->GetDepth() < two->GetDepth());
	}

}

#endif
