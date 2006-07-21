#ifndef Header_FusionEngine_FusionNode
#define Header_FusionEngine_FusionNode

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

/// Fusion
#include "FusionDrawable.h"

namespace FusionEngine
{
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
	 * FusionScene | FusionShip | FusionShipWeapon | FusionShipEngine.
	 */
	class FusionNode
	{
	public:
		/*!
		 * \brief
		 * Constructor. Should only be called by FusionScene.
		 *
		 * \param creator
		 * The scene which will control this node.
		 *
		 * \remarks
		 * This constructor is private and thus can only be called by scenes and other nodes,
		 * which are both friendly classes.
		 * Actually, the constructor is no longer private (I decided using friend classes was
		 * bad form), but that doesn't mean it should be used outside of the FusionScene /
		 * FusionNode based classes!
		 */
		FusionNode(FusionScene *creator);
		//! Virtual destructor.
		virtual ~FusionNode();

	public:
		typedef std::vector<FusionNode*> ChildNodeList;
		typedef std::vector<FusionDrawable*> DrawableList;

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
		virtual void SetPosition(const CL_Vector2 &position);
		/*!
		 * \brief
		 * Gets the position of the node relative to its parent.
		 * 
		 * \returns
		 * The position relative to the parent.
		 */
		virtual const CL_Vector2 &GetPosition() const;

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

		virtual const CL_Vector2 &_getDerivedPosition() const;
		virtual float _getDerivedFacing() const;

		//! Used when drawing nodes.
		virtual const CL_Vector2 &GetGlobalPosition() const;
		//! Used when drawing nodes.
		virtual float GetGlobalFacing() const;

		void SetInitialState();
		void ResetToInitialState();

		//! Self explanitory.
		ChildNodeList GetChildren() const;

		//! Self explanitory.
		DrawableList GetAttachedDrawables() const;

		/*!
		 * \brief
		 * Attaches a FusionDrawable object to the node.
		 * 
		 * \param draw
		 * A pointer to the FusionDrawable.
		 */
		void AttachDrawable(FusionDrawable *draw);
		
		/*!
		 * \brief
		 * Removes an attached drawable.
		 * 
		 * \param child
		 * A pointer to the attached object to remove.
		 */
		void DetachDrawable(FusionDrawable *draw);

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
		 * Only SceneManager should call this!
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
		 * Creates a node below this on in the tree.
		 * 
		 * \param position
		 * The initial position relative to the parent.
		 * 
		 * \param facing
		 * The initial facing (degrees) relative to the parent.
		 *
		 * \returns
		 * A pointer to the node created.
		 */
		FusionNode *CreateChildNode(
			const CL_Vector2 &position = CL_Vector2::ZERO,
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
		void AddChild(FusionNode *child);

		/*!
		 * \brief
		 * Removes a child node.
		 * 
		 * \param child
		 * A pointer to the child to remove.
		 */
		void RemoveChild(FusionNode *child);

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
			* Sorts child nodes by their depth attribute.
			*
			* This sorts nodes so their position in the scene graph reflects their m_Depth
			* attribute, in other words nodes with a higher m_Depth within a branch are drawn
			* first.
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
		struct DepthIsGreater
			: std::binary_function<const FusionNode *, const FusionNode *, bool>
		{
			bool operator()(const FusionNode *x, const FusionNode *y) const
			{
				return (x->GetDepth() > y->GetDepth());
			}
		};

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
		CL_Vector2 m_Position;
		//! The facing (degrees) of the node relative to its parent.
		float m_Facing;

		//! The absolute position of the node (used internally).
		mutable CL_Vector2 m_DerivedPosition;
		//! The absolute facing (degrees) of the node (used internally).
		mutable float m_DerivedFacing;

		//! The identity position of the node relative to its parent.
		CL_Vector2 m_InitialPosition;
		//! The identity facing of the node relative to its parent.
		float m_InitialFacing;

	};

}

#endif
