
#include "FusionBitmask.h"

namespace FusionEngine
{

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


	int FusionBitmask::Save(void *buffer, int len)
	{
		CL_OutputSource_Memory source;

		// Mark this as bitmask data
		source.write_uint32(g_BitmaskCacheFiletype);
		source.write_uint8(g_BitmaskCacheVersion);

		// Dimensions and scale
		source.write_int32(m_Bitmask->w);
		source.write_int32(m_Bitmask->h);
		source.write_int32(m_PPB);

		// Write bitmask data
		for (int y = 0; y < m_Bitmask->w; y++)
		{
			for (int x = 0; x < m_Bitmask->h; x++)
			{
				source.write_bool8( bitmask_getbit(m_Bitmask, x, y) ? true : false );
			}
		}

		// Shove everything into the buffer
		if (source.size() > len)
			return (-1);

		memcpy(buffer, source.get_data().c_str(), source.size());

		return source.size();
	}

	bool FusionBitmask::Load(void *buffer, int len)
	{
		CL_InputSource_Memory source(buffer, len);

		// Make sure the data provided is valid
		int type = source.read_uint32();
		int version = source.read_uint8();

		if( type != g_BitmaskCacheFiletype )
			throw CL_Error("Data received is not bitmask data.");
		if( version != g_BitmaskCacheVersion )
			throw CL_Error( CL_String::format("Bitmask version %1 is unsupported. Supported version: %2.", version, g_BitmaskCacheVersion) );


		// Dimensions and scale
		int mask_w = source.read_int32();
		int mask_h = source.read_int32();
		m_PPB = source.read_int32();

		m_PPBInverse = 1.0f/m_PPB;

		
		// Allocate memory
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(mask_w, mask_h);
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}

		// Read the bitmask data
		for (int y = 0; y < mask_h; y++)
		{
			for (int x = 0; x < mask_w; x++)
			{
				if (source.read_bool8())
					bitmask_setbit(m_Bitmask, x, y);
			}
		}

		return true;
	}

	int FusionBitmask::Save(const std::string &name, CL_OutputSource *source)
	{
		// Mark this as bitmask data
		source->write_uint32(g_BitmaskCacheFiletype);
		source->write_uint8(g_BitmaskCacheVersion);

		// Dimensions and scale
		source->write_int32(m_Bitmask->w);
		source->write_int32(m_Bitmask->h);
		source->write_int32(m_PPB);

		// Write bitmask data
		for (int y = 0; y < m_Bitmask->h; y++)
		{
			for (int x = 0; x < m_Bitmask->w; x++)
			{
				source->write_int32( bitmask_getbit(m_Bitmask, x, y) );
				//source->write_bool8( bitmask_getbit(m_Bitmask, x, y) ? true : false );
			}
		}

		return source->size();
	}

	int FusionBitmask::Save(const std::string &name, CL_OutputSourceProvider *provider)
	{
		CL_OutputSource *source = provider->open_source(name);

		if (source == NULL)
			return (-1);

		return Save(name, source);
	}

	bool FusionBitmask::Load(const std::string &name, CL_InputSourceProvider *provider)
	{
		CL_InputSource *source;

		try
		{
			source = provider->open_source(name);
		}
		catch (CL_Error e)
		{
			//Console::getSingleton().Add(e.message);
			return false;
		}

		// Make sure the data provided is valid
		int type = source->read_uint32();
		int version = source->read_uint8();

		if( type != g_BitmaskCacheFiletype )
			throw CL_Error("Data received is not bitmask data.");
		if( version != g_BitmaskCacheVersion )
			throw CL_Error( CL_String::format("Bitmask version %1 is unsupported. Supported version: %2.", version, g_BitmaskCacheVersion) );


		// Dimensions and scale
		int mask_w = source->read_int32();
		int mask_h = source->read_int32();
		m_PPB = source->read_int32();

		m_PPBInverse = 1.0f/m_PPB;


		// Allocate memory
		if (!m_Bitmask)
			m_Bitmask = bitmask_create(mask_w, mask_h);
		else
		{
			bitmask_free(m_Bitmask);
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}

		// Read the bitmask data
		for (int y = 0; y < mask_h; y++)
		{
			for (int x = 0; x < mask_w; x++)
			{
				//if (source->read_bool8())
				if ( source->read_int32() )
					bitmask_setbit(m_Bitmask, x, y);
			}
		}

		return true;
	}


	void FusionBitmask::DisplayBits(int x_offset, int y_offset)
	{
		for (int y = 0; y < m_Bitmask->h; y++)
			for (int x = 0; x < m_Bitmask->w; x++)
			{
				if (bitmask_getbit(m_Bitmask, x, y))
				{
					//CL_Pointf point(x*m_PPB + x_offset, y*m_PPB + y_offset);
					//CL_Rectf rect(point, CL_Sizef(m_PPB, m_PPB));

					//CL_Display::draw_rect(rect, CL_Color::azure);
					CL_Display::draw_pixel(x*m_PPB + x_offset, y*m_PPB + y_offset, CL_Color::azure);
				}
			}
	}


	//void FusionBitmask::ToStream(std::ostream &os) const
	//{
	//	os << g_BitmaskCacheVersion;
	//	os << '|';
	//	os << m_Bitmask->w;
	//	os << '|';
	//	os << m_Bitmask->h;
	//	os << '|';

	//	os << m_PPB;
	//	os << '|';

	//	for (int y = 0; y < m_MaskHeight; y++)
	//	{
	//		for (int x = 0; x < m_MaskWidth; x++)
	//		{
	//			os << bitmask_getbit(m_Bitmask, x, y);
	//		}
	//	}

	//}

	//bool FusionBitmask::SetFromStream(std::istream &is)
	//{
	//	char d[sizeof(int)];
	//	// Make sure it has the right version
	//	is.getline(&d[0], sizeof(int), '|');
	//	if (atoi(d) != g_BitmaskCacheVersion)
	//		return false;

	//	// Read the dimensions
	//	is.getline(&d[0], sizeof(int), '|');
	//	m_MaskWidth = atoi(d);

	//	is.getline(&d[0], sizeof(int), '|');
	//	m_MaskHeight = atoi(d);

	//	is.getline(&d[0], sizeof(int), '|');
	//	m_PPB = atoi(d);

	//	m_PPBInverse = 1.0f/(float)m_PPB;

	//	// Allocate the memory
	//	if (!m_Bitmask)
	//		m_Bitmask = bitmask_create(m_MaskWidth, m_MaskHeight);
	//	else
	//	{
	//		bitmask_free(m_Bitmask);
	//		m_Bitmask = bitmask_create(m_MaskWidth, m_MaskHeight);
	//	}

	//	// Read the bits
	//	for (int y = 0; y < m_MaskHeight; y++)
	//	{
	//		for (int x = 0; x < m_MaskWidth; x++)
	//		{
	//			d[0] = is.get();
	//			int bit = atoi(d);
	//			if (bit == 1)
	//				bitmask_setbit(m_Bitmask, x, y);
	//		}
	//	}

	//	return true;
	//}


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
		else if(mask_width == m_Bitmask->w)
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
			for (float a = 0.0f; a < 2*PI; a+=0.1f)
			{
				bit.x = int( sinf(a) * i + mask_radius);
				bit.y = int( cosf(a) * i + mask_radius);
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
		else if (mask_w == m_Bitmask->w && mask_h == m_Bitmask->h)
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
		int sur_w = surface->get_width();
		int sur_h = surface->get_height();
		int mask_w = int(surface->get_width() / gridsize);
		int mask_h = int(surface->get_height() / gridsize);

		// Make sure the bitmask is only created once; if it's already created,
		// just clear the data.
		if (!m_Bitmask)
		{
			m_Bitmask = bitmask_create(mask_w, mask_h);
		}
		else if (mask_w == m_Bitmask->w && mask_h == m_Bitmask->h)
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

		CL_PixelBuffer pdat = surface->get_pixeldata();

		for (int x = 0; x < mask_w; x++)
			for (int y = 0; y < mask_h; y++)
			{
				int sur_x = fe_clamped<int>( int( x * gridsize ), 0, sur_w);
				int sur_y = fe_clamped<int>( int( y * gridsize ), 0, sur_h);
				if (pdat.get_pixel(sur_x, sur_y).get_alpha() >= threshold)
				{
					bitmask_setbit(m_Bitmask, x, y);
				}
			}
	}

	void FusionBitmask::Draw(const FusionBitmask *other, const CL_Point &offset, bool auto_scale)
	{
		cl_assert(m_Bitmask != 0);

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
		cl_assert(m_Bitmask != 0);

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
		cl_assert(m_Bitmask != 0);


		if ((point.x > m_Bitmask->w) | (point.y > m_Bitmask->h) | (point.x < 0) | (point.y < 0))
		{
			throw CL_Error("Bitmask: point out of range!");

			//return false;
		}

		return (bitmask_getbit(m_Bitmask, point.x, point.y) == 1) ? true : false;
	}

	bool FusionBitmask::Overlap(const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		cl_assert(m_Bitmask != 0);

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		return (bitmask_overlap(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y)) ? true : false;
	}

	bool FusionBitmask::OverlapPoint(CL_Point *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		cl_assert(m_Bitmask != 0);
		int x, y;
		x = y = 0;

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		if (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, &x, &y))
		{
			memcpy(output, &CL_Point(x, y), sizeof(Vector2));
			return true;
		}

		return false;
	}

	bool FusionBitmask::OverlapPoint(int *x_out, int *y_out, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		cl_assert(m_Bitmask != 0);
		cl_assert(x_out && y_out);

		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		return (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, x_out, y_out) == 1) ? true : false;
	}

	void FusionBitmask::CalcCollisionNormal(Vector2 *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	{
		int scaled_x = int( offset.x * m_PPBInverse );
		int scaled_y = int( offset.y * m_PPBInverse );

		int dx = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x+1, scaled_y) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x-1, scaled_y);
		int dy = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y+1) - 
			bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y-1);

		memcpy(output, &Vector2(dx, dy), sizeof(Vector2));
	}

}
