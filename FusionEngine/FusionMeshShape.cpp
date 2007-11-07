
#include "Common.h"

#include "FusionBitmask.h"

#include "FusionPhysicsBody.h"

namespace FusionEngine
{

	///////////
	// Bitmask
	Bitmask::Bitmask(cpSpace* space, PhysicsBody* body)
		: m_PPB(10),
		m_Width(0),
		m_Height(0)
	{
		m_Space = space;
		//m_Body = cpBodyNew(INFINITY, INFINITY);
		//m_Body->p.x = position.x;
		//m_Body->p.y = position.y;
		m_Body = body->GetChipBody();
		m_PhysBody = body;

		//m_Shapes = cpSpaceHashNew(m_PPB, 10000, &bbfunc);
		m_Shapes = NULL;
	}

	Bitmask::Bitmask(cpSpace* space, const std::string &filename, int gridsize, unsigned int threshold)
		: m_PPB(10),
		m_Width(0),
		m_Height(0)
	{
		m_Space = space;
		m_Body = cpBodyNew(INFINITY, INFINITY);

		m_Shapes = cpSpaceHashNew(m_PPB, 10000, &bbfunc);
		SetFromImage(filename, gridsize, threshold);
	}

	Bitmask::Bitmask(cpSpace* space, const CL_Surface *surface, int gridsize, unsigned int threshold)
		: m_PPB(10),
		m_Width(0),
		m_Height(0)
	{
		m_Space = space;
		m_Body = cpBodyNew(INFINITY, INFINITY);

		m_Shapes = cpSpaceHashNew(m_PPB, 10000, &bbfunc);
		SetFromSurface(surface, gridsize, threshold);
	}

	Bitmask::Bitmask(cpSpace* space, float radius, int gridsize)
		: m_PPB(10),
		m_Width(0),
		m_Height(0)
	{
		m_Space = space;
		m_Body = cpBodyNew(INFINITY, INFINITY);

		m_Shapes = cpSpaceHashNew(m_PPB, 10000, &bbfunc);
		SetFromRadius(radius, gridsize);
	}

	Bitmask::Bitmask(cpSpace* space, const CL_Size &dimensions, int gridsize)
		: m_PPB(10),
		m_Width(0),
		m_Height(0)
	{
		m_Space = space;
		m_Body = cpBodyNew(INFINITY, INFINITY);

		m_Shapes = cpSpaceHashNew(m_PPB, 10000, &bbfunc);
		SetFromDimensions(dimensions, gridsize);
	}


	Bitmask::~Bitmask()
	{
		if (m_Body)
			cpBodyFree(m_Body);
		if (m_Shapes)
			cpSpaceHashFree(m_Shapes);
	}


	int Bitmask::Save(void *buffer, int len)
	{
		CL_OutputSource_Memory source;

		// Mark this as bitmask data
		source.write_uint32(g_BitmaskCacheFiletype);
		source.write_uint8(g_BitmaskCacheVersion);

		// Dimensions and scale
		source.write_int32(m_Width);
		source.write_int32(m_Height);
		source.write_int32(m_PPB);

		// Write bitmask data
		for (int b = 0; b < m_Width*m_Height; b++)
		{
				source.write_bool8( m_Bits[b] );
		}

		// Shove everything into the buffer
		if (source.size() > len)
			return (-1);

		memcpy(buffer, source.get_data().c_str(), source.size());

		return source.size();
	}

	bool Bitmask::Load(void *buffer, int len)
	{
		CL_InputSource_Memory source(buffer, len);

		// Make sure the data provided is valid
		int type = source.read_uint32();
		int version = source.read_uint8();

		if( type != g_BitmaskCacheFiletype )
			FSN_EXCEPT(ExCode::IO, "Bitmask::Load", "Data received is not bitmask data.");
		if( version != g_BitmaskCacheVersion )
			FSN_EXCEPT(ExCode::FileType, "Bitmask::Load", CL_String::format("Bitmask version %1 is unsupported. Supported version: %2.", version, g_BitmaskCacheVersion) );


		// Dimensions and scale
		m_Width = source.read_int32();
		m_Height = source.read_int32();
		m_PPB = source.read_int32();

		m_PPBInverse = 1.0f/m_PPB;
		m_BitRadius = m_PPB * 0.7; // do we need to scale this, or can we just use PPB?

		
		m_Bits = boost::dynamic_bitset<>(m_Width*m_Height);

		int b = 0;
		// Read the bitmask data
		for (int y = 0; y < m_Height; y++)
		{
			for (int x = 0; x < m_Width; x++)
			{
				if (source.read_bool8())
				{
					m_Bits[b] = true;
					cpShape* shape = cpCircleShapeNew(m_Body, m_BitRadius, cpv(x*m_PPB, y*m_PPB));
					shape->e = 1.0; shape->u = 1.0;
					shape->collision_type = g_PhysBodyCpCollisionType;
					cpSpaceAddStaticShape(m_Space, shape);
				}
				b++;
			}
		}

		return true;
	}

	int Bitmask::Save(const std::string &name, CL_OutputSource *source)
	{
		// Mark this as bitmask data
		source->write_uint32(g_BitmaskCacheFiletype);
		source->write_uint8(g_BitmaskCacheVersion);

		// Dimensions and scale
		source->write_int32(m_Width);
		source->write_int32(m_Height);
		source->write_int32(m_PPB);

		// Write bitmask data
		for (int b = 0; b < m_Width*m_Height; b++)
		{
			source->write_int32( m_Bits[b] );
		}

		return source->size();
	}

	int Bitmask::Save(const std::string &name, CL_OutputSourceProvider *provider)
	{
		CL_OutputSource *source = provider->open_source(name);

		if (source == NULL)
			return (-1);

		bool saved = Save(name, source);

		source->close();

		return saved;
	}

	bool Bitmask::Load(const std::string &name, CL_InputSourceProvider *provider)
	{
		CL_InputSource *source;

		try
		{
			source = provider->open_source(name);
		}
		catch (CL_Error& e)
		{
			//Console::getSingleton().Add(e.message);
			return false;
		}

		// Make sure the data provided is valid
		int type = source->read_uint32();
		int version = source->read_uint8();

		if( type != g_BitmaskCacheFiletype )
			FSN_EXCEPT(ExCode::FileType, "Bitmask::Load", "Expected bitmask file; found some other shit.");
		if( version != g_BitmaskCacheVersion )
			FSN_EXCEPT(ExCode::FileType, "Bitmask::Load", 
			            CL_String::format("Expected Bitmask version %1, found version %2", g_BitmaskCacheVersion, version) );


		// Dimensions and scale
		m_Width = source->read_int32();
		m_Height = source->read_int32();
		m_PPB = source->read_int32();

		m_PPBInverse = 1.0f/m_PPB;
		m_BitRadius = m_PPB*0.7;

		
		m_Bits.clear();
		m_Bits.resize(m_Width*m_Height);

		// Read the bitmask data
		int b = 0;
		for (int y = 0; y < m_Height; y++)
		{
			for (int x = 0; x < m_Width; x++)
			{
				//if (source->read_bool8())
				if ( source->read_int32() )
				{
					m_Bits[b] = true;
					cpShape* shape = cpCircleShapeNew(m_Body, m_BitRadius, cpv(x*m_PPB, y*m_PPB));
					shape->e = 1.0; shape->u = 1.0;
					shape->collision_type = g_PhysBodyCpCollisionType;
					shape->data = m_PhysBody;
					cpSpaceAddStaticShape(m_Space, shape);
				}

				b++;
			}
		}

		source->close();

		cpSpaceRehashStatic(m_Space);

		return true;
	}


	void Bitmask::DisplayBits(int x_offset, int y_offset, CL_Color col)
	{
		for (int y = 0; y < m_Height; y++)
			for (int x = 0; x < m_Width; x++)
			{
				if (m_Bits[x+y*m_Width])
				{
					//CL_Pointf point(x*m_PPB + x_offset, y*m_PPB + y_offset);
					//CL_Rectf rect(point, CL_Sizef(m_PPB, m_PPB));

					//CL_Display::draw_rect(rect, col);
					CL_Display::draw_pixel(x*m_PPB + x_offset, y*m_PPB + y_offset, col);
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


	void Bitmask::SetFromRadius(float radius, int gridsize)
	{
		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

		float mask_radius = radius * m_PPBInverse;
		m_Width = m_Height = int( mask_radius * 2 ) + 1;

		m_BitRadius = m_PPB*0.7;
		
		m_Bits.clear();
		m_Bits.resize(m_Width*m_Height);

		// Build the mask
		int x = 0;
		int y = 0;

		for (float i = 1.0f; i < mask_radius; i++)
		{
			for (float a = 0.0f; a < 2*PI; a+=0.1f)
			{
				x = int( sinf(a) * i + mask_radius);
				y = int( cosf(a) * i + mask_radius);
				fe_clamp(x, 0, m_Width);
				fe_clamp(y, 0, m_Width);

				addBit(x, y);
			}
		}
	}

	void Bitmask::SetFromDimensions(const CL_Size &dimensions, int gridsize)
	{
		m_Width = int(dimensions.width / gridsize);
		m_Height = int(dimensions.height / gridsize);

		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

		m_BitRadius = m_PPB*0.67;
		
		m_Bits.clear();
		m_Bits.resize(m_Width*m_Height, true);
	}

	void Bitmask::SetFromImage(const std::string &filename, int gridsize, unsigned int threshold)
	{
		CL_Surface *source = &CL_Surface(filename);

		SetFromSurface(source, gridsize);
	}

	void Bitmask::SetFromSurface(const CL_Surface *surface, int gridsize, unsigned int threshold)
	{
		int sur_w = surface->get_width();
		int sur_h = surface->get_height();
		m_Width = int(surface->get_width() / gridsize);
		m_Height = int(surface->get_height() / gridsize);

		m_PPB = gridsize;
		m_PPBInverse = 1.0f/(float)gridsize;

		m_BitRadius = m_PPB*0.7;


		m_Bits.clear();
		m_Bits.resize(m_Width*m_Height);

		CL_PixelBuffer pdat = surface->get_pixeldata();

		for (int x = 0; x < m_Width; x++)
			for (int y = 0; y < m_Height; y++)
			{
				int sur_x = fe_clamped<int>( int( x * gridsize ), 0, sur_w);
				int sur_y = fe_clamped<int>( int( y * gridsize ), 0, sur_h);
				if (pdat.get_pixel(sur_x, sur_y).get_alpha() >= threshold)
				{
					addBit(x, y);
				}
			}
	}

	void Bitmask::Draw(const Shape *other, const CL_Point &offset, bool auto_scale)
	{
	}
	//{
	//	cl_assert(m_Bitmask != 0);

	//	int scaled_x = offset.x;
	//	int scaled_y = offset.y;

	//	if (auto_scale)
	//	{
	//		scaled_x = int( offset.x * m_PPBInverse );
	//		scaled_y = int( offset.y * m_PPBInverse );
	//	}

	//	// Invert the offset, and swap the reference mask/offsetted mask, where necessary
	//	//  X and Y are negative
	//	if ((scaled_x < 0) & (scaled_y < 0))
	//		bitmask_draw(other->m_Bitmask, m_Bitmask, -scaled_x, -scaled_y);
	//	//  X is negative
	//	else if (scaled_x < 0)
	//		bitmask_draw(other->m_Bitmask, m_Bitmask, -scaled_x, scaled_y);
	//	//  Y is negative
	//	else if (scaled_y < 0)
	//		bitmask_draw(other->m_Bitmask, m_Bitmask, scaled_x, -scaled_y);
	//	//  No need to invert
	//	else
	//		bitmask_draw(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y);
	//}

	void Bitmask::Erase(const Shape *eraser, const Vector2 &offset)
	{
		cpShape* shape = eraser->GetShape();
		if (offset != Vector2::ZERO)
		{
			shape->body->p = cpv(offset.x, offset.y);
			cpShapeCacheBB(shape);
		}
		//cpSpaceHashQuery(m_Space->staticShapes, shape, shape->bb, &eraseFunc, &m_RemoveQueue);
		EraseData data;
		data.space = m_Space;
		data.list = &m_RemoveQueue;
		data.other = shape;
		cpSpaceHashEach(m_Space->staticShapes, &eraseFunc, &data);
	}

	void Bitmask::Update()
	{
		for (ShapeList::iterator it = m_RemoveQueue.begin(), end = m_RemoveQueue.end(); it != end; ++it)
			cpSpaceRemoveStaticShape(m_Space, (*it));
		m_RemoveQueue.clear();
	}

	//{
	//	cl_assert(m_Bitmask != 0);

	//	int scaled_x = offset.x;
	//	int scaled_y = offset.y;

	//	if (auto_scale)
	//	{
	//		scaled_x = int( offset.x * m_PPBInverse );
	//		scaled_y = int( offset.y * m_PPBInverse );
	//	}

	//	// Invert the offset, and swap the reference mask/offsetted mask, where necessary
	//	//  X and Y are negative
	//	if ((scaled_x < 0) & (scaled_y < 0))
	//		bitmask_erase(other->m_Bitmask, m_Bitmask, -scaled_x, -scaled_y);
	//	//  X is negative
	//	else if (scaled_x < 0)
	//		bitmask_erase(other->m_Bitmask, m_Bitmask, -scaled_x, scaled_y);
	//	//  Y is negative
	//	else if (scaled_y < 0)
	//		bitmask_erase(other->m_Bitmask, m_Bitmask, scaled_x, -scaled_y);
	//	//  No need to invert
	//	else
	//		bitmask_erase(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y);

	//}


	int Bitmask::GetPPB() const
	{
		return m_PPB;
	}


	bool Bitmask::GetBit(const CL_Point &point) const
	{
		return true;
	}
	//{
	//	cl_assert(m_Bitmask != 0);

	//	if ((point.x > m_Bitmask->w) | (point.y > m_Bitmask->h) | (point.x < 0) | (point.y < 0))
	//	{
	//		FSN_EXCEPT(ExCode::Base, "Bitmask::GetBit", "point");
	//	}

	//	return (bool)bitmask_getbit(m_Bitmask, point.x, point.y);
	//}

	bool Bitmask::GetBit(int x, int y) const
	{
		return true;
	}
	//{
	//	cl_assert(m_Bitmask != 0);

	//	if ((x > m_Bitmask->w) | (y > m_Bitmask->h) | (x < 0) | (y < 0))
	//	{
	//		FSN_EXCEPT(ExCode::Base, "Bitmask::GetBit", "x, y");
	//	}

	//	return (bool)m_Bits[x+y*m_Width];
	//}

	//bool Bitmask::Overlap(const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	//{
	//	cl_assert(m_Bitmask != 0);

	//	int scaled_x = int( offset.x * m_PPBInverse );
	//	int scaled_y = int( offset.y * m_PPBInverse );

	//	return (bitmask_overlap(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y)) ? true : false;
	//}

	//bool FusionBitmask::OverlapPoint(CL_Point *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	//{
	//	cl_assert(m_Bitmask != 0);
	//	int x, y;
	//	x = y = 0;

	//	int scaled_x = int( offset.x * m_PPBInverse );
	//	int scaled_y = int( offset.y * m_PPBInverse );

	//	if (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, &x, &y))
	//	{
	//		memcpy(output, &CL_Point(x, y), sizeof(Vector2));
	//		return true;
	//	}

	//	return false;
	//}

	//bool FusionBitmask::OverlapPoint(int *x_out, int *y_out, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	//{
	//	cl_assert(m_Bitmask != 0);
	//	cl_assert(x_out && y_out);

	//	int scaled_x = int( offset.x * m_PPBInverse );
	//	int scaled_y = int( offset.y * m_PPBInverse );

	//	return (bitmask_overlap_pos(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y, x_out, y_out) == 1) ? true : false;
	//}

	//void FusionBitmask::CalcCollisionNormal(Vector2 *output, const FusionEngine::FusionBitmask *other, const CL_Point &offset)
	//{
	//	int scaled_x = int( offset.x * m_PPBInverse );
	//	int scaled_y = int( offset.y * m_PPBInverse );

	//	int dx = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x+1, scaled_y) - 
	//		bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x-1, scaled_y);
	//	int dy = bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y+1) - 
	//		bitmask_overlap_area(m_Bitmask, other->m_Bitmask, scaled_x, scaled_y-1);

	//	memcpy(output, &Vector2(dx, dy), sizeof(Vector2));
	//}

}
