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
	 * Nodes should be added to the scene using code simmular to this
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
		//! Constructor
		FusionScene();
		//! Destructor
		~FusionScene();

	public:
		//! A list of scene nodes
		typedef std::vector<FusionNode*> SceneNodeList;

		//! Gives you access to the root node
		virtual FusionNode *GetRootNode() const;

		//! Runs the node factory to give you a node
		virtual FusionNode *CreateNode();
		//! Destroys a node cleanly
		virtual void DestroySceneNode(FusionNode *node, bool destroy_children = true, bool destroy_drawables = true);

		/*!
		 * \brief
		 * Removes and deletes all nodes in the scene.
		 */
		virtual void CleanScene();

		//! Updates dynamic drawables
		virtual void UpdateDynamics(unsigned int split);

		//! Updates nodes and draws them.
		virtual void Draw();

		//! Sets the drawing mode
		/*!
		 * True for flat (ignores child/parent ordering)<br>
		 * False for graph (cascade drawing from the root node)
		 */
		virtual void SetDrawingMode(bool flat);

		//! Use this to enable or disable global (flat) sorting
		/*!
		 * Flat sorting should only be used if you aren't drawing using the
		 * scene graph (i.e. you are using flat drawing)
		 */
		virtual void SetEnableSorting(bool enable);

		//! Allows nodes to request a global (flat) sort when their depth changes
		virtual void _requestSort();

		/*!
		 * \brief
		 * Sorts child nodes in the global (flat) list by their depth attribute.
		 *
		 * This sorts nodes so their position in the scene graph reflects their m_Depth
		 * attribute, in other words nodes with a lower m_Depth are drawn first.
		 * <br>
		 * This is only used when nodes aren't drawn via the graph (flat drawing).
		 */
		 virtual void Sort();

	protected:
		//! All nodes created by this scene are listed here.
		SceneNodeList m_SceneNodes;
		//! The node to which all other nodes are children
		FusionNode *m_RootNode;

		//! If this is true, the scene ignores parent/child relationships
		bool m_FlatScene;

		//! The scene will only call Sort() if this is
		bool m_EnableSorting;
		//! Makes the graph resort if a nodes depth changes
		bool m_NeedSort;
	};

}

#endif
