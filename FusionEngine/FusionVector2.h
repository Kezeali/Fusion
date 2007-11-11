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


	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_Vector2
#define Header_FusionEngine_Vector2

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

namespace FusionEngine
{

	//! A 2D vector class
	class Vector2
	{
	public:
		//! Copying constructor.
		Vector2(const Vector2 &other);
		//! Init. constructor
		Vector2(float x = 0.0, float y = 0.0);

	public:
		//! Coordinates
		float x, y;

		//! (0,0)
		static const Vector2 ZERO;
		//! (1,0)
		static const Vector2 UNIT_X;
		//! (0,1)
		static const Vector2 UNIT_Y;

	public:
		///////////////
		// Assignement
		//! Assignment operator
		Vector2 &operator=(const Vector2 &other);
		//! Addition assignment operator
		Vector2 &operator+=(const Vector2 &other);
		//! Subtraction assignment operator
		Vector2 &operator-=(const Vector2 &other);
		//! Multiplication assignment operator
		Vector2 &operator*=(float scalar);

		//////////////
		// Comparison
		//! Equivalancy operator
		bool operator==(const Vector2 &other) const;
		//! Not-equals operator
		bool operator!=(const Vector2 &other) const;

		//////////////
		// Arithmatic
		//! Addition operator
		Vector2 operator+(const Vector2 &other) const;
		//! Subtraction operator
		Vector2 operator-(const Vector2 &other) const;
		//! Multiplication operator
		Vector2 operator*(float scalar) const;
		//! Division operator
		//Vector2 operator/ (float scalar) const;
		//! Friend access for multiplication operator
		/*!
		 * For Scalar * Vector2 (scalar on LHS, that is)
		 * \todo Make operator* (scalar * vector) work
		 */
		friend Vector2 operator*(float scalar, const Vector2 &vector);

		//! Negation operator
		Vector2 operator-() const;

		//! Returns reference to n-th ordinate (0. == x, 1. == y, ...).
		/*!
		 * \param n Number of ordinate (starting with 0).
		 */
		float& operator[](int n);

	public:
		//! The length of the vector
		/*!
		 * \returns sqrt(x^2+y^2)
		 */
		float length() const;
		//! The squared length of the vector
		/*!
		 * \returns x^2+y^2
		 */
		float squared_length() const;
		//! Returns the dot product
		float dot(const Vector2& other) const;
		//! Returns the cross product
		float cross(const Vector2& other) const;
		//! Projects this vector onto the given one
		Vector2 project(const Vector2& other) const;
		//! Rotates this vector by another
		/*!
		 * Uses complex multiplication to rotate (and scale) this by the
		 * given vector.
		 */
		void rotate(const Vector2& other);
		//! Inverse of Vector2#rotate()
		void unrotate(const Vector2& other);
		//! Returns the angle between two vectors
		/*!
		 * Returns the angle <i>from</i> the given vector; i.e. if the given
		 * other vector is ahead of this, the angle returned will be negative.
		 */
		float angleFrom(const Vector2& other);
		//! Normalizes this vector
		/*!
		 * \returns The orginal length of this vector
		 */
		float normalize();
		//! Returns a normalized copy of this vector
		/*!
		 * \returns This vector, normalised
		 */
		Vector2 normalized();
		//! Returns a vector perpendicular to this
		Vector2 perpendicular() const;
		//! Returns the normal to this vector
		/*!
		 * Returns a normalised, perpendicular vector (a 'normal'):
		 * <br>
		 * (y,-x)/sqrt(x^2+y^2)
		 */
		Vector2 normal() const;

		//! Returns the distance between two vectors as points
		static float distance(const Vector2& p1, const Vector2& p2)
		{
			return (p1-p2).length();
		}

		//! Compute minimum distance from a point to a line segment
		/*!
		 * \param[in] point
		 * The point
		 *
		 * \param[in] ep0
		 * Point 0 on the line
		 *
		 * \param[in] ep1
		 * Point 1 on the line
		 *
		 * \param[in] segmentLength
		 * Length of the segment (usualy used by caller also, so no point calculating it again)
		 *
		 * \param[out] segmentProjection
		 * Segment projection (<code>segmentNormal.dot(point - ep0)</code>). Rather than
		 * calling dot again later on the same params, this is used (optimisation)
		 *
		 * \param[out] segmentProjection
		 * The point on the line chosen as the nearest point to the one given
		 */
		static float pointToSegmentDistance(const Vector2& point,
	                               const Vector2& ep0,
	                               const Vector2& ep1,
	                               float segmentLength,
	                               const Vector2& segmentNormal,
	                               float& segmentProjection,
	                               Vector2& chosen);
	};

	static inline void v2Add(const Vector2& a, const Vector2& b, Vector2& c)
	{
		c.x = a.x + b.x;
		c.y = a.y + b.y;
	}

	static inline void v2Subtract(const Vector2& a, const Vector2& b, Vector2& c )
	{
		c.x = a.x - b.x;
		c.y = a.y - b.y;
	}

	static inline void v2Multiply(const Vector2& a, float b, Vector2& c )
	{
		c.x = a.x * b;
		c.y = a.y * b;
	}

	static inline void v2Multiply(const Vector2& a, const Vector2& b, Vector2& c )
	{				  
		c.x = a.x * b.x;
		c.y = a.y * b.y;
	}


	static inline void v2Divide(const Vector2& a, float b, Vector2& c )
	{
		float bInv = 1.0f / b;
		c.x = a.x * bInv;
		c.y = a.y * bInv;
	}

	static inline void v2Divide(const Vector2& a, const Vector2& b, Vector2& c )
	{
		c.x = a.x / b.x;
		c.y = a.y / b.y;
	}

	static inline bool v2Equal(const Vector2& v1, const Vector2 &v2)
	{
		return (v1.x == v2.x) && (v1.y == v2.y);
	}

	static inline bool v2NotEqual(const Vector2& v1, const Vector2 &v2)
	{
		return (v1.x != v2.x) || (v1.y != v2.y);
	}

	//! Uses complex multiplication to rotate (and scale) v1 by v2.
	static inline Vector2 v2Rotate(const Vector2& v1, const Vector2 &v2)
	{
		return Vector2(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
	}
	//!  Inverse of FusionEngine#v2Rotate()
	static inline Vector2 v2UnRotate(const Vector2& v1, const Vector2 &v2)
	{
		return Vector2(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
	}

}

#endif
