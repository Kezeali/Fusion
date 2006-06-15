
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"

/// Class
#include "FusionScene.h"

using namespace FusionEngine;

FusionScene::FusionScene()
{
	m_RootNode = new FusionNode(this);
	m_RootNode->_notifyRootNode();
}

FusionScene::~FusionScene()
{
	CleanScene();
	delete m_RootNode;
}

void FusionScene::CleanScene()
{
	NodeList::iterator it;
	for (it = m_SceneNodes.begin(); it < m_SceneNodes.end(); ++it)
	{
		delete (*it);
	}
	m_SceneNodes.clear();
}

FusionNode *FusionScene::CreateNode()
{
	FusionNode *node = new FusionNode(this);
	node->_setParent(m_RootNode);
	return node;
}

void FusionScene::DestroySceneNode(FusionNode *node)
{
	if (node == m_RootNode)
		throw new CL_Error("Ai' pitty da foo who try to delete da root node!");

	m_SceneNodes.remove(node);
	delete node;
}

void FusionScene::Draw()
{
	m_RootNode->_update(true, false);

	for (FusionNode::ChildNodeList::iterator it = m_SceneNodes.begin();
		it != m_SceneNodes.end; ++it)
	{
		(*it)->_draw();
	}
}
