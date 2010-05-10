
/*! @file FusionPrerequisites.h
 * I didn't know what to call the file that defines forward decl.s, so I just copied
 * the name for a simmilar file in Ogre3D even though it seems like a misnomer
 */

#ifndef Header_FusionPrerequisites
#define Header_FusionPrerequisites

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionStdHeaders.h"

#include <boost/intrusive_ptr.hpp>

namespace FusionEngine
{

	///////////////////////////
	// --Forward declarations--
	///////////////////////////
	class Camera;
	class ClientOptions;
	class Console;
	class Editor;
	class EditorMapEntity;
	class Entity;
	class EntityFactory;
	class EntitySynchroniser;
	class EntityManager;
	class Exception;
	class FileSystemException;
	class Fixture;
	class GameMapLoader;
	class InputDefinitionLoader;
	class InputManager;
	class InstancingSynchroniser;
	class Log;
	class Logger;
	class Module;
	class Network;
	class NetworkManager;
	class NetworkSystem;
	class OntologicalSystem;
	class IPacket;
	class PacketDispatcher;
	class PacketHandler;
	class PacketHandlerNode;
	class PhysicalWorld;
	class PlayerInput;
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

	//! EditorMapEntity ptr
	typedef boost::intrusive_ptr<EditorMapEntity> EditorMapEntityPtr;

}

#endif
