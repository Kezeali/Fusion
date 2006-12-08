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

/// LinearParticle
#include "..\LinearParticle\include\L_ParticleSystem.h"

/// AngelScript
//#define AS_USE_NAMESPACE
#include <angelscript.h>

/// CEGUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/openglrenderer.h>

#include "FusionStringVector.h"

namespace FusionEngine
{
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
	class FusionPhysicsResponse;
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
