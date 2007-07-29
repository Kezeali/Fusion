
/// Fusion
#include "FusionScene.h"
#include "FusionNode.h"

namespace FusionEngine
{

	FusionNode::FusionNode(FusionScene *creator)
		: m_Creator(creator),
		m_Parent(0),
		m_NeedChildUpdate(false),
		m_NeedParentUpdate(false),
		m_ParentNotified(false),
		m_InSceneGraph(false),
		m_AllowChildSort(false),
		m_NeedChildSort(false),
		m_Depth(0),
		m_Position(Vector2::ZERO),
		m_Facing(0),
		m_DerivedPosition(Vector2::ZERO),
		m_DerivedFacing(0)
	{
		NeedUpdate();
	}


	FusionNode::~FusionNode()
	{
		// This tells all of this nodes children that they have been orphaned
		_setInSceneGraph(false);

		// Tell all Drawables that they've been detached.
		DrawableList::iterator it = m_AttachedObjects.begin();
		for (; it != m_AttachedObjects.end(); ++it )
		{
			(*it)->_notifyAttached(NULL);
		}
		m_AttachedObjects.clear();
	}

	void FusionNode::AttachDrawable(FusionDrawable *draw)
	{
		m_AttachedObjects.push_back(draw);
		draw->_notifyAttached(this, false);
	}

	void FusionNode::AttachDynamicDrawable(FusionDrawable *draw)
	{
		m_AttachedDynamics.push_back(draw);
		draw->_notifyAttached(this, true);
	}

	void FusionNode::DetachDrawable(FusionDrawable *draw)
	{
		if (draw->IsDynamic())
		{
			// Dynamic drawables
			DrawableList::iterator it = m_AttachedDynamics.begin();
			for (; it != m_AttachedDynamics.end(); ++it )
			{
				if ((*it) == draw)
				{
					m_AttachedDynamics.erase(it);
					break;
				}
			}
		}
		else
		{
			// Normal drawables
			DrawableList::iterator it = m_AttachedObjects.begin();
			for (; it != m_AttachedObjects.end(); ++it )
			{
				if ((*it) == draw)
				{
					m_AttachedObjects.erase(it);
					break;
				}
			}
		}

		draw->_notifyAttached(NULL);
	}

	void FusionNode::DetachAndDestroyAllDrawables()
	{
		{
			DrawableList::iterator it = m_AttachedObjects.begin();
			for (; it != m_AttachedObjects.end(); ++it)
			{
				delete (*it);
			}
		}

		m_AttachedObjects.clear();

		{
			DrawableList::iterator it = m_AttachedDynamics.begin();
			for (; it != m_AttachedDynamics.end(); ++it)
			{
				delete (*it);
			}
		}

		m_AttachedDynamics.clear();
	}

	void FusionNode::UpdateDynamics(unsigned int split)
	{
		DrawableList::iterator it = m_AttachedDynamics.begin();
		for (; it != m_AttachedDynamics.end(); ++it)
		{
			(*it)->Update(split);
		}
	}

	void FusionNode::NeedUpdate()
	{
		m_NeedParentUpdate = true;
		m_NeedChildUpdate = true;

		if (m_Parent && !m_ParentNotified)
		{
			m_Parent->_requestUpdate(this);
			m_ParentNotified = true;
		}

		// This node is no longer the update controler, so we can empty this set.
		m_ChildrenToUpdate.clear();
	}

	void FusionNode::_requestUpdate(FusionNode *child)
	{
		// If everything is going to be updated anyway, this doesn't matter.
		if (m_NeedChildUpdate)
		{
			return;
		}

		m_ChildrenToUpdate.insert(child);

		// Just in case NeedUpdate hasn't been called on this node, notify parent.
		if (m_Parent && !m_ParentNotified)
		{
			m_Parent->_requestUpdate(this);
			m_ParentNotified = true;
		}
	}

	void FusionNode::SetPosition(const Vector2 &position)
	{
		m_Position = position;

		NeedUpdate();
	}

	const Vector2 &FusionNode::GetPosition() const
	{
		return m_Position;
	}

	void FusionNode::SetFacing(float angle)
	{
		m_Facing = angle;

		NeedUpdate();
	}

	float FusionNode::GetFacing() const
	{
		return m_Facing;
	}

	const Vector2 &FusionNode::_getDerivedPosition() const
	{
		if (m_NeedParentUpdate)
		{
			_updateFromParent();
			m_NeedParentUpdate = false;
		}

		return m_DerivedPosition;
	}

	float FusionNode::_getDerivedFacing() const
	{
		if (m_NeedParentUpdate)
		{
			_updateFromParent();
			m_NeedParentUpdate = false;
		}

		return m_DerivedFacing;
	}

	const Vector2 &FusionNode::GetGlobalPosition() const
	{
		// Note: this is a good method because calculations only have to be made up to
		//  the highest node that has moved (and don't forget that this time!)
		return _getDerivedPosition();
	}

	float FusionNode::GetGlobalFacing() const
	{
		return _getDerivedFacing();
	}

	void FusionNode::SetInitialState()
	{
		m_InitialPosition = m_Position;
		m_InitialFacing = m_Facing;
	}

	void FusionNode::ResetToInitialState()
	{
		m_Position = m_InitialPosition;
		m_Facing = m_InitialFacing;

		NeedUpdate();
	}

	void FusionNode::_update(bool cascade, bool parentHasChanged)
	{
		// Allow further updates to be requested.
		m_ParentNotified = false;

		// Exit if there is no reason to update
		if (!cascade && !m_NeedParentUpdate && !m_NeedChildUpdate && !parentHasChanged)
		{
			return;
		}

		if (m_NeedParentUpdate || parentHasChanged)
		{
			_updateFromParent();
			m_NeedParentUpdate = false;
		}

		// Resort this nodes children if it has been requested.
		if (m_NeedChildSort)
		{
			_sortChildren(false);
		}

		if (m_NeedChildUpdate || parentHasChanged)
		{
			// This has been updated (ie. this has changed) so parentHasChanged is set to true
			// to ensure that all children update their translations.
			ChildNodeList::iterator it;
			for (it = m_Children.begin(); it != m_Children.end(); ++it)
			{
				(*it)->_update(true, true);
			}
		}
		else
		{
			// If this hasn't been updated, then only children who requested an update should be
			// updated.
			ChildUpdateSet::iterator it;
			for (it = m_ChildrenToUpdate.begin(); it != m_ChildrenToUpdate.end(); ++it)
			{
				(*it)->_update(true, false);
			}
		}

		m_ChildrenToUpdate.clear();
		m_NeedChildUpdate = false;
	}

	void FusionNode::_updateFromParent() const
	{
		if (m_Parent)
		{
			m_DerivedPosition = m_Position + m_Parent->_getDerivedPosition();
			m_DerivedFacing = m_Facing + m_Parent->_getDerivedFacing();
		}
		else
		{
			// Root node, no parent
			m_DerivedPosition = m_Position;
			m_DerivedFacing = m_Facing;
		}
	}

	FusionNode::ChildNodeList FusionNode::GetChildren() const
	{
		return m_Children;
	}

	FusionNode::DrawableList FusionNode::GetAttachedDrawables() const
	{
		return m_AttachedObjects;
	}

	FusionNode::DrawableList FusionNode::GetAttachedDynamicDrawables() const
	{
		return m_AttachedDynamics;
	}

	FusionNode::DrawableList FusionNode::GetAllAttachedDrawables() const
	{
		DrawableList list;
		DrawableList::iterator dest = list.begin();
		std::merge(m_AttachedObjects.begin(), m_AttachedObjects.end(), 
			m_AttachedDynamics.begin(), m_AttachedDynamics.end(),
			dest);

		return list;
	}

	void FusionNode::_setParent(FusionNode* parent)
	{
		m_Parent = parent;

		m_ParentNotified = false;
		NeedUpdate();

		if (parent)
		{
			_setInSceneGraph(parent->IsInSceneGraph());
		}
		else
		{
			_setInSceneGraph(false);
		}
	}

	FusionNode *FusionNode::GetParent() const
	{
		return m_Parent;
	}

	void FusionNode::_setInSceneGraph(bool inSceneGraph)
	{
		if (inSceneGraph != m_InSceneGraph)
		{
			m_InSceneGraph = inSceneGraph;

			ChildNodeList::iterator it;
			for (it = m_Children.begin(); it != m_Children.end(); ++it)
			{
				(*it)->_setInSceneGraph(inSceneGraph);
			}
		}
	}

	bool FusionNode::IsInSceneGraph() const
	{
		return m_InSceneGraph;
	}

	FusionNode *FusionNode::CreateChildNode(const Vector2 &position, float facing)
	{
		cl_assert(m_Creator);

		FusionNode *child = m_Creator->CreateNode();
		child->SetPosition(position);
		child->SetFacing(facing);
		AddChild(child);

		return child;
	}

	FusionNode *FusionNode::CreateChildNode(const CL_Point &position, float facing)
	{
		cl_assert(m_Creator);

		FusionNode *child = m_Creator->CreateNode();
		child->SetPosition(Vector2(position.x, position.y));
		child->SetFacing(facing);
		AddChild(child);

		return child;
	}

	void FusionNode::AddChild(FusionNode* child)
	{
		// Don't give nodes multiple parents!
		if (child->GetParent())
			child->GetParent()->RemoveChild(child);

		m_Children.push_back(child);
		child->_setParent(this);

		if (m_AllowChildSort)
			m_NeedChildSort = true;
	}

	void FusionNode::RemoveChild(FusionNode* child)
	{
		ChildNodeList::iterator it = m_Children.begin();
		for (; it != m_Children.end(); ++it)
		{
			m_Children.erase(it);
		}

		child->_setParent(NULL);
	}

	void FusionNode::RemoveAndDestroyChild(FusionNode *child)
	{
		RemoveChild(child);

		delete child;
	}

	void FusionNode::RemoveAndDestroyAllChildren()
	{
		ChildNodeList::iterator it;
		for (it = m_Children.begin(); it != m_Children.end(); ++it)
		{
			// Cascade
			(*it)->RemoveAndDestroyAllChildren();
			// Then delete
			delete (*it);
		}

		m_Children.clear();
	}

	void FusionNode::_draw(bool cascade) const
	{
		// Nodes shouldn't be drawn if they haven't been added to the graph
		// (although the scene might try to because of the way it finds nodes to draw.)
		if (m_InSceneGraph == false)
			return;

		DrawableList::const_iterator it = m_AttachedObjects.begin();
		for (; it != m_AttachedObjects.end(); ++it )
		{
			(*it)->Draw();
		}

		if (cascade)
		{
			ChildNodeList::const_iterator it;
			for (it = m_Children.begin(); it != m_Children.end(); ++it)
			{
				(*it)->_draw(true);
			}
		}
	}

	void FusionNode::_sortChildren(bool cascade)
	{
		std::sort(m_Children.begin(), m_Children.end(), DepthIsLess);

		if (cascade)
		{
			ChildNodeList::iterator it;
			for (it = m_Children.begin(); it != m_Children.end(); ++it)
			{
				(*it)->_sortChildren(true);
			}
		}

		m_NeedChildSort = false;
	}

	int FusionNode::GetDepth() const
	{
		return m_Depth;
	}

	void FusionNode::SetDepth(int depth)
	{
		m_Depth = depth;

		NeedSort();
	}

	void FusionNode::NeedSort()
	{
		m_Parent->_requestSort(this);
		// This requests a global sort (used for flat-scene drawing)
		m_Creator->_requestSort();
	}

	void FusionNode::_requestSort(FusionNode *child)
	{
		if (m_AllowChildSort)
		{
			m_NeedChildSort = true;
		}
	}

	bool FusionNode::GetAllowSort() const
	{
		return m_AllowChildSort;
	}

	void FusionNode::SetAllowSort(bool allow, bool cascade)
	{
		m_AllowChildSort = allow;

		if (cascade)
		{
			ChildNodeList::iterator it;
			for (it = m_Children.begin(); it != m_Children.end(); ++it)
			{
				(*it)->SetAllowSort(allow, true);
			}
		}
	}

}
