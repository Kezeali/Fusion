/*
*  Copyright (c) 2006-2011 Fusion Project Team
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

/*! @mainpage Fusion
 *
 * Fusion is a game engine which can be scripted with
 * AngleScript. It has many features and is full of wonder.
 *
 * The project website is at http://steelfusion.sourceforge.net/
 */


#ifndef H_FusionCommon
#define H_FusionCommon

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionAssert.h"
#include "FusionExceptionFactory.h"
#include "FusionVector2.h"

#include "FusionTypes.h"
#include "FusionHashable.h"

#include <angelscript.h>

#include <Box2D\Box2D.h>

#include <algorithm>
#include <sstream>

#include <vector>

#include <boost/intrusive_ptr.hpp>

namespace FusionEngine
{

	///////////////
	// --Typedefs--
	///////////////
	typedef Vector2T<float> Vector2;
	typedef Vector2T<int> Vector2i;

	typedef std::vector<Vector2> Vector2Array;

	////////////////
	// --Constants (used by common functions below)
	////////////////
	static const float s_pi = 3.1415926f;

	//! Ratio of degrees to radians
	static const float s_DegToRad = s_pi/180.0f;
	//! Ratio of radians to degrees
	static const float s_RadToDeg = 180.0f/s_pi;

	static const double s_FloatComparisonEpsilon = 0.009;

	//! Scale for converting physics units to rendering units
	static const float s_GameUnitsPerSimUnit = 100.0f;
	//! For converting between game units and physics units
	/*
	* \see s_GameUnitsPerSimUnit
	*/
	static const float s_SimUnitsPerGameUnit = 1.0f/s_GameUnitsPerSimUnit;

	////////////////////////
	// --General functions--
	////////////////////////
	static float ToGameUnits(float sim_coord) { return sim_coord * s_GameUnitsPerSimUnit; }

	static float ToSimUnits(float game_coord) { return game_coord * s_SimUnitsPerGameUnit; }

	static inline bool fe_fzero(float value) { return fabs(value) <= (float)s_FloatComparisonEpsilon; }
	static inline bool fe_fzero(double value) { return fabs(value) <= s_FloatComparisonEpsilon; }

	static inline bool fe_fzero(float value, float e) { return fabs(value) <= e; }
	static inline bool fe_fzero(double value, double e) { return fabs(value) <= e; }

	static inline bool fe_fequal(float a, float b) { return fabs(a-b) <= (float)s_FloatComparisonEpsilon; }
	static inline bool fe_fequal(double a, double b) { return fabs(a-b) <= s_FloatComparisonEpsilon; }

	static inline bool fe_fequal(float a, float b, float e) { return fabs(a-b) <= e; }
	static inline bool fe_fequal(double a, double b, double e) { return fabs(a-b) <= e; }

	//! Converts deg to rad
	static inline float fe_degtorad(float deg) { return deg * s_DegToRad; }
	//! Converts deg to rad (double)
	static inline double fe_degtorad(double deg) { return deg * s_DegToRad; }

	//! Converts rad to deg
	static inline float fe_radtodeg(float rad) { return rad * s_RadToDeg; }
	//! Converts rad deg (double)
	static inline double fe_radtodeg(double rad) { return rad * s_RadToDeg; }

	//! Returns true if the bit is set
	template <unsigned int i>
	inline bool checkbit(uint16_t mask)
	{
		return !!(mask & (0x1 << i));
	}

	/*!
	* /brief
	* Determines whether a string contains only alphabet characters
	*/
	static bool fe_isalpha(const std::string &str)
	{
		for (std::string::const_iterator it = str.begin(), end = str.end(); it != end; ++it)
		{
			if (!isalpha(*it))
				return false;
		}
		return true;
	}

	/*!
	* /brief
	* Determines whether a string contains only numeric characters (digits 1-9)
	*
	* /remarks
	* This will not work for floating point or negative numbers, as they contain punctuation.
	*/
	static bool fe_issimplenumeric(const std::string &str)
	{
		if (str.empty())
			return false;

		for (std::string::const_iterator it = str.begin(), end = str.end(); it != end; ++it)
		{
			if (!isdigit(*it))
				return false;
		}
		return true;
	}

	/*!
	* /brief
	* Determines whether a string is a number
	*/
	static bool fe_isnumeric(const std::string &str)
	{
		// Empty string is not considered numeric
		if (str.empty())
			return false;

		// 0 - start: accept '-' to 1; digit to 1; '.' to 2; else to 3
		// 1 - success: accept digit to 1; '.' to 2; else to 3
		// 2 - success: accept digit to 2; else to 3
		// 3 - failure: return false
		int state = 0;
		for (std::string::const_iterator it = str.begin(), end = str.end(); it != end; ++it)
		{
			switch (state)
			{
			case 0:
				if (*it == '-')
					state = 1;
				else if (*it == '.')
					state = 2;
				else if (isdigit(*it))
					state = 1;
				else
					state = 3;
				break;

			case 1:
				if (*it == '.')
					state = 2;
				else if (!isdigit(*it))
					state = 3;
				break;

			case 2:
				if (!isdigit(*it))
					state = 3;
				break;

			case 3:
				return false;

			default:
				return false;
			}
		}

		// state can't be 3 at this point (since that would have already returned false)
		//  and we've already ruled out state 0 (the only way it could still
		//  be at 0 is an empty string, which is checked at the start)
		return true;
	}

	static std::wstring fe_widen(const std::string &str)
	{
		std::wostringstream wstm;
		const std::ctype<wchar_t>& ctfacet = std::use_facet< std::ctype<wchar_t> >( wstm.getloc() );
		for( size_t i=0 ; i < str.size() ; ++i )
			wstm << ctfacet.widen( str[i] );
		return wstm.str();
	}

	static std::string fe_narrow(const std::wstring &str)
	{
		std::ostringstream stm;
		const std::ctype<wchar_t>& ctfacet = std::use_facet< std::ctype<wchar_t> >( stm.getloc() );
		for( size_t i=0 ; i < str.size() ; ++i )
			stm << ctfacet.narrow( str[i], 0 );
		return stm.str();
	}

	//! Returns the given string in upper case
	static std::string fe_newupper(const std::string &str)
	{
		std::string upper(str);
		std::transform(str.begin(), str.end(), upper.begin(), toupper);
		return upper;
	}

	//! Returns the given string in upper case
	static std::string fe_newlower(const std::string &str)
	{
		std::string lower(str);
		std::transform(str.begin(), str.end(), lower.begin(), tolower);
		return lower;
	}

	//! Returns the given text trimed (white-space removed)
	static std::string fe_trim(const std::string &str)
	{
		std::string::size_type first_char = str.find_first_not_of(" \r\n\t");
		std::string::size_type last_char = str.find_last_not_of(" \r\n\t");
		if (first_char == std::string::npos)
			return std::string();
		if (last_char == std::string::npos)
			return std::string();
		return str.substr(first_char, last_char - first_char + 1);
	}

	//! Splits ('tokenizes') the given string at each instance of delim
	static std::vector<std::string> fe_splitstring(const std::string &str, const std::string &delim, bool skip_empty = true)
	{
		std::vector<std::string> result;
		std::string::size_type end_pos = 0, begin_pos = 0;
		while (true)
		{
			end_pos = str.find(delim, begin_pos);
			if (end_pos == std::string::npos)
			{
				if (begin_pos != str.length())
					result.push_back(str.substr(begin_pos));
				break;
			}
			else
			{
				if (!skip_empty || begin_pos != end_pos)
					result.push_back(str.substr(begin_pos, end_pos-begin_pos));

				begin_pos = end_pos + delim.length();
			}
		}
		return result;
	}

	//! Converts a string representing key value pairs to a map. See full description.
	/*!
	* The string format is "key: value, key: value", whitespace optional. <br>
	* Note that whitespace adjacent to delimeters be trimmed from output,
	* e.g. "Player Name: Jo Jo,  Skill Level: 21" would be equivilent to "Player Name:Jo Jo,Skill Level:21". <br>
	* On a related note, the string in the above example would be functionally
	* equivilent to the c++ expression:
	* <code>
	*  out_map["Player Name"] = "Jo Jo";
	*  out_map["Skill Level"] = "21";
	* </code>
	*/
	template <typename T>
	void fe_pairize(const std::string& entries, T& out_map)
	{
		std::string::size_type token_pos = 0, token_end = 0;
		while (true)
		{
			token_end = entries.find(":", token_pos);
			if (token_end == std::string::npos)
				break;

			std::string key = fe_trim(
				entries.substr(token_pos, token_end - token_pos)
				);

			token_pos = token_end + 1;
			token_end = entries.find(",", token_pos);

			if (!key.empty())
			{
				std::string value = entries.substr(token_pos, token_end != std::string::npos ? token_end - token_pos : token_end);
				value = fe_trim(value);

				out_map[key] = value;
			}

			if (token_end == std::string::npos)
				break; // Reached the end of the string

			// Next token pos
			token_pos = token_end + 1;
		}
	}

	//! Returns the path part of the given filename-path - i.e. "/core/file.txt" returns "/core"
	static std::string fe_getbasepath(const std::string &path)
	{
		std::string::size_type pathEnd = path.find_last_of("/");
		if (pathEnd != std::string::npos)
			return path.substr(0, pathEnd);
		else
			return "/";
	}

	//! toupper()-like function for C++ strings
	/*!
	* Transformation is done directly to the passed object
	*/
	static void fe_toupper(std::string &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}

	//! toupper()-like function for C++ strings
	/*!
	* Transformation is done directly to the passed object
	*/
	static void fe_tolower(std::string &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
	}

	//! toupper() function for C strings
	/*!
	* \param[out] upper
	* A pointer to the allocated memory in which the upper-case string will written
	*
	* \param[in] str
	* The null-terminated string to make uppercase
	*/
	static void fe_toupper(char *upper, const char *str)
	{
		for (unsigned int i = 0; i < strlen(str); i++)
			upper[i] = toupper(str[i]);
	}

	//! Like strcmp, but case-insensitive
	static int fe_nocase_strcmp(const char *x, const char *y)
	{
#if (defined _MSC_VER)
		return (_stricmp(x, y));
#else
		int ux, uy;

		do
		{
			ux = toupper((int) *x);
			uy = toupper((int) *y);
			if (ux != uy)
				return((ux > uy) ? 1 : -1);
			x++;
			y++;
		} while ((ux) && (uy));

		return(0);
#endif
	}

	//! Converts an int to a C string
	//static inline void fe_itoa(int v, char &buffer, int radix = 10)
	//{
	//	_itoa_s(v, buffer, radix);
	//}
	//#ifdef _WIN32
	//#define fe_itoa(v, &buffer, radix) _itoa_s(v, buffer, radix)
	//#else
	//#define fe_itoa(v, &buffer, radix) itoa(v, buffer, radix)
	//#endif

	//! Returns rounded value converted to int64
	static inline int64_t fe_lround(double v) { return static_cast<int64_t>(v > 0.0 ? v + 0.5 : v - 0.5); }
	//! Returns rounded value converted to type R, where R is an integral type
	template<typename R, typename T>
	static inline typename std::enable_if<std::is_integral<R>::value, R>::type fe_round(T v)
	{ static_assert(std::is_floating_point<T>::value, "No need to round an integer!"); return static_cast<R>(v > 0.0 ? v + 0.5 : v - 0.5); }
	//! Returns rounded value converted to type R, where R is not an integral type
	template<typename R, typename T>
	static inline typename std::enable_if<std::is_floating_point<R>::value && !std::is_same<R, T>::value, R>::type fe_round(T v)
	{ static_assert(std::is_floating_point<T>::value, "No need to round an integer!"); return static_cast<R>(static_cast<int64_t>(v > 0.0 ? v + 0.5 : v - 0.5)); }
	//! Returns rounded value
	template<typename T>
	static inline T fe_round(T v)
	{ static_assert(std::is_floating_point<T>::value, "No need to round an integer!"); return static_cast<T>(static_cast<int64_t>(v > 0.0 ? v + 0.5 : v - 0.5)); }

	//! Returns the bigger value
	template<class T>
	static inline T fe_max(const T &a, const T &b) { return (a > b) ? a : b; }
	//! Returns the smaller value
	template<class T>
	static inline T fe_min(const T &a, const T &b) { return (a < b) ? a : b; }

	//! Wraps a around if it is below lb or above ub
	template<class T>
	static inline T fe_wrap(const T &a, const T &lb, const T &ub) 
	{
		// This basically ammounts to:
		//  if (a < lb) return a + ub;
		//  else if (a > ub) return a - ub;
		//  else return a;
		return a < lb ? a + ub : (a > ub ? a - ub : a);
	}


	//! Returns a if it is no less than lb, and no greater than ub
	template <class T>
	static inline const T &fe_clamped(const T &a, const T &lb, const T &ub) 
	{
		return a < lb ? lb : (ub < a ? ub : a); 
	}
	//! Sets a to no less than lb, and no greater than ub
	template <class T>
	inline void fe_clamp(T &a, const T &lb, const T &ub) 
	{
		if (a < lb) 
		{
			a = lb; 
		}
		else if (a > ub) 
		{
			a = ub;
		}
	}

	//! Checks for the given flag in the given bit-set
	inline bool fe_checkflag(int bits, int flag)
	{
		return (bits & flag) != 0;
	}

	//! Converts Box2D Vector2 to FE Vector2
	static inline Vector2 b2v2(const b2Vec2 &other)
	{
		return Vector2(other.x, other.y);
	}

	static inline const char* ctxGetModuleName(asIScriptContext *context)
	{
		FSN_ASSERT(context->GetState() == asEXECUTION_ACTIVE ||
			context->GetState() == asEXECUTION_EXCEPTION ||
			context->GetState() == asEXECUTION_SUSPENDED);

		return context->GetFunction()->GetModuleName();
	}

	static inline asIScriptModule* ctxGetModule(asIScriptContext *context, asEGMFlags flag = asGM_ONLY_IF_EXISTS)
	{
		FSN_ASSERT(context->GetState() == asEXECUTION_ACTIVE ||
			context->GetState() == asEXECUTION_EXCEPTION ||
			context->GetState() == asEXECUTION_SUSPENDED);

		return context->GetEngine()->GetModule(ctxGetModuleName(context), flag);
	}

	static inline asIScriptObject* ctxGetObject(asIScriptContext *context)
	{
		FSN_ASSERT(context->GetState() == asEXECUTION_ACTIVE ||
			context->GetState() == asEXECUTION_EXCEPTION ||
			context->GetState() == asEXECUTION_SUSPENDED);

		FSN_ASSERT((context->GetThisTypeId() & asTYPEID_APPOBJECT) == 0);

		return static_cast<asIScriptObject*>(context->GetThisPointer());
	}

}

#endif
