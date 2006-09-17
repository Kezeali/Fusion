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

#include "FusionBitmask.h"

using namespace FusionEngine;

FusionBitmask::FusionBitmask()
: m_BitmaskCreated(false)
{
}

FusionBitmask::FusionBitmask(const std::string &filename, int gridsize, unsigned int threshold)
: m_BitmaskCreated(false)
{
	SetFromImage(const std::string &filename, gridsize, threshold);
}

FusionBitmask::FusionBitmask(const CL_Surface &surface, int gridsize, unsigned int threshold)
: m_BitmaskCreated(false)
{
	SetFromSurface(const CL_Surface &filename, gridsize, threshold);
}


FusionBitmask::~FusionBitmask()
{
	bitmask_free(m_Bitmask);
}


FusionBitmask::SetFromImage(const std::string &filename, int gridsize, unsigned int threshold)
{
	CL_Surface source(filename);
	
	SetFromSurface(source, gridsize);
}

FusionBitmask::SetFromSurface(const CL_Surface &surface, int gridsize, unsigned int threshold)
{
	int mask_w = int(surface.get_width() / gridsize);
	int mask_h = int(surface.get_height() / gridsize);

	if (!m_BitmaskCreated)
	{
		m_Bitmask = bitmask_create(mask_w, mask_h);
		m_BitmaskCreated = true;
	}
	else
	{
		bitmask_clear(m_Bitmask);
	}

	for (int x = 0; x <= mask_w; x++)
		for (int y = 0; y <= mask_h; y++)
		{
			if (surface.get_pixeldata().get_pixel(x, y).get_alpha() <= threshold)
				bitmask_setbit(m_Bitmask, x, y);
		}
}

bool FusionBitmask::GetBit(const CL_Point &point) const
{
	assert(m_BitmaskCreated);

	if (point.x > m_MaskWidth) | (point.y > m_MaskHeight) // Out of range!
		return false;

	return bool(bitmask_getbit(m_Bitmask, point.x, point.y));
}

bool FusionBitmask::Overlap(const FusionEngine::FusionBitmask &other, const CL_Point &offset)
{
	assert(m_BitmaskCreated);
	return bool(bitmask_overlap(m_Bitmask, other.m_Bitmask, offset.x, offset.y));
}

const CL_Point &FusionBitmask::OverlapPoint(const FusionEngine::FusionBitmask &other, const CL_Point &offset)
{
	assert(m_BitmaskCreated);
	int x, y;
	x = y = -1;

	bitmask_overlap_pos(m_Bitmask, other.m_Bitmask, offset.x, offset.y, x, y);
	return CL_Point(x, y);
}

const CL_Vector2 &FusionBitmask::CalcCollisionNormal(const FusionEngine::FusionBitmask &other, const CL_Point &offset)
{
	int dx = bitmask_overlap_area(m_Bitmask, other.m_Bitmask, offset.x+1, offset.y) - 
         bitmask_overlap_area(m_Bitmask, other.m_Bitmask, offset.x-1, offset.y);
	int dy = bitmask_overlap_area(m_Bitmask, other.m_Bitmask, offset.x, offset.y+1) - 
         bitmask_overlap_area(m_Bitmask, other.m_Bitmask, offset.x, offset.y-1);

	return CL_Vector2(dx, dy);
}