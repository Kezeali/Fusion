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

#ifndef Header_Fusion_Common
#define Header_Fusion_Common

#if _MSC_VER > 1000
#pragma once
#endif

#include "Common.h"

/// Boost
#include <boost/function.hpp>
#include <boost/bind.hpp>

/// AngelScript
//#define AS_USE_NAMESPACE
#include <angelscript.h>

namespace FusionEngine
{
	//! Ratio of degrees to radians
	static const double g_DegToRad = M_PI/180.0f;

	// --Useful functions--
	// Ohh, I'm such a rebel - look at all these macro-type functions in smallcaps!

	//! Converts deg to rad
	static inline float fe_degtorad(float deg) { return deg * g_DegToRad; }

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


	//! Returns the bigger value
	template<class T>
	static inline const T &fe_max(const T &a, const T &b) {return (a > b) ? a : b; }
	//! Returns the smaller value
	template<class T>
	static inline const T &fe_min(const T &a, const T &b) {return (a < b) ? a : b; }

	//! Wraps a around if it is below lb or above ub
	template<class T>
	static inline const T &fe_wrap(const T &a, const T &lb, const T &ub) 
	{
		// This basically ammounts to:
		//  if (a <= lb) return ub;
		//  else if (a > ub) return lb;
		//  else return a;
		return a <= lb ? ub : (a >= ub ? lb : a);
		// It's so unreadable, it must be efficiant... Right? :P
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
		else if (ub < a) 
		{
			a = ub;
		}
	}


	// --Forward declarations--
	class GenericEnviornment;
	class ServerEnvironment;
	class ClientEnvironment;
	class FusionScene;
	class FusionNode;
	struct ShipState;
	struct ShipInput;
	struct ProjectileState;
	class FusionShip;
	class FusionProjectile;
	class ShipResource;
	class FusionPhysicsWorld;
	class FusionPhysicsBody;
	class FusionPhysicsCollisionGrid;
	class FusionState;
	class FusionStatePackSync;
	class FusionStateMessage;
	class StateManager;
	class Script;
	class ScriptingEngine;


	// --Typedefs--
	typedef unsigned short PlayerInd;
	typedef unsigned short ObjectID;
	//typedef std::string ResourceID;

	//! It's a vector. It's a string. It's a StringVector! (it's used /atleast/ three times...)
	typedef std::vector<std::string> StringVector;

	//! Type for a list of bodies
	typedef std::list<FusionPhysicsBody *> BodyList;
}

#endif
