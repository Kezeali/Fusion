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

	m_Bitmask = bitmask_create(mask_w, mask_h);

	for (int x = 0; x <= mask_w; x++)
		for (int y = 0; y <= mask_h; y++)
		{
			if (surface.get_pixeldata().get_pixel(x, y).get_alpha() <= threshold)
				bitmask_setbit(m_Bitmask, x, y);
		}
}
