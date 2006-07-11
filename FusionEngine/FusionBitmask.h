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
		 * \param filename The image to base the mask on.
		 * \param gridsize The amount of pixels per bit in the bitmask.
		 * \param threshold The alpha value above which pixels are considered opaque.
		 */
		void SetFromSurface(const CL_Surface &surface, int gridsize, unsigned int threshold = 128);

		bool GetBit(CL_Point point) const;

		bool Overlap(FusionBitmask other);

	protected:
		bitmask_t *m_Bitmask;

		bool m_BitmaskCreated;

	};

}

#endif
