#ifndef Header_FusionEngine_FusionDrawableObject
#define Header_FusionEngine_FusionDrawableObject

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

// STL


namespace FusionEngine
{

	/*!
	 * \brief
	 * Base class for objects controlled by a node.
	 *
	 * The main use of this class is the Draw() method, which is called by the scene passing
	 * draw position and facing. A later implementation may have the scene setting some
	 * position / facing attributes, to act as a cache or something and therefore allow for
	 * drawing / updates out of frame (ie. while the scene isn't drawing.)
	 */
	class FusionDrawable
	{
	public:
		FusionDrawable();
		virtual ~FusionDrawable();

		/*!
		 * \brief
		 * Draws the drawable.
		 *
		 * The implementation of this function should draw something at the given position and
		 * rotation. It is the implementors responsibility to decide what is done with the
		 * parameters.
		 *
		 * \param position
		 * The absolute position of the attached node.
		 *
		 * \param facing
		 * The absolute rotation of the attached node.
		 */
		virtual void Draw();

		//! Returns the FusionNode to which this object is attached.
		virtual FusionNode *GetParentSceneNode() const;

		//! Internal method called to notify the object that it has been attached to a node.
		virtual void _notifyAttached(FusionNode* parent);

		// Returns true if this object is attached to a SceneNode or TagPoint.
		virtual bool IsAttached() const;

		/*!
		 * Returns true if this object is attached to a SceneNode or TagPoint, 
		 * and this SceneNode / TagPoint is currently in an active part of the
		 * scene graph.
		 */
		virtual bool IsInScene() const;

	protected:
		//! The node which controls this objects position.
		FusionNode *m_ParentNode;

	};

}

#endif