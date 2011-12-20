
#include "FusionDrawable.h"

// Fusion
#include "FusionNode.h"

using namespace FusionEngine;

FusionDrawable::FusionDrawable(void)
: m_ParentNode(0),
m_Dynamic(false)
{
}

FusionDrawable::~FusionDrawable(void)
{
	// Shouldn't need to do this, but just in case we still have a parent,
	//  this'll be necessary.
	if (m_ParentNode)
	{
		m_ParentNode->DetachDrawable(this);
	}
}

FusionNode *FusionDrawable::GetParentSceneNode() const
{
	return m_ParentNode;
}

void FusionDrawable::_notifyAttached(FusionNode *parent, bool dynamic)
{
	m_ParentNode = parent;
	m_Dynamic = dynamic;
}

bool FusionDrawable::IsAttached() const
{
	return (m_ParentNode != 0);
}

bool FusionDrawable::IsInScene() const
{
	if (m_ParentNode != 0)
		return m_ParentNode->IsInSceneGraph();
	return false;
}
