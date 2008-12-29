/*
  Copyright (c) 2006-2007 Fusion Project Team

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

/*! @mainpage Fusion
 *
 * Fusion is a multiplayer, 2D / top-down shooter, based on a
 * scriptable and highly extendable engine.
 *
 * Go to http://steelfusion.sourceforge.net/ for more information.
 */

/*!
 * \todo
 * Implement a better assert macro (than cl_assert)
 */

#ifndef Header_FusionEngine_Common
#define Header_FusionEngine_Common

#if _MSC_VER > 1000
#pragma once
#endif

//! XML version to write to the xml declaration of new files
#define XML_STANDARD "1.0"


#include "Common.h"

#include "FusionVector2.h"

#include "FusionSlotContainer.h"

#include "FusionException.h"
#include "FusionExceptionFactory.h"
//#include "FusionFileSystemException.h"
//#include "FusionFileNotFoundException.h"
//#include "FusionFileTypeException.h"
//#include "FusionInvalidArgumentException.h"

// Chipmunk
//#include <chipmunk.h>

#include <Box2D.h>

// Boost
//#include <boost/function.hpp>
//#include <boost/bind.hpp>

// TinyXML
#define USE_TINYXPATH

#ifdef USE_TINYXPATH
# define TIXML_USE_TICPP
# define TIXML_USE_STL
#include "../tinyxpathpp/xpath_static.h"
#include "../tinyxpathpp/ticpp.h"

#else
//#ifndef TIXML_USE_STL
//# define TIXML_USE_STL
//#endif
#ifndef TIXML_USE_TICPP
# define TIXML_USE_TICPP
#endif
#include "../tinyxml/tinyxml.h"
#endif


// AngelScript
//not #define AS_USE_NAMESPACE
#include <angelscript.h>

namespace FusionEngine
{

	typedef Vector2T<float> Vector2;

	//! Ratio of degrees to radians
	static const double s_DegToRad = M_PI/180.0f;
	//! Ratio of radians to degrees
	static const double s_RadToDeg = 180.0f/M_PI;

	static const double s_FloatComparisonEpsilon = 0.05;


	////////////////////////
	// --General functions--
	////////////////////////

	static inline bool fe_fzero(float value) { return fabs(value) <= (float)s_FloatComparisonEpsilon; }
	static inline bool fe_fzero(double value) { return fabs(value) <= s_FloatComparisonEpsilon; }

	static inline bool fe_fzero(float value, float e) { return fabs(value) <= e; }
	static inline bool fe_fzero(double value, double e) { return fabs(value) <= e; }

	static inline bool fe_fequal(float a, float b) { return fabs(a-b) <= (float)s_FloatComparisonEpsilon; }
	static inline bool fe_fequal(double a, double b) { return fabs(a-b) <= s_FloatComparisonEpsilon; }

	static inline bool fe_fequal(float a, float b, float e) { return fabs(a-b) <= e; }
	static inline bool fe_fequal(double a, double b, double e) { return fabs(a-b) <= e; }

	//! Converts deg to rad
	static inline float fe_degtorad(float deg) { return deg * (float)s_DegToRad; }
	//! Converts deg to rad (double)
	static inline double fe_degtorad(double deg) { return deg * s_DegToRad; }

	//! Converts rad to deg
	static inline float fe_radtodeg(float rad) { return rad * (float)s_RadToDeg; }
	//! Converts rad deg (double)
	static inline double fe_radtodeg(double rad) { return rad * s_RadToDeg; }

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
		// 1 - success: accept digit to ; '.' to 2; else to 3
		// 2 - success: accept digit to 2; else to 3
		// 3 - accept anything to 3
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

	//! Returns rounded value converted to long int
	static inline long int fe_lround(double v) { return static_cast<long int>( v + (v > 0.0 ? 0.5 : -0.5) ); }
	//! Returns rounded value
	template<typename T>
	static inline T fe_round(T v) { return fe_lround(static_cast<double>(v)); }

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
	static inline void fe_clamp(T &a, const T &lb, const T &ub) 
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

	//! Converts Box2D Vector2 to FE Vector2
	static inline Vector2 b2v2(const b2Vec2 &other)
	{
		return Vector2(other.x, other.y);
	}


	///////////////////////////
	// --Forward declarations--
	///////////////////////////
	//! \todo List forward declarations in alphabetical order
	//class Archive;
	class Control;
	class ClientOptions;
	class ServerOptions;
	class Environment;
	class ServerEnvironment;
	class ClientEnvironment;
	class Network;
	class NetworkServer;
	class NetworkClient;
	class Scene;
	class INode;
	class ShipState;
	struct ShipInput;
	class ProjectileState;
	class FusionShip;
	class FusionProjectile;
	class PhysicsWorld;
	class PhysicsBody;
	class Shape;
	class CollisionGrid;
	class FusionState;
	class FusionStatePackSync;
	class FusionStateMessage;
	class StateManager;
	class Script;
	class ScriptingEngine;
	class Exception;
	class FileSystemException;
	class Logger;
	class Console;
	class LoadingState;
	class ClientLoadingState;
	class ServerLoadingState;
	class ClientLoadingSyncCallback;
	class ServerLoadingSyncCallback;
	class PackSyncClient;
	class PackSyncServer;
	class LoadingStage;
	class ResourceContainer;
	class ResourceManager;
	class InputPluginLoader;


	///////////////
	// --Typedefs--
	///////////////

	//! Unique identifier type for game objects
	typedef unsigned short ObjectID;
	//typedef std::string ResourceID;

	//! It's a vector. It's a string. It's a StringVector! (it's used /atleast/ three times...)
	typedef std::vector<std::string> StringVector;

	//! Type for a list of bodies
	typedef std::vector<PhysicsBody*> BodyList;

	//! Self managing state pointer
	/*!
	 * Use it!
	 */
	typedef CL_SharedPtr<FusionState> SharedState;

	//! Resource tags (aka. names/handles)
	typedef std::string ResourceTag;

	//! ID for script functions
	typedef std::string ScriptFuncSig;
}

#endif
