/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_FusionEngine_Common
#define Header_FusionEngine_Common

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
	// Useful inline functions
	//! Returns the bigger value
	template<class T>
	inline const T &fe_max(const T &a, const T &b) {return (a > b) ? a : b; }
	//! Returns the smaller value
	template<class T>
	inline const T &fe_min(const T &a, const T &b) {return (a < b) ? a : b; }
	//! Wraps a around if it is below lb or above ub
	template<class T>
	inline const T &fe_wrap(const T &a, const T &lb, const T &ub) 
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
	inline const T &fe_clamped(const T &a, const T &lb, const T &ub) 
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
		else if (ub <= a) 
		{
			a = ub;
		}
	}

	// Forward declarations
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
	class Archive;

	// Global typedefs
	typedef unsigned short PlayerInd;
	typedef unsigned short ObjectID;
	//typedef std::string ResourceID;
}

#endif
