/*
Copyright (c) 2006-2008 Fusion Project Team

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

	//! Templatized 2D vector class
	template<class T>
	class Vector2T
	{
	public:
		typedef T type;
	public:
		//! Copying constructor.
		Vector2T(const Vector2T<T> &other)
		{
			x = other.x;
			y = other.y;
		}
		//! Convert from Box2D vector
		//Vector2T(const b2Vec2 &other);
		//! Init. constructor
		Vector2T(T x = 0.0, T y = 0.0)
		{
			this->x = x;
			this->y = y;
		}

	public:
		//! Coordinates
		T x, y;

	public:
		static inline Vector2T<T> zero()
		{
			return Vector2T<T>(0, 0);
		}

		static inline Vector2T<T> unit_x()
		{
			return Vector2T<T>(1, 0);
		}

		static inline Vector2T<T> unit_y()
		{
			return Vector2T<T>(0, 1);
		}
		///////////////
		// Assignement
		//! Assignment operator
		Vector2T<T> &operator=(const Vector2T<T> &other)
		{ 
			x = other.x;
			y = other.y;
			return *this;
		}
		//! Addition assignment operator
		Vector2T<T> &operator+=(const Vector2T<T> &other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		//! Subtraction assignment operator
		Vector2T<T> &operator-=(const Vector2T<T> &other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		//! Multiplication assignment operator
		Vector2T<T> &operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		//////////////
		// Comparison
		//! Equivalancy operator
		bool operator==(const Vector2T<T> &other) const
		{
			return ((x == other.x) && (y == other.y));
		}
		//! Not-equals operator
		bool operator!=(const Vector2T<T> &other) const
		{
			return (x != other.x) || (y != other.y);
		}

		//////////////
		// Arithmatic
		//! Addition operator
		Vector2T<T> operator+(const Vector2T<T> &other) const
		{
			return Vector2T<T>(x + other.x, y + other.y);
		}
		//! Subtraction operator
		Vector2T<T> operator-(const Vector2T<T> &other) const
		{
			return Vector2T<T>(x - other.x, y - other.y);
		}
		//! Multiplication operator
		Vector2T<T> operator*(T scalar) const
		{
			return Vector2T<T>(scalar * x, scalar * y);
		}
		//! Division operator
		//Vector2T operator/ (float scalar) const;
		//! Friend access for multiplication operator
		/*!
		 * For Scalar * Vector2 (scalar on LHS, that is)
		 * \todo Make operator* (scalar * vector) work
		 */
		friend Vector2T<T> operator*(T scalar, const Vector2T<T> &vector);

		//! Negation operator
		Vector2T<T> operator-() const
		{
			return Vector2T<T>(-x, -y);
		}

		//! Returns reference to n-th ordinate (0 == x, 1 == y).
		/*!
		 * \param n Number of ordinate (starting with 0).
		 */
		T& operator[](int n)
		{
			switch (n)
			{
			case 0:	return x;
			case 1: return y;
			}

			static float dummy = 0.0f;
			return dummy;
		}

	public:
		inline T getX() const { return x; }
		inline T getY() const { return y; }
		//! The length of the vector
		/*!
		 * \returns sqrt(x^2+y^2)
		 */
		float length() const
		{
#ifdef WIN32
			return sqrt(x*x + y*y);
#else
			return std::sqrt(x*x + y*y);
#endif
		}
		//! The squared length of the vector
		/*!
		 * \returns x^2+y^2
		 */
		float squared_length() const
		{
			return (x*x + y*y);
		}
		//! Returns the dot product
		float dot(const Vector2T<T>& other) const
		{
			return x*other.x + y*other.y;  
		}
		//! Returns the cross product
		float cross(const Vector2T<T>& other) const
		{
			return ( x*other.y - y*other.x );
		}
		//! Projects this vector onto the given one
		Vector2T<T> project(const Vector2T<T>& other) const
		{
			//       v1 * (v1 dot v2 / v2 dot v2)
			return ( (*this) * (this->dot(other) / other.dot(other)) );
		}
		//! Rotates this vector by another
		/*!
		 * Uses complex multiplication to rotate (and scale) this by the
		 * given vector.
		 */
		void rotate(const Vector2T<T>& other)
		{
			x = (x   * other.x - y   * other.y);
			y = (x   * other.y + y   * other.x);
		}
		//! Inverse of Vector2T#rotate()
		void unrotate(const Vector2T<T>& other)
		{
			x = (x   * other.x + y   * other.y);
			y = (x   * other.y - y   * other.x);
		}
		//! Returns the angle between two vectors
		/*!
		 * Returns the angle <i>from</i> the given vector; i.e. if the given
		 * other vector is ahead of this, the angle returned will be negative.
		 */
		float angleFrom(const Vector2T<T>& other)
		{
			float cosine = this->x * other.x + this->y * other.y / (this->length() * other.length());
			// rounding errors might make dotproduct out of range for cosine
			if (cosine > 1) cosine = 1;
			else if (cosine < -1) cosine = -1;

			if ((this->x * other.y - this->y * other.x) < 0)
				return -acos(cosine);
			else
				return acos(cosine);
		}
		//! Normalizes this vector
		/*!
		 * \returns The orginal length of this vector
		 */
		float normalize()
		{
			float f = length();
			if (f!=0)
			{
				x /= f;
				y /= f;
			}
			return f;
		}
		//! Returns a normalized copy of this vector
		/*!
		* \returns This vector, normalised
		*/
		Vector2T<T> normalized()
		{
			Vector2T<T> u;
			float l = length();
			if (l!=0)
			{
				u.x = this->x/l;
				u.y = this->y/l;
			}
			return u;
		}
		//! Returns a vector perpendicular to this
		Vector2T<T> perpendicular() const
		{
			return Vector2T<T>( -y, x );
		}
		//! Returns the normal to this vector
		/*!
		 * Returns a normalised, perpendicular vector (a 'normal'):
		 * <br>
		 * (y,-x)/sqrt(x^2+y^2)
		 */
		Vector2T<T> normal() const
		{
			float l = length();
			return Vector2T<T>( y/l, -x/l );
		}

		//! Returns the distance between two vectors as points
		static float distance(const Vector2T<T>& p1, const Vector2T<T>& p2)
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
		static float pointToSegmentDistance(const Vector2T<T>& point,
			const Vector2T<T>& ep0,
			const Vector2T<T>& ep1,
			float segmentLength,
			const Vector2T<T>& segmentNormal,
			float& segmentProjection,
			Vector2T<T>& chosen)
		{
			// convert the test point to be "local" to ep0
			Vector2T<T> local = point - ep0;

			// find the projection of "local" onto "segmentNormal"
			segmentProjection = segmentNormal.dot(local);

			// handle boundary cases: when projection is not on segment, the
			// nearest point is one of the endpoints of the segment
			if (segmentProjection < 0)
			{
				chosen = ep0;
				segmentProjection = 0;
				return Vector2T<T>::distance(point, ep0);
			}
			if (segmentProjection > segmentLength)
			{
				chosen = ep1;
				segmentProjection = segmentLength;
				return Vector2T<T>::distance(point, ep1);
			}

			// otherwise nearest point is projection point on segment
			chosen = segmentNormal * segmentProjection;
			chosen +=  ep0;
			return Vector2T<T>::distance(point, chosen);
		}
	};

	//! Stream extraction for Vector2
	template <typename T>
	std::istream& operator>> (std::istream &in, Vector2T<T> &target)
	{
		in >> target.x;
		in >> target.y;
		return in;
	}

	//! Stream serialization for Vector2
	template <typename T>
	std::ostream& operator<< (std::ostream &out, const Vector2T<T> &object)
	{
		out << object.x;
		out << object.y;
		return out;
	}

	template <class T>
	Vector2T<T> operator *(T scalar, const Vector2T<T>& vector)
	{
		return Vector2T<T>(vector.x * scalar, vector.y * scalar);
	}

	template <class T>
	static inline void v2Add(const Vector2T<T>& a, const Vector2T<T>& b, Vector2T<T>& result)
	{
		result.x = a.x + b.x;
		result.y = a.y + b.y;
	}

	template <class T>
	static inline void v2Subtract(const Vector2T<T>& a, const Vector2T<T>& b, Vector2T<T>& result)
	{
		result.x = a.x - b.x;
		result.y = a.y - b.y;
	}

	template <class T>
	static inline void v2Multiply(const Vector2T<T>& a, const Vector2T<T>& b, Vector2T<T>& result)
	{				  
		result.x = a.x * b.x;
		result.y = a.y * b.y;
	}

	template <class T>
	static inline void v2Multiply(const Vector2T<T>& a, T scalar, Vector2T<T>& result)
	{				  
		result.x = a.x * scalar;
		result.y = a.y * scalar;
	}

	template <class T>
	static inline void v2Divide(const Vector2T<T>& numerator, T scalar_denominator, Vector2T<T>& result)
	{
		double bInv = 1.0f / (double)b;
		result.x = a.x * bInv;
		result.y = a.y * bInv;
	}

	template <class T>
	static inline void v2Divide(const Vector2T<T>& numerator, const Vector2T<T>& denominator, Vector2T<T>& result)
	{
		result.x = a.x / b.x;
		result.y = a.y / b.y;
	}

	template <class T>
	static inline bool v2Equal(const Vector2T<T>& v1, const Vector2T<T> &v2, float e = 0.001f)
	{
		return (v1.x - v2.x) < e && (v1.y - v2.y) < e;
	}

	template <class T>
	static inline bool v2NotEqual(const Vector2T<T>& v1, const Vector2T<T> &v2, float e = 0.001f)
	{
		return !v2Equal(v1, v2, e);
	}

	//! Uses complex multiplication to rotate (and scale) v1 by v2.
	template <class T>
	static inline Vector2T<T> v2Rotate(const Vector2T<T>& v1, const Vector2T<T> &v2)
	{
		return Vector2T<T>(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
	}
	//!  Inverse of FusionEngine#v2Rotate()
	template <class T>
	static inline Vector2T<T> v2UnRotate(const Vector2T<T>& v1, const Vector2T<T> &v2)
	{
		return Vector2T<T>(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
	}

}

#endif
