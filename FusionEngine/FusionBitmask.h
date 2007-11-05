/*
Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_FusionEngine_Bitmask
#define Header_FusionEngine_Bitmask

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicsShape.h"

//#include "windows/config.h"
//#include <google/sparse_hash_map>
#include <boost/dynamic_bitset.hpp>
//#include "bitmask.h"


namespace FusionEngine
{

	//! Used to confirm that the file being read is a bitmask file.
	static const int g_BitmaskCacheFiletype = 0x17122006;
	//! The version of bitmask cache that can be read by this class
	static const unsigned char g_BitmaskCacheVersion = 3;

	/*!
	* \brief
	* Terrain physical shape
	*
	* Fakes the functionality of the Bitmask class from previous versions
	* of Fusion Physics, using chipmunk shapes
	*/
	class Bitmask
	{
	private:
		typedef std::set<cpShape*> ShapeList;
	public:
		//! Basic constructor.
		Bitmask(cpSpace* space, PhysicsBody* body);

		/*!
		* Creates a bitmask from an external bitmask file.
		*
		* \param filename The image to base the mask on.
		* \param gridsize The amount of pixels per bit in the bitmask.
		* \param threshold The alpha value above which pixels are considered opaque.
		*/
		Bitmask(cpSpace* space, const std::string &filename, int gridsize, unsigned int threshold);
		/*!
		* Creates a bitmask from an internal image.
		*
		* \param filename The image to base the mask on.
		* \param gridsize The amount of pixels per bit in the bitmask.
		* \param threshold The alpha value above which pixels are considered opaque.
		*/
		Bitmask(cpSpace* space, const CL_Surface *surface, int gridsize, unsigned int threshold);
		/*!
		* Creates a circular bitmask of the given radius.
		*
		* \param radius Radius, in pixels, of the bitmask.
		* \param gridsize The amount of pixels per bit in the bitmask.
		*/
		Bitmask(cpSpace* space, float radius, int gridsize);
		/*!
		* Creates a rectangular bitmask of the given dimensions.
		*
		* \param[in] dimensions
		* Size, in pixels, of the bitmask.
		* \param[in] gridsize
		* The amount of pixels per bit in the bitmask.
		*/
		Bitmask(cpSpace* space, const CL_Size &dimensions, int gridsize);

		//! Destructor.
		~Bitmask();

	public:

		//! Save to a buffer.
		/*!
		* \param[out] buffer
		* Pass a pointer here to the buffer into which data should be written.
		*
		* \param[in] len
		* The length of the buffer (i.e. the maximum amount of data to write to it)
		*
		* \returns
		* The actual amount of data written.
		*
		* \retval -1
		* If the write fails.
		*/
		int Save(void *buffer, int len);

		//! Load from a buffer
		/*!
		* \param[in] buffer
		* The buffer from which to read data.
		*
		* \param[in] len
		* The length of (buffer).
		*
		* \returns
		* True if the bitmask loaded successfully, false otherwise.
		*/
		bool Load(void *buffer, int len);

		//! Save to a device.
		/*!
		* \param[in] name
		* Name of the file to be created.
		*
		* \param[in] source
		* The output source to write to.
		*
		* \returns
		* The actual amount of data written.
		*
		* \retval -1
		* If the write fails.
		*/
		int Save(const std::string &name, CL_OutputSource *source);

		//! Save to a device.
		/*!
		* \param[in] name
		* Name of the file to be created.
		*
		* \param[in] provider
		* The output source provider to use when creating the file.
		*
		* \returns
		* The actual amount of data written.
		*
		* \retval -1
		* If the write fails.
		*/
		int Save(const std::string &name, CL_OutputSourceProvider *provider = 0);

		//! Load from a device.
		/*!
		* \param[in] name
		* The file to open for reading.
		*
		* \param[in] provider
		* The input source provider to use when opening the file.
		*
		* \returns
		* True if the bitmask loaded successfully, false otherwise.
		*/
		bool Load(const std::string &name, CL_InputSourceProvider *provider = 0);


		//! Debug only, to be removed
		void DisplayBits(int x_offset, int y_offset, CL_Color col = CL_Color::azure);


		//! [removed] Gets the current bits, so the can be saved or transfered
		void ToStream(std::ostream &os) const {};

		//! [removed] Sets the bitmask from the given cache
		bool SetFromStream(std::istream &is) { return false; }


		//! Creates a circular bitmask of the given radius.
		/*!
		* \param radius
		* Radius, in pixels, of the bitmask.
		* \param gridsize
		* The amount of pixels per bit in the bitmask.
		*/
		void SetFromRadius(float radius, int gridsize);

		/*!
		* Creates a rectangular bitmask of the given dimensions.
		*
		* \param[in] dimensions
		* Size, in pixels, of the bitmask.
		* \param[in] gridsize
		* The amount of pixels per bit in the bitmask.
		*/
		void SetFromDimensions(const CL_Size &dimensions, int gridsize);

		//! Creates a bitmask from an external image file.
		/*!
		* \param filename The image to base the mask on.
		* \param gridsize The amount of pixels per bit in the bitmask.
		* \param threshold The alpha value above which pixels are considered opaque.
		*/
		void SetFromImage(const std::string &filename, int gridsize, unsigned int threshold = 128);
		//! Creates a bitmask from an internal image.
		/*!
		* \remarks
		* Me - I don't know if I should define the solid parts by alpha or colour...
		* I think just checking alpha is faster so I'm using that; but if you want to
		* change it, feel free :P
		*
		* \param filename The image to base the mask on.
		* \param gridsize The amount of pixels per bit in the bitmask.
		* \param threshold The alpha value above which pixels are considered opaque.
		*/
		void SetFromSurface(const CL_Surface *surface, int gridsize, unsigned int threshold = 128);

		//! Adds the given bitmask to this one.
		/*!
		* Gets the points from this mask AND the points from the other mask.
		*
		* \param[in] other
		* The bitmask to add.
		* \param[in] offset
		* The offset from (0,0) on this bitmask.
		*/
		void Draw(const Shape *other, const CL_Point &offset, bool auto_scale = true);

		//! Removes the given bitmask to this one.
		/*!
		* Puts zeros on this bitmask where 'other' has ones.
		*
		* \param[in] other
		* The bitmask to remove.
		* \param[in] offset
		* The offset from (0,0) on this bitmask.
		*/
		void Erase(const Shape *other, const Vector2 &offset = Vector2());

		void Update();

		//! Returns the Pixels per Bit property of this bitmask.
		/*!
		* PPB aka gridsize.
		* \returns
		* The PPB (gridsize) which was used to create this bitmask.
		*/
		int GetPPB() const;

		//! Returns the solidality of the bit at the given point.
		/*!
		* \param point
		* A point giving the x and y co-ords to check
		*
		* \retval true If the bit is solid solid
		*/
		bool GetBit(const CL_Point &point) const;
		//! Returns the solidality of the bit at the given point.
		/*!
		* \param point
		* A point giving the x and y co-ords to check
		*
		* \retval true If the bit is solid solid
		*/
		bool GetBit(int x, int y) const;

		//! Tells ya if there's a collision between this object and the one given
		//bool Overlap(const Shape *other, const CL_Point &offset);
		//! Same as FusionBitmask#Overlap, but this tells you the first of intersection.
		/*!
		* \param[out] output
		* Will be set to the first point of overlap found.
		* \param[in] other
		* The bitmask to check against.
		* \param[in] offset
		* The distance between the the bitmasks.
		*
		* \retval true If a point was found - output will be set to this point.
		* \retval false If a point was not found - output will not be touched.
		*/
		//bool OverlapPoint(CL_Point *output, const FusionBitmask *other, const CL_Point &offset);
		//! Same as FusionBitmask#Overlap, but this tells you the first of intersection.
		/*!
		* \param[out] x_out
		* Will be set to the x param of the first point of overlap found.
		* \param[out] y_out
		* Will be set to the y param of the first point of overlap found.
		* \param[in] other
		* The bitmask to check against.
		* \param[in] offset
		* The distance between the the bitmasks.
		*
		* \retval true If a point was found - output will be set to this point.
		* \retval false If a point was not found - output will not be touched.
		*/
		//bool OverlapPoint(int *x_out, int *y_out, const FusionBitmask *other, const CL_Point &offset);
		//! Calculates an <i>approximate</i> normal to the edge of this bitmask.
		/*!
		* This calculates an approximate normal to the edge of this bitmask at a
		* given point of collision; by checking the relative ammount of bits which
		* collide with the bitmask 'other' on each side of said point. Obviously
		* this method is very hap-hazard.
		*
		* \param[out] output
		* Will be set to the collision normal vector.
		* \param[in] other
		* The bitmask to check against.
		* \param[in] offset
		* The distance between the the bitmasks.
		*/
		//void CalcCollisionNormal(Vector2 *output, const FusionBitmask *other, const CL_Point &offset);

		struct EraseData
		{
			cpShape *other;
			cpSpace *space;
			ShapeList *list;
		};

		//static int eraseFunc(void *p1, void *p2, void *data)
		static void eraseFunc(void *p1, void *p2)
		{
			cpShape *b = (cpShape *)p1;
			EraseData* d = (EraseData*)p2;
			ShapeList* removeList = d->list;
			cpShape* a = d->other;
			//cpShape *b = (cpShape *)p2;
			//Bitmask *terrain = (Bitmask*)p2;
			//cpSpace *space = terrain->getSpace();
			//cpBody *body = terrain->getBody();
			//ShapeList* removeList = (ShapeList*)data;

			// Reject any of the simple cases
			if(
				// BBoxes must overlap
				!cpBBintersects(a->bb, b->bb)
				// Don't collide shapes attached to the same body.
				|| a->body == b->body
				//) return 0;
				) return;


			cpShape *c_a = a;
			cpShape *c_b = b;
			// Shape 'a' should have the lower shape type. (required by cpCollideShapes() )
			if(a->type > b->type){
				c_a = b;
				c_b = a;
			}

			// Narrow-phase collision detection.
			cpContact *contacts = NULL;
			int numContacts = cpCollideShapes(c_a, c_b, &contacts);
			//if(!numContacts) return 0; // Shapes are not colliding.
			if (!numContacts) return;
			free(contacts);

			removeList->insert(b);
			//cpSpaceRemoveStaticShape(space, b);

			//return 0;
			return;
		}

	protected:
		//! Bitmask n' stuff
		//bitmask_t *m_Bitmask;
		boost::dynamic_bitset<> m_Bits;
		cpSpaceHash* m_Shapes;
		cpBody* m_Body;
		cpSpace* m_Space;
		PhysicsBody* m_PhysBody;
		ShapeList m_RemoveQueue;

		//! [depreciated] This isn't really necessary.
		bool m_BitmaskCreated;

		//! The gridsize used to create this bitmask.
		int m_PPB;
		//! Bits per Pixel, the inverted PPB.
		float m_PPBInverse;

		float m_BitRadius;

		//! Width
		int m_Width;
		//! Height
		int m_Height;

	protected:
		//! Returns the applicable body
		cpBody* getBody() const
		{
			return m_Body;
		}
		//! Returns the applicable space
		cpSpace* getSpace() const
		{
			return m_Space;
		}

		//! Adds a pixel to the terrain
		inline void addBit(float x, float y)
		{
			int b = (int)x + (int)y*m_Width;
			m_Bits[b] = true;
			cpShape* shape = cpCircleShapeNew(m_Body, m_BitRadius, cpv(x*m_PPB, y*m_PPB));
			shape->e = 1.0; shape->u = 1.0;
			shape->collision_type = g_PhysBodyCpCollisionType;
			shape->data = m_PhysBody;
			cpSpaceAddStaticShape(m_Space, shape);
		}

	};

	// BBfunc callback for the spatial hash.
	static cpBB bbfunc(void *ptr)
	{
		cpShape *shape = (cpShape *)ptr;
		return shape->bb;
	}

}

#endif
