
#include "FusionEngineCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"

/// Class
#include "FusionScene.h"

using namespace FusionEngine;

FusionScene::FusionScene()
: m_FlatScene(false),
m_EnableSorting(false)
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
	SceneNodeList::iterator it;
	for (it = m_SceneNodes.begin(); it < m_SceneNodes.end(); ++it)
	{
		delete (*it);
	}
	m_SceneNodes.clear();
}

FusionNode *FusionScene::CreateNode()
{
	FusionNode *node = new FusionNode(this);
	// Remember this node
	m_SceneNodes.push_back(node);

	// I don't know why I had CreateNode() setting the parent
	//  but I'm pretty sure it shouldn't.
	//node->_setParent(m_RootNode);
	return node;
}

void FusionScene::DestroySceneNode(FusionNode *node, bool destroy_children, bool destroy_drawables)
{
	if (node == m_RootNode)
		throw new CL_Error("Ai' pitty da foo who try to delete da root node!");

	SceneNodeList::iterator it = m_SceneNodes.begin();
	for (; it != m_SceneNodes.end(); ++it)
	{
		if ((*it) == node)
			m_SceneNodes.erase(it);
	}

	if (destroy_children)
		node->RemoveAndDestroyAllChildren();

	if (destroy_drawables)
		node->DetachAndDestroyAllDrawables();

	delete node;
}

void FusionScene::UpdateDynamics(unsigned int split)
{
	FusionNode::ChildNodeList::iterator it = m_SceneNodes.begin();
	for (; it != m_SceneNodes.end(); ++it)
	{
		(*it)->UpdateDynamics(split);
	}
}

void FusionScene::Draw()
{
	m_RootNode->_update(true, false);

	if (m_FlatScene)
	{
		// Execute a global (flat-scene) sort
		if (m_EnableSorting & m_NeedSort)
			Sort();

		// Flat (ignoring child/parent relationships) drawing
		FusionNode::ChildNodeList::iterator it = m_SceneNodes.begin();
		for (; it != m_SceneNodes.end(); ++it)
		{
			(*it)->_draw(false);
		}
	}
	else
	{
		// Scene graph drawing
		m_RootNode->_draw(true);
	}
}

void FusionScene::SetDrawingMode(bool flat)
{
	m_FlatScene = flat;
}

void FusionScene::SetEnableSorting(bool enable)
{
	m_EnableSorting = enable;
	if (enable)
		m_NeedSort = true;
}

void FusionScene::Sort()
{
	m_NeedSort = false;

	std::sort(m_SceneNodes.begin(), m_SceneNodes.end(), DepthIsLess);
}
