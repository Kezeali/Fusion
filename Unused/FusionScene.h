#ifndef Header_FusionEngine_FusionScene
#define Header_FusionEngine_FusionScene

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"


namespace FusionEngine
{
	/*!
	 * \example fusionscene_example.cpp
	 * Nodes should be added to the scene using code simmular to this
	 */

	/*!
	 * \brief
	 * Scene provides easy management for SceneNodes.
	 *
	 * Assuming you have a pointer to a FusionScene object called 'scene',
	 * you can create new scene nodes as follows:
	 * \code
	 * //...
	 * FusionNode* child = scene->CreateNode();
	 * scene->GetRootNode()->AddChild(child);
	 * //...
	 * \endcode
	 * or
	 * \code
	 * //...
	 * FusionNode *child = scene->GetRootNode()->CreateChildNode();
	 * //...
	 * \endcode
	 * 
	 * \sa
	 * FusionNode
	 */
	class IScene
	{
	public:
		//! Gives you access to the root node
		virtual ISceneNode *GetRootNode() const = 0;

		//! Adds the given node to the scene
		virtual void AddNode(const std::string& name, ISceneNode *node) = 0;
		//! Removes the given node from the scene
		virtual void RemoveNode(const std::string &name) = 0;
		//! Destroys a node cleanly
		virtual void DestroySceneNode(ISceneNode *node, bool destroy_children = true) = 0;

		/*!
		 * \brief
		 * Removes and deletes all nodes in the scene.
		 */
		virtual void CleanScene() = 0;

		//! Updates nodes
		virtual void Update(unsigned int split) = 0;

		//! Updates nodes and draws them.
		virtual void Draw() = 0;

		/*!
		 * \brief
		 * Sorts child nodes in the global (flat) list by their depth attribute.
		 *
		 * This sorts nodes so their position in the scene graph reflects their m_Depth
		 * attribute, in other words nodes with a lower m_Depth are drawn first.
		 * <br>
		 * This is only used when nodes aren't drawn via the graph (flat drawing).
		 */
		 virtual void Sort() = 0;
	};

	class Scene
	{	
	public:
		//! Constructor
		Scene();
		//! Destructor
		virtual ~Scene();

	public:
		//! A list of scene nodes
		typedef std::map<std::string, SceneNode*> NodeList;

		//! Gives you access to the root node
		virtual SceneNode *GetRootNode() const;

		//! Runs the node factory to give you a node
		virtual void AddNode(ISceneNode *node);
		virtual void RemoveNode(ISceneNode *node);
		//! Destroys a node cleanly
		virtual void DestroySceneNode(ISceneNode *node, bool destroy_children = true, bool destroy_drawables = true);

		/*!
		* \brief
		* Removes and deletes all nodes in the scene.
		*/
		virtual void CleanScene();

		//! Updates nodes
		virtual void Update(unsigned int split);

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
		//! Used for mem-management
		SceneNodeList m_Entities;
		//! The node to which all other nodes are children
		SimpleSceneNode *m_RootNode;

		//! If this is true, the scene ignores parent/child relationships
		bool m_FlatScene;

		//! The scene will only call Sort() if this is
		bool m_EnableSorting;
		//! Makes the graph resort if a nodes depth changes
		bool m_NeedSort;
	};


}

#endif
