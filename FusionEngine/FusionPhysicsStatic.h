/*
  Copyright (c) 2006 FusionTeam

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_FusionPhysicsStatic
#define Header_FusionEngine_FusionPhysicsStatic

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "FusionBitmask.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * Abstract (Not ATM... maybe never) class, the basis for colliding but non-moving objects.
	 *
	 *
	 * \remarks
	 * With regurad to the state stored in this class:
	 * For FusionShips this should be initialisd by the ShipFactory when it creates
	 * a new ship. It should remain indipendant of the ClientEnvironment after that
	 * point - all modification to it can be done manually, rather than requiring it
	 * to know of ShipResource.
	 * <br>
	 * MCS - Just one other key thing to remember, FusionPhysicsStatic is brainless!
	 * This class just stores data, and keeps that data valid (i.e. modify the AABB
	 * to fit the bitmask if it rotates.)
	 *
	 * \todo AABB for FusionPhysicsStatic
	 *
	 * \see
	 * FusionPhysicsWorld | FusionFhysicsElipse.
	 */
	class FusionPhysicsStatic
	{
		friend class FusionPhysicsWorld;
	public:
		//! Constructor.
		/*!
		 * \param world
		 * The world in which this body resides.
		 */
		FusionPhysicsStatic(FusionPhysicsWorld *world);

		//! Virtual destructor.
		virtual ~FusionPhysicsStatic();

	public:
		//@{
		//! Properties.
		virtual void SetColBitmask(const FusionBitmask &bitmask);
		virtual void SetColAABB(float width, float height);
		//virtual SetColAABB(const CL_Rectf &bbox);
		virtual void SetColDist(float dist);
		//@}

		//@{
		//! Property retreival.
		virtual FusionBitmask GetColBitmask() const;
		//! Returns true if the given point is solid
		/*!
		 * Quick access to the bitmask function FusionBitmask#GetBit()
		 */
		virtual bool GetColPoint(CL_Point point) const;
		virtual CL_Rectf GetColAABB() const;
		virtual float GetColDist() const;
		//@}

		//@{
		//! Collision type properties.
		/*!
		 * I think these are self explanatory.
		 */
		void SetUsePixelCollisions(bool usePixel);
		void SetUseAABBCollisions(bool useAABB);
		void SetUseDistCollisions(bool useDist);
		//@}

		//@{
		/**
		 * Collision type property retrieval.
		 * I think these are self explanatory.
		 */
		bool GetUsePixelCollisions();
		bool GetUseAABBCollisions();
		bool GetUseDistCollisions();
		//@}

		//@{
		//! State retreival.
		virtual CL_Vector2 &GetPosition() const;
		virtual CL_Point &GetPositionPoint() const;
		//@}

		//@{
		//! For syncronising client-side only, shouldn't be called otherwise.
		virtual void _setPosition(const CL_Vector2 &position);
		//@}

	protected:
		//! Containing world
		FusionPhysicsWorld *m_World;

		//! The unique ID for the current object
		/*!
		 * Blah, I was going to write something here, but I forget.
		 */
		int m_Type;

		//! Bitmask
		FusionBitmask m_Bitmask;
		//! AABB
		CL_Rectf m_AABB;
		//! Dist
		float m_ColDist;

		//! I'm not sure if this is used...
		bool m_IsColliding;

		//! Bitmask collisions
		bool m_UsesPixel;
		//! Bounding Box collisions
		bool m_UsesAABB;
		//! Distance (circle) based collisions
		bool m_UsesDist;

		//@{
		//! "State" stuff.

		CL_Vector2 m_Position;
		//@}

	};

}

#endif