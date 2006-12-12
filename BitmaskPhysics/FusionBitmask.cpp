
#include "FusionBitmask.h"

namespace FusionEngine
{

	FusionBitmask::FusionBitmask()
		: m_Bitmask(0),
		m_MaskWidth(0),
		m_MaskHeight(0)
	{
	}

	FusionBitmask::FusionBitmask(const std::string &filename, int gridsize, unsigned int threshold)
		: m_Bitmask(0)
	{
		SetFromImage(filename, gridsize, threshold);
	}

	FusionBitmask::FusionBitmask(const CL_Surface *surface, int gridsize, unsigned int threshold)
		: m_Bitmask(0)
	{
		SetFromSurface(surface, gridsize, threshold);
	}

	FusionBitmask::FusionBitmask(float radius, int gridsize)
		: m_Bitmask(0)
	{
		SetFromRadius(radius, gridsize);
	}

	FusionBitmask::FusionBitmask(const CL_Size &dimensions, int gridsize)
		: m_Bitmask(0)
	{
		SetFromDimensions(dimensions, gridsize);
	}


	FusionBitmask::~FusionBitmask()
	{
		if (m_Bitmask)
			bitmask_free(m_Bitmask);
	}


	void FusionBitmask::DisplayBits(int x_offset, int y_offset)
	{
		for (int x = 0; x < m_MaskWidth; x++)
			for (int y = 0; y < m_MaskHeight; y++)
			{
				if (bitmask_getbit(m_Bitmask, x, y))
				{
					CL_Pointf point(x*m_PPB + x_offset, y*m_PPB + y_offset);
					CL_Rectf rect(point, CL_Sizef(m_PPB, m_PPB));

					CL_Display::draw_rect(rect, CL_Color::azure);
				}
			}
	}


	void FusionBitmask::SetFromRadius(float radius, int gridsize)
	{
		int mask_radius = int(radius / gridsize);

		m_MaskWidth = mask_radius;
		m_MaskHeight = mask_radius;
		m_PPB = gridsize;

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(mask_radius, mask_radius);
		else
			bitmask_clear(m_Bitmask);

		// Build the mask
		CL_Point middle(int(mask_radius/2), int(mask_radius/2));
		CL_Point bit(0, int(mask_radius/2));
		
		for (int i = 0; i < mask_radius; i++)
		{
			bit.x = i;
			for (int a = 0; a < 360; a++)
			{
				bit = bit.rotate(middle, a);
				bit.x = fe_min(bit.x, mask_radius);
				bit.y = fe_min(bit.y, mask_radius);
				bitmask_setbit(m_Bitmask, bit.x, bit.y);
			}
		}
	}

	void FusionBitmask::SetFromDimensions(const CL_Size &dimensions, int gridsize)
	{
		int mask_w = int(dimensions.width / gridsize);
		int mask_h = int(dimensions.height / gridsize);

		m_MaskWidth = mask_w;
		m_MaskHeight = mask_h;
		m_PPB = gridsize;

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(mask_w, mask_h);
		else
			bitmask_clear(m_Bitmask);

		// Fill the bitmask with 1s
		bitmask_fill(m_Bitmask);

	}

	void FusionBitmask::SetFromImage(const std::string &filename, int gridsize, unsigned int threshold)
	{
		CL_Surface *source = &CL_Surface(filename);

		SetFromSurface(source, gridsize);
	}

	void FusionBitmask::SetFromSurface(const CL_Surface *surface, int gridsize, unsigned int threshold)
	{
		int mask_w = int(surface->get_width() / gridsize);
		int mask_h = int(surface->get_height() / gridsize);

		m_MaskWidth = mask_w;
		m_MaskHeight = mask_h;
		m_PPB = gridsize;

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(mask_w, mask_h);
		else
			bitmask_clear(m_Bitmask);

		for (int x = 0; x < mask_w; x++)
			for (int y = 0; y < mask_h; y++)
			{
				int sur_x = int( floor(float(x * gridsize)) );
				int sur_y = int( floor(float(y * gridsize)) );
				if (surface->get_pixeldata().get_pixel(sur_x, sur_y).get_alpha() >= threshold)
				{
					std::cout << "Compiling Bitmask: SetBit " << x << "," << y << std::endl;
					bitmask_setbit(m_Bitmask, x, y);
				}
			}
	}

	void FusionBitmask::Draw(const FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);

		// Invert the offset, and swap the reference mask/offsetted mask, where necessary
		//  X and Y are negative
		if ((offset.x < 0) & (offset.x < 0))
			bitmask_draw(other->m_Bitmask, m_Bitmask, -offset.x, -offset.y);
		//  X is negative
		else if (offset.x < 0)
			bitmask_draw(other->m_Bitmask, m_Bitmask, -offset.x, offset.y);
		//  Y is negative
		else if (offset.y < 0)
			bitmask_draw(other->m_Bitmask, m_Bitmask, offset.x, -offset.y);
		//  No need to invert
		else
			bitmask_draw(m_Bitmask, other->m_Bitmask, offset.x, offset.y);
	}

	void FusionBitmask::Erase(const FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);

		// Invert the offset, and swap the reference mask/offsetted mask, where necessary
		//  X and Y are negative
		if ((offset.x < 0) & (offset.x < 0))
			bitmask_erase(other->m_Bitmask, m_Bitmask, -offset.x, -offset.y);
		//  X is negative
		else if (offset.x < 0)
			bitmask_erase(other->m_Bitmask, m_Bitmask, -offset.x, offset.y);
		//  Y is negative
		else if (offset.y < 0)
			bitmask_erase(other->m_Bitmask, m_Bitmask, offset.x, -offset.y);
		//  No need to invert
		else
			bitmask_erase(m_Bitmask, other->m_Bitmask, offset.x, offset.y);

	}


	int FusionBitmask::GetPPB() const
	{
		return m_PPB;
	}


	bool FusionBitmask::GetBit(const CL_Point &point) const
	{
		assert(m_Bitmask);

		// We don't use assert here, because point may be passed from the 
		//  collision detection, which I wouldn't trust, even at release. ;)
		if ((point.x > m_MaskWidth) | (point.y > m_MaskHeight) | (point.x < 0) | (point.y < 0))
			return false; // Out of range!

		return (bitmask_getbit(m_Bitmask, point.x, point.y) == 1) ? true : false;
	}

	bool FusionBitmask::Overlap(const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);
		return (bitmask_overlap(m_Bitmask, other->m_Bitmask, offset.x, offset.y)) ? true : false;
	}

	bool FusionBitmask::OverlapPoint(CL_Point *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);
		int x, y;
		x = y = 0;

		if (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, offset.x, offset.y, &x, &y))
		{
			memcpy(output, &CL_Point(x, y), sizeof(CL_Vector2));
			return true;
		}

		return false;
	}

	bool FusionBitmask::OverlapPoint(int *x_out, int *y_out, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);
		assert(x_out && y_out);

		return (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, offset.x, offset.y, x_out, y_out) == 1) ? true : false;
	}

	void FusionBitmask::CalcCollisionNormal(CL_Vector2 *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		int scaled_x = int( offset.x / m_PPB );
		int scaled_y = int( offset.y / m_PPB );

		int dx = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x+1, scaled_y) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x-1, scaled_y);
		int dy = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y+1) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y-1);

		memcpy(output, &CL_Vector2(dx, dy), sizeof(CL_Vector2));
	}

}
