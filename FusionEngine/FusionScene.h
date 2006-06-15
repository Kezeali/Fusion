#ifndef Header_FusionEngine_FusionScene
#define Header_FusionEngine_FusionScene

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

// Fusion

namespace FusionEngine
{
	/*!
	 * \example fusionscene_example.cpp
	 * Nodes should be added to the scene using code simmular to the following
	 */

	/*!
	 * \brief
	 * FusionScene provides easy memory management for FusionNodes.
	 * 
	 * \sa
	 * FusionNode
	 */
	class FusionScene
	{
	public:
		typedef std::vector<FusionNode*> SceneNodeList;

		FusionScene();
		~FusionScene();

		virtual FusionNode *GetRootNode() const;

		virtual FusionNode *CreateNode();
		virtual void DestroySceneNode(FusionNode *node);

		/*!
		 * \brief
		 * Removes and deletes all nodes in the scene.
		 */
		virtual void CleanScene();

		//! Updates nodes and draws them.
		virtual void Draw();

	protected:
		//! All nodes created by this scene are listed here.
		SceneNodeList m_SceneNodes;
		FusionNode *m_RootNode;

		bool m_UseDrawOrder;
	};

}

#endif
