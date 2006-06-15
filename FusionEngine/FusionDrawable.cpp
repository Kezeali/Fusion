#include "FusionDrawable.h"

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

FusionNode *FusionDrawable::GetParentSceneNode()
{
	return m_ParentNode;
}

void FusionDrawable::_notifyAttached(FusionNode *parent)
{
	m_ParentNode = parent;
}

bool FusionDrawable::IsAttached()
{
	return (m_ParentNode != NULL);
}

bool FusionDrawable::IsInScene()
{
	if (mParentNode != 0)
	{
		if (mParentIsTagPoint)
		{
			TagPoint* tp = static_cast<TagPoint*>(mParentNode);
			return tp->getParentEntity()->isInScene();
		}
		else
		{
			SceneNode* sn = static_cast<SceneNode*>(mParentNode);
			return sn->isInSceneGraph();
		}
	}
	else
	{
		return false;
	}
}
