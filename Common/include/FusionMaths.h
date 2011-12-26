/*
*  Copyright (c) 2010 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionMaths
#define H_FusionMaths

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionAssert.h"

#include <cmath>

namespace FusionEngine
{
	namespace Maths
	{

		// This doesn't really belong in here, will put it in it's own file later
		template <typename T>
		struct size_components
		{
			size_components( T both_components ) : width(both_components), height(both_components) {}
			size_components( T _width, T _height ) : width(_width), height(_height) {}
			size_components( const size_components& to_copy ) : width( to_copy.width ), height( to_copy.height ) {}
			size_components( size_components&& to_move ) : width( std::move(to_move.width) ), height( std::move(to_move.height) ) {}
			size_components& operator= ( const size_components& to_assign ) { width = to_assign.width; height = to_assign.height; return *this; }
			size_components& operator= ( size_components&& to_move ) { width = std::move(to_move.width); height = std::move(to_move.height); return *this; }
			size_components& operator= ( T size ) { width = size; height = size; return *this; }
			bool operator== ( const size_components& other ) const { return (other.width == this->width && other.height == this->height); }
			bool operator!= ( const size_components& other ) const { return (other.width != this->width || other.height != this->height); }
			bool operator== ( T both_equal ) const { return (both_equal == this->width && both_equal == this->height); }
			bool operator!= ( T either_not_equal ) const { return (either_not_equal != this->width || either_not_equal != this->height); }
			bool either_is( T equal_to ) const { return (equal_to == this->width || equal_to == this->height); }

			T width, height;
		};

		template <typename T>
		inline T Clamp(T value, T lower_bound, T upper_bound)
		{
			FSN_ASSERT(lower_bound < upper_bound);
			if (value < lower_bound)
				return lower_bound;
			else if (value > upper_bound)
				return upper_bound;
			else
				return value;
		}

		//template <typename T>
		//inline T& Clamp(T& value, T& lower_bound, T& upper_bound)
		//{
		//	FSN_ASSERT(lower_bound < upper_bound);
		//	return (value < lower_bound) ? lower_bound : (value > upper_bound) ? upper_bound : value;
		//}

		template <typename T>
		inline void ClampThis(T& value, T lower_bound, T upper_bound)
		{
			FSN_ASSERT(lower_bound < upper_bound);
			value = (value < lower_bound) ? lower_bound : (value > upper_bound) ? upper_bound : value;
		}

		template <typename T>
		void Lerp(T& out, const T& start, const T& end, float alpha)
		{
			out = start * (1 - alpha) + end * alpha;
		}

		static void AngleInterp(float& out, float start, float end, float alpha)
		{
			if (std::abs(end - start) < b2_pi)
			{
				Lerp(out, start, end, alpha);
				return;
			}

			if (start < end)
				start += b2_pi * 2.f;
			else
				end += b2_pi * 2.f;

			Lerp(out, start, end, alpha);
		}
	}
}

#endif