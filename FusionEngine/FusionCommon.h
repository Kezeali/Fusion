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
 * highly scriptable and extendable engine.
 *
 * Go to http://steelfusion.sourceforge.net/ for more information.
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
#include "FusionException.h"

// Boost
//#include <boost/function.hpp>
//#include <boost/bind.hpp>

// TinyXML
//#define TIXML_USE_STL
#include "../tinyxml/tinyxml.h"

// AngelScript
//not #define AS_USE_NAMESPACE
#include "angelscript.h"

//! \todo Load exception error messages from XML
namespace FusionEngine
{

	//! Ratio of degrees to radians
	static const double g_DegToRad = M_PI/180.0f;
	//! Ratio of radians to degrees
	static const double g_RadToDeg = 180.0f/M_PI;

	////////////////////////
	// --General functions--
	////////////////////////

	//! Converts deg to rad
	static inline float fe_degtorad(float deg) { return deg * (float)g_DegToRad; }
	//! Converts deg to rad (double)
	static inline double fe_degtorad(double deg) { return deg * g_DegToRad; }

	//! Converts rad to deg
	static inline float fe_radtodeg(float rad) { return rad * (float)g_RadToDeg; }
	//! Converts rad deg (double)
	static inline double fe_radtodeg(double rad) { return rad * g_RadToDeg; }


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


	///////////////////////////
	// --Forward declarations--
	///////////////////////////
	class Archive;
	class ClientOptions;
	class ServerOptions;
	class GenericEnvironment;
	class ServerEnvironment;
	class ClientEnvironment;
	class FusionNetworkGeneric;
	class FusionNetworkServer;
	class FusionNetworkClient;
	class FusionScene;
	class FusionNode;
	class ShipState;
	struct ShipInput;
	class ProjectileState;
	class FusionShip;
	class FusionProjectile;
	class ShipResourceBundle;
	class FusionPhysicsWorld;
	class FusionPhysicsBody;
	class FusionPhysicsCollisionGrid;
	class FusionState;
	class FusionStatePackSync;
	class FusionStateMessage;
	class StateManager;
	class Script;
	class ScriptingEngine;
	class Exception;
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


	///////////////
	// --Typedefs--
	///////////////

	//! Unique identifier type for game objects
	typedef unsigned short ObjectID;
	//typedef std::string ResourceID;

	//! It's a vector. It's a string. It's a StringVector! (it's used /atleast/ three times...)
	typedef std::vector<std::string> StringVector;

	//! Type for a list of bodies
	typedef std::vector<FusionPhysicsBody*> BodyList;

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
