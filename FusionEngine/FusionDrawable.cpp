
#include "FusionDrawable.h"

// Fusion
#include "FusionNode.h"

using namespace FusionEngine;

FusionDrawable::FusionDrawable(void)
: m_ParentNode(0)
{
}

FusionDrawable::~FusionDrawable(void)
{
	if (m_ParentNode)
	{
		m_ParentNode->DetachDrawable(this);
	}
}

FusionNode *FusionDrawable::GetParentSceneNode() const
{
	return m_ParentNode;
}

void FusionDrawable::_notifyAttached(FusionNode *parent)
{
	m_ParentNode = parent;
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
