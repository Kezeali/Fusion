#ifndef Header_FusionEngine_FusionDrawableObject
#define Header_FusionEngine_FusionDrawableObject

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// STL


namespace FusionEngine
{

	/*!
	 * \brief
	 * Abstract base for objects which need to track a node.
	 */
	class FusionDrawable
	{
	public:
		//! Constructor
		FusionDrawable();
		//! Destructor
		virtual ~FusionDrawable();

		/*!
		 * \brief
		 * Updates the drawable
		 *
		 * The implementation of this function should update anything that
		 * needs to change over time.
		 */
		virtual void Update(unsigned int split) {}

		/*!
		 * \brief
		 * Draws the drawable.
		 *
		 * The implementation of this function should draw something.
		 */
		virtual void Draw() =0;

		//! Returns the FusionNode to which this object is attached.
		FusionNode *GetParentSceneNode() const;

		//! Internal method called to notify the object that it has been attached to a node.
		void _notifyAttached(FusionNode* parent, bool dynamic = false);

		// Returns true if this object is attached to a SceneNode.
		bool IsAttached() const;

		/*!
		 * Returns true if this object is attached to a SceneNode,
		 * and this SceneNode is currently in an active part of the
		 * scene graph.
		 */
		bool IsInScene() const;

		//! Returns true if this object uses the update functionality
		bool IsDynamic() const;

	protected:
		//! The node which controls this objects position.
		FusionNode *m_ParentNode;

		//! Will be set to true by the node if this is attached as a dynamic drawable
		bool m_Dynamic;

	};

}

#endif
