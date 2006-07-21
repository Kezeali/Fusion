/*
  Copyright (c) 2006 Elliot Hayward

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

#ifndef Header_FusionEngine_FusionBitmask
#define Header_FusionEngine_FusionBitmask

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionEngineCommon.h"

#include "bitmask.h"

namespace FusionEngine
{
	/*!
	 * \brief
	 * OO wrapper for Ulf Ekstrom's bitmask library.
	 */
	class FusionBitmask
	{
	public:
		/*!
		 * Creates a bitmask from an external image file.
		 *
		 * \param filename The image to base the mask on.
		 * \param gridsize The amount of pixels per bit in the bitmask.
		 * \param threshold The alpha value above which pixels are considered opaque.
		 */
		FusionBitmask(const std::string &filename, int gridsize, unsigned int threshold);
		/*!
		 * Creates a bitmask from an internal image.
		 *
		 * \param filename The image to base the mask on.
		 * \param gridsize The amount of pixels per bit in the bitmask.
		 * \param threshold The alpha value above which pixels are considered opaque.
		 */
		FusionBitmask(const CL_Surface &surface, int gridsize, unsigned int threshold);

		~FusionBitmask();

	public:

		/*!
		 * Creates a bitmask from an external image file.
		 *
		 * \param filename The image to base the mask on.
		 * \param gridsize The amount of pixels per bit in the bitmask.
		 * \param threshold The alpha value above which pixels are considered opaque.
		 */
		void SetFromImage(const std::string &filename, int gridsize, unsigned int threshold = 128);
		/*!
		 * Creates a bitmask from an internal image.
		 *
		 * \remarks
		 * MCS - I don't know if I should define the solid parts by alpha or colour...
		 * I think just checking alpha is faster so I'm using that; but if you want to
		 * change it, feel free :P
		 *
		 * \param filename The image to base the mask on.
		 * \param gridsize The amount of pixels per bit in the bitmask.
		 * \param threshold The alpha value above which pixels are considered opaque.
		 */
		void SetFromSurface(const CL_Surface &surface, int gridsize, unsigned int threshold = 128);

		//! Returns the solidality of the bit at the given point.
		bool GetBit(CL_Point point) const;

		//! Tells ya if there's a collision between this object and the one given
		bool Overlap(const FusionBitmask &other, const CL_Point &offset);
		//! Same as FusionBitmask#Overlap, but this tells you the first of intersection.
		/*!
		 * \return The first point at which an overlap was found.
		 * \retval (-1, -1) If no overlap was found.
		 */
		const CL_Point &OverlapPoint(const FusionBitmask &other, const CL_Point &offset);
		//! Calculates an <i>approximate</i> normal to the edge of this bitmask.
		/*!
		 * This calculates an approximate normal to the edge of this bitmask at a
		 * given point of collision; by checking the relative ammount of bits which
		 * collide with the bitmask 'other' on each side of said point. Obviously
		 * this method is very hap-hazard - e.g. in the case of there being no
		 * colliding bits on one side, you'll probably get a vector pointing straight
		 * through the mask :P
		 */
		const CL_Vector2 &CalcCollisionNormal(const FusionBitmask &other, const CL_Point &offset);

	protected:
		//! Bitmask n' stuff
		bitmask_t *m_Bitmask;

		//! We don't want to be using an uninitialised object now do we?
		bool m_BitmaskCreated;

		//! Width of the bitmask, to prevent checks going out of bounds.
		int m_MaskWidth;
		//! Height of the bitmask, to prevent checks going out of bounds.
		int m_MaskHeight;

	};

}

#endif
