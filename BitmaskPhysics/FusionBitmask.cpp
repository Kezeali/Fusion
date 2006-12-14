
#include "FusionBitmask.h"

namespace FusionEngine
{

	///////////////////////
	// Stream input/output
	std::istream &operator>> ( std::istream &is, FusionBitmask *bitmask )
	{
		//std::string str;
		//is >> str;
		//bitmask.SetFromStream(str);
		//bitmask_t *m = bitmask->m_Bitmask;

		// Read the dimensions
		char d[sizeof(BITW)];
		is.getline(&d[0], sizeof(int), '|');
		bitmask->m_MaskWidth = atoi(d);

		is.getline(&d[0], sizeof(int), '|');
		bitmask->m_MaskHeight = atoi(d);

		// Allocate the memory
		if (!bitmask->m_Bitmask)
			bitmask->m_Bitmask = bitmask_create(bitmask->m_MaskWidth, bitmask->m_MaskHeight);
		else
		{
			bitmask_free(bitmask->m_Bitmask);
			bitmask->m_Bitmask = bitmask_create(bitmask->m_MaskWidth, bitmask->m_MaskHeight);
		}

		// Read the bits
		for (int y = 0; y < bitmask->m_MaskHeight; y++)
		{
			for (int x = 0; x < bitmask->m_MaskWidth; x++)
			{
				is.getline(&d[0], sizeof(int), '|');
				int bit = atoi(d);
				if (bit == 1)
					bitmask_setbit(bitmask->m_Bitmask, x, y);
					//bitmask->m_Bitmask->bits[x/BITW_LEN*bitmask->m_Bitmask->h + y] = bit;
			}
		}

		return is;
	}

	std::ostream &operator<< ( std::ostream &os, const FusionBitmask *bitmask )
	{
		//bitmask_t *m = bitmask->m_Bitmask;

		os << bitmask->m_Bitmask->w;
		os << '|';
		os << bitmask->m_Bitmask->h;
		os << '|';

		for (int y = 0; y < bitmask->m_MaskHeight; y++)
		{
			for (int x = 0; x < bitmask->m_MaskWidth; x++)
			{
				os << bitmask_getbit(bitmask->m_Bitmask, x, y);
				os << '|';
			}
		}

		//os << bitmask.ToStream();
		return os;
	}

	///////////
	// Bitmask
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
		for (int y = 0; y < m_MaskHeight; y++)
			for (int x = 0; x < m_MaskWidth; x++)
			{
				if (bitmask_getbit(m_Bitmask, x, y))
				{
					CL_Pointf point(x*m_PPB + x_offset, y*m_PPB + y_offset);
					CL_Rectf rect(point, CL_Sizef(m_PPB, m_PPB));

					CL_Display::draw_rect(rect, CL_Color::azure);
				}
			}
	}


	void FusionBitmask::ToStream(std::ostream &os) const
	{
		os << m_Bitmask->w;
		os << '|';
		os << m_Bitmask->h;
		os << '|';

		os << m_PPB;
		os << '|';

		for (int y = 0; y < m_MaskHeight; y++)
		{
			for (int x = 0; x < m_MaskWidth; x++)
			{
				os << bitmask_getbit(m_Bitmask, x, y);
				os << '|';
			}
		}

		//os << bitmask.ToStream();
		//return os;
	}

	void FusionBitmask::SetFromStream(std::istream &is)
	{
		// Read the dimensions
		char d[sizeof(BITW)];
		is.getline(&d[0], sizeof(int), '|');
		m_MaskWidth = atoi(d);

		is.getline(&d[0], sizeof(int), '|');
		m_MaskHeight = atoi(d);

		is.getline(&d[0], sizeof(int), '|');
		m_PPB = atoi(d);

		m_PPBInverse = 1.0f/(float)m_PPB;

		// Allocate the memory
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(m_MaskWidth, m_MaskHeight);
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(m_MaskWidth, m_MaskHeight);
		}

		// Read the bits
		for (int y = 0; y < m_MaskHeight; y++)
		{
			for (int x = 0; x < m_MaskWidth; x++)
			{
				is.getline(&d[0], sizeof(int), '|');
				int bit = atoi(d);
				if (bit == 1)
					bitmask_setbit(m_Bitmask, x, y);
			}
		}

		//return is;
	}


	void FusionBitmask::SetFromRadius(float radius, int gridsize)
	{
		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

		float mask_radius = radius * m_PPBInverse;
		int mask_width = int( mask_radius * 2 + 0.5f );


		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
		{
			m_Bitmask = bitmask_create(mask_width+1, mask_width+1);
		}
		else if(mask_width == m_MaskWidth)
		{
				bitmask_clear(m_Bitmask);
		}
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(mask_width+1, mask_width+1);
		}

		m_MaskWidth = mask_width;
		m_MaskHeight = mask_width;

		// Build the mask
		CL_Point middle((int)mask_radius, (int)mask_radius);
		CL_Point bit(0, (int)mask_radius);
		
		for (float i = 1.0f; i < mask_radius; i++)
		{
			for (float a = 0.0f; a < 360.0f; a++)
			{
				bit.x = int( sin(a) * i + mask_radius);
				bit.y = int( cos(a) * i + mask_radius);
				fe_clamp(bit.x, 0, mask_width);
				fe_clamp(bit.y, 0, mask_width);

				bitmask_setbit(m_Bitmask, bit.x, bit.y);
			}
		}
	}

	void FusionBitmask::SetFromDimensions(const CL_Size &dimensions, int gridsize)
	{
		int mask_w = int(dimensions.width / gridsize);
		int mask_h = int(dimensions.height / gridsize);

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
		{
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}
		else if (mask_w == m_MaskWidth && mask_h == m_MaskHeight)
		{
			bitmask_clear(m_Bitmask);
		}
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}

		m_MaskWidth = mask_w;
		m_MaskHeight = mask_h;
		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

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

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
		{
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}
		else if (mask_w == m_MaskWidth && mask_h == m_MaskHeight)
		{
			bitmask_clear(m_Bitmask);
		}
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}

		m_MaskWidth = mask_w;
		m_MaskHeight = mask_h;
		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

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

	void FusionBitmask::Draw(const FusionBitmask *other, const CL_Point &offset, bool auto_scale)
	{
		assert(m_Bitmask);

		int scaled_x = offset.x;
		int scaled_y = offset.y;

		if (auto_scale)
		{
			scaled_x = int( offset.x * m_PPBInverse );
			scaled_y = int( offset.y * m_PPBInverse );
		}

		// Invert the offset, and swap the reference mask/offsetted mask, where necessary
		//  X and Y are negative
		if ((scaled_x < 0) & (scaled_y < 0))
			bitmask_draw(other->m_Bitmask, m_Bitmask, -scaled_x, -scaled_y);
		//  X is negative
		else if (scaled_x < 0)
			bitmask_draw(other->m_Bitmask, m_Bitmask, -scaled_x, scaled_y);
		//  Y is negative
		else if (scaled_y < 0)
			bitmask_draw(other->m_Bitmask, m_Bitmask, scaled_x, -scaled_y);
		//  No need to invert
		else
			bitmask_draw(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y);
	}

	void FusionBitmask::Erase(const FusionBitmask *other, const CL_Point &offset, bool auto_scale)
	{
		assert(m_Bitmask);

		int scaled_x = offset.x;
		int scaled_y = offset.y;

		if (auto_scale)
		{
			scaled_x = int( offset.x * m_PPBInverse );
			scaled_y = int( offset.y * m_PPBInverse );
		}

		// Invert the offset, and swap the reference mask/offsetted mask, where necessary
		//  X and Y are negative
		if ((scaled_x < 0) & (scaled_y < 0))
			bitmask_erase(other->m_Bitmask, m_Bitmask, -scaled_x, -scaled_y);
		//  X is negative
		else if (scaled_x < 0)
			bitmask_erase(other->m_Bitmask, m_Bitmask, -scaled_x, scaled_y);
		//  Y is negative
		else if (scaled_y < 0)
			bitmask_erase(other->m_Bitmask, m_Bitmask, scaled_x, -scaled_y);
		//  No need to invert
		else
			bitmask_erase(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y);

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

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		return (bitmask_overlap(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y)) ? true : false;
	}

	bool FusionBitmask::OverlapPoint(CL_Point *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		assert(m_Bitmask);
		int x, y;
		x = y = 0;

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		if (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, &x, &y))
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

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		return (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, x_out, y_out) == 1) ? true : false;
	}

	void FusionBitmask::CalcCollisionNormal(CL_Vector2 *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		int dx = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x+1, scaled_y) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x-1, scaled_y);
		int dy = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y+1) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y-1);

		memcpy(output, &CL_Vector2(dx, dy), sizeof(CL_Vector2));
	}

}
