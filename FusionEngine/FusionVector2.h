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
		Vector2 &operator= (const Vector2 &other);
		//! Addition assignment operator
		Vector2 &operator+= (const Vector2 &other);
		//! Subtraction assignment operator
		Vector2 &operator-= (const Vector2 &other);
		//! Multiplication assignment operator
		Vector2 &operator*= (float scalar);

		//////////////
		// Comparison
		//! Equivalancy operator
		bool operator== (const Vector2 &other) const;
		//! Not-equals operator
		bool operator!= (const Vector2 &other) const;

		//////////////
		// Arithmatic
		//! Addition operator
		Vector2 operator+ (const Vector2 &other) const;
		//! Subtraction operator
		Vector2 operator- (const Vector2 &other) const;
		//! Multiplication operator
		Vector2 operator* (float scalar) const;
		//! Division operator
		//Vector2 operator/ (float scalar) const;
		//! Friend access for multiplication operator
		/*!
		 * For Scalar * Vector2 (scalar on LHS, that is)
		 */
		friend Vector2 operator * (float scalar, const Vector2 &vector);

		//! Negation operator
		Vector2 operator- () const;

		//! Returns reference to n-th ordinate (0. == x, 1. == y, ...).
		/*!
		 * \param n Number of ordinate (starting with 0).
		 */
		float& operator [] (int n);

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
		float dot(const Vector2 &other) const;
		//! Normalizes this vector
		/*!
		 * \returns The orginal length of this vector
		 */
		float normalize();
		//! Returns a normalised, perpendicular vector
		/*!
		 * (y,-x)/sqrt(x^2+y^2)
		 */
		Vector2 normal() const;
	};

}

#endif
