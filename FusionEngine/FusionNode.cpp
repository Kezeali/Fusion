
/// Fusion
#include "FusionScene.h"
#include "FusionNode.h"

using namespace FusionEngine;

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
m_Position(CL_Vector2::ZERO),
m_Facing(0),
m_DerivedPosition(CL_Vector2::ZERO),
m_DerivedFacing(0)
{
	NeedUpdate();
}


FusionNode::~FusionNode()
{
	// Tell all Drawables that they've been detached.
	DrawableList::iterator it;
	for ( it = m_AttachedObjects.begin(); it != m_AttachedObjects.end(); ++it )
	{
		(*it)->_notifyAttached(NULL);
	}
	m_AttachedObjects.clear();
}

void FusionNode::AttachDrawable(FusionDrawable *draw)
{
	m_AttachedObjects.push_back(draw);
	draw->_notifyAttached(this);
}

void FusionNode::DetachDrawable(FusionDrawable *draw)
{
	m_AttachedObjects.remove(draw);
	draw->_notifyAttached(NULL);
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

void FusionNode::SetPosition(const CL_Vector2 &position)
{
	m_Position = position;

	NeedUpdate();
}

const CL_Vector2 &FusionNode::GetPosition() const
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

const CL_Vector2 &FusionNode::_getDerivedPosition() const
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

const CL_Vector2 &FusionNode::GetGlobalPosition() const
{
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

	// Exit if there is reason to update
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

		m_ChildrenToUpdate.clear();
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

		m_ChildrenToUpdate.clear();
	}

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
			(*it)->_setInSceneGraph(inSceneGrpah);
		}
	}
}

bool FusionNode::IsInSceneGraph() const
{
	return m_InSceneGraph;
}

FusionNode *FusionNode::CreateChildNode(const CL_Vector2 &position, float facing)
{
	assert(m_Creator);

	FusionNode *child = m_Creator->CreateNode();
	child->SetPosition(position);
	child->SetFacing(facing);
	AddChild(child);

	return child;
}

void FusionNode::AddChild(FusionNode* child)
{
	assert(!child->GetParent());

	m_Children.push_back(child);
	child->_setParent(this);

	// The lazy way to insert children.
	//! \todo Insert children by depth
	if (m_AllowChildSort)
		m_NeedChildSort = true;
}

void FusionNode::RemoveChild(FusionNode* child)
{
	m_Children.erase(child);
}

void FusionNode::RemoveAndDestroyChild(FusionNode *child)
{
	delete child;

	m_Children.erase(child);
}

void FusionNode::RemoveAndDestroyAllChildren()
{
	ChildNodeList::iterator it;
	for (it = m_Children.begin(); it != m_Children.end() ++it)
	{
		delete (*it);
	}
	m_Children.clear();
}

void FusionNode::_draw(bool cascade) const
{
	{
		DrawableList::iterator it;
		for (it = m_AttachedObjects.begin(); it != m_AttachedObjects.end(); ++it)
		{
			(*it)->Draw();
		}
	}

	if (cascade)
	{
		ChildNodeList::iterator it;
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
		ChildList::iterator it;
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
		ChildList::iterator it;
		for (it = m_Children.begin(); it != m_Children.end(); ++it)
		{
			(*it)->SetAllowSort(allow, true);
		}
	}
}