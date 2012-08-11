/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionPrerequisites
#define H_FusionPrerequisites

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER > 1000 && _MSC_VER < 1600
#error Visual Studio 2010 or later required (for c++11 features)
#endif

//#include "FusionStdHeaders.h"

#ifdef MessageBox
#undef MessageBox
#endif

#ifdef Yield
#undef Yield
#endif

#ifdef GetObject
#undef GetObject
#endif

#define FSN_TBB_AVAILABLE

#ifndef BOOST_BIND_NO_PLACEHOLDERS
#define BOOST_BIND_NO_PLACEHOLDERS
#endif
#ifndef BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#define BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#endif

namespace FusionEngine
{

	///////////////////////////
	// --Forward declarations--
	///////////////////////////
	class Camera;
	class CameraSynchroniser;
	class ClientOptions;
	class EntityComponent;
	class ComponentFactory;
	class Console;
	class Editor;
	class EditorMapEntity;
	class Entity;
	class EntityRepo;
	class EntitySynchroniser;
	class EntityManager;
	class Exception;
	class FileSystemException;
	class Fixture;
	class GameMap;
	class GameMapLoader;
	class InputDefinitionLoader;
	class InputManager;
	class EntityInstantiator;
	class Log;
	class Logger;
	class MessageBox;
	class Module;
	class Network;
	class NetworkManager;
	class NetworkSystem;
	class OntologicalSystem;
	class P2PEntityInstantiator;
	class IPacket;
	class PacketDispatcher;
	class PacketHandler;
	class PacketHandlerNode;
	class PhysicalWorld;
	class PlayerInput;
	struct PlayerInfo;
	class RakNetwork;
	class Renderable;
	class Renderer;
	class ResourceContainer;
	class ResourceManager;
	class ScriptedSlotWrapper;
	class ScriptManager;
	class Shape;
	class StreamingManager;
	class System;
	class SystemMessage;
	class SystemsManager;

}

#endif
