
#include "FusionCommon.h"

/// STL

/// Fusion
#include "FusionNode.h"

/// Class
#include "FusionScene.h"

namespace FusionEngine
{

	Scene::Scene()
		: m_FlatScene(false),
		m_EnableSorting(false)
	{
		m_RootNode = new SceneNode();
		m_RootNode->_notifyRootNode();
	}

	Scene::~Scene()
	{
		CleanScene();
		delete m_RootNode;
	}

	void Scene::CleanScene()
	{
		EntityList::iterator it;
		for (it = m_Entities.begin(); it < m_Entities.end(); ++it)
		{
			delete (*it);
		}
		m_Entities.clear();
	}

	void Scene::AddNode(Entity *node)
	{
		// Remember this node
		m_Entities.push_back(node);
	}

	void Scene::DestroySceneNode(Entity *node, bool destroy_children, bool destroy_drawables)
	{
		if (node == m_RootNode)
			throw new CL_Error("Can't delete root node");

		EntityList::iterator it = m_Entities.begin();
		for (; it != m_Entities.end(); ++it)
		{
			if ((*it) == node)
				m_Entities.erase(it);
		}

		if (destroy_children)
			node->RemoveAndDestroyAllChildren();

		if (destroy_drawables)
			node->DetachAndDestroyAllDrawables();

		delete node;
	}

	void Scene::Update(unsigned int split)
	{
		FusionNode::ChildNodeList::iterator it = m_Entities.begin();
		for (; it != m_Entities.end(); ++it)
		{
			(*it)->UpdateDynamics(split);
		}
	}

	void Scene::Draw()
	{
		m_RootNode->_update(true, false);

		if (m_FlatScene)
		{
			// Execute a global (flat-scene) sort
			if (m_EnableSorting & m_NeedSort)
				Sort();

			// Flat (ignoring child/parent relationships) drawing
			FusionNode::ChildNodeList::iterator it = m_Entities.begin();
			for (; it != m_Entities.end(); ++it)
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

	void Scene::SetDrawingMode(bool flat)
	{
		m_FlatScene = flat;
	}

	void Scene::SetEnableSorting(bool enable)
	{
		m_EnableSorting = enable;
		if (enable)
			m_NeedSort = true;
	}

	void Scene::Sort()
	{
		m_NeedSort = false;

		std::sort(m_Entities.begin(), m_Entities.end(), FusionEngine::DepthIsLess);
	}

}