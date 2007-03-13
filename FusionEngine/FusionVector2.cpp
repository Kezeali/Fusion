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

		Magnus Norddahl (Some code from ClanLib's CL_Vector is used)
*/

#include "FusionVector2.h"

namespace FusionEngine
{
	const Vector2 Vector2::ZERO(0.0f,0.0f);
	const Vector2 Vector2::UNIT_X(1.0f,0.0f);
	const Vector2 Vector2::UNIT_Y(0.0f,1.0f);

	////////////////
	// Construction
	Vector2::Vector2(const Vector2 &other)
	{
		x = other.x;
		y = other.y;
	}

	Vector2::Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	/////////////
	// Operators
	// Assignment
	Vector2& Vector2::operator=(const Vector2& v)
	{ 
		x = v.x;
		y = v.y;
		return *this;
	}

	Vector2 &Vector2::operator+=(const Vector2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	Vector2 &Vector2::operator-=(const Vector2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vector2 &Vector2::operator*=(float s)
	{
		x *= s;
		y *= s;
		return *this;
	}

	//////////////
	// Comparison
	bool Vector2::operator==(const Vector2& v) const
	{
		return ((x == v.x) && (y == v.y));
	}

	bool Vector2::operator!=(const Vector2& v) const
	{
		return !(operator == (v));
	}

	//////////////
	// Arithmatic
	Vector2 Vector2::operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2 Vector2::operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	Vector2 Vector2::operator*(float s) const
	{
		return Vector2(s * x, s * y);
	}

	Vector2 operator*(float s, CL_Vector const &v)
	{
		return Vector2(s * v.x, s * v.y);
	}


	Vector2 Vector2::operator-() const
	{
		return Vector2(-x, -y);
	}


	float &Vector2::operator[](int n)
	{
		switch (n)
		{
		case 0:	return x;
		case 1: return y;
		}

		static float dummy = 0.0f;
		return dummy;
	}

	//////////
	// Methods
	float Vector2::length() const
	{
#ifdef WIN32
		return sqrt(x*x + y*y);
#else
		return std::sqrt(x*x + y*y);
#endif
	}

	float Vector2::squared_length() const
	{
		return (x*x + y*y);
	}

	float Vector2::dot(const Vector2& v) const
	{
		return x*v.x + y*v.y;  
	}

	float Vector2::cross(const Vector2& v) const
	{
		return ( x*v.y - y*v.x );
	}

	Vector2 Vector2::project(const Vector2& v) const
	{
		//       v1 * (v1 dot v2 / v2 dot v2)
		return ( v * (this->dot(v) / v.dot(v)) );
	}

	void Vector2::rotate(const Vector2& v)
	{
		x = (x   * v.x - y   * v.y);
		y = (x   * v.y + y   * v.x);
	}

	void Vector2::unrotate(const Vector2& v)
	{
		x = (x   * v.x + y   * v.y);
		y = (x   * v.y - y   * v.x);
	}

	float Vector2::angleFrom(const Vector2& v)
	{
		return ( atan2(this->y,this->x) - atan2(v.y,v.x) );
		//return acosf(this->dot(v));
	}

	float Vector2::normalize()
	{
		float f = length();
		if (f!=0)
		{
			x /= f;
			y /= f;
		}
		return f;
	}

	Vector2 Vector2::normalized()
	{
		Vector2 u;
		float l = length();
		if (l!=0)
		{
			u.x = this->x/l;
			u.y = this->y/l;
		}
		return u;
	}

	Vector2 Vector2::perpendicular() const
	{
		return Vector2( -y, x );
	}

	Vector2 Vector2::normal() const
	{
		float l = length();
		return Vector2( y/l, -x/l );
	}

}
