/*
  Copyright (c) 2009 Fusion Project Team

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

#include "Common.h"

// Class
#include "FusionOntologicalSystem.h"

// Fusion
#include "FusionEntityManager.h"
#include "FusionEntityFactory.h"
#include "FusionGameMapLoader.h"
#include "FusionPhysicalEntityManager.h"
#include "FusionInputHandler.h"
#include "FusionNetworkSystem.h"
#include "FusionScriptingEngine.h"
#include "FusionClientOptions.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIODeviceProvider.h"
#include "FusionXml.h"
#include "FusionPlayerRegistry.h"
#include "FusionRakNetwork.h"
#include "FusionEditor.h"

#include <Calling/Caller.h>

#include <boost/lexical_cast.hpp>

#include <Bitstream.h>

#include "FusionScriptedEntity.h"


namespace FusionEngine
{

	class SimpleEntity : public PhysicalEntity
	{
	public:
		SimpleEntity(const std::string &name)
			: PhysicalEntity(name)
		{
			ScriptingEngine *manager = ScriptingEngine::getSingletonPtr();
			if (manager != NULL && manager->GetEnginePtr() != NULL)
				manager->GetEnginePtr()->NotifyGarbageCollectorOfNewObject(this, s_TypeId);
		}

		virtual ~SimpleEntity()
		{
			SendToConsole(GetName() + " was deleted");
		}

		virtual std::string GetType() const { return "Simple"; }

		virtual void Spawn()
		{
			SendToConsole(GetName() + " spawned");
		}
		virtual void Update(float split)
		{
		}

		virtual void OnStreamIn()
		{
			SendToConsole(GetName() + " streamed in");
		}
		virtual void OnStreamOut()
		{
			SendToConsole(GetName() + " streamed out");
		}

		//! Save state to buffer
		virtual void SerialiseState(SerialisedData &state, bool local) const
		{
		}

		//! Read state from buffer
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
		{
			return 0;
		}
	};

	class SimpleInstancerTest : public EntityInstancer
	{
	public:
		SimpleInstancerTest()
			: EntityInstancer("Simple")
		{
		}

		//! Returns a Simple object
		virtual Entity *InstanceEntity(const std::string &name)
		{
			return new SimpleEntity(name);
		}
	};

	const std::string s_OntologicalSystemName = "Entities";

	OntologicalSystem::OntologicalSystem(ClientOptions *options, Renderer *renderer, InputManager *input_manager, NetworkSystem *network)
		: m_Options(options),
		m_Renderer(renderer),
		m_InputManager(input_manager),
		m_NetworkSystem(network),
		m_EntityManager(NULL),
		m_MapLoader(NULL),
		m_PhysWorld(NULL),
		m_NextPlayerIndex(1)
	{
	}

	OntologicalSystem::~OntologicalSystem()
	{
		CleanUp();
	}

	const std::string &OntologicalSystem::GetName() const
	{
		return s_OntologicalSystemName;
	}

	bool OntologicalSystem::Initialise()
	{
		if (m_EntityManager == NULL)
		{
			m_EntitySyncroniser = new EntitySynchroniser(m_InputManager, m_NetworkSystem);
			m_EntityFactory = new EntityFactory();
			m_Streaming = new StreamingManager();
			m_EntityManager = new EntityManager(m_EntityFactory, m_Renderer, m_InputManager, m_EntitySyncroniser, m_Streaming);
			m_MapLoader = new GameMapLoader(m_Options, m_EntityManager);

			m_Streaming->SetRange(2000);

			m_Editor.reset(new Editor(m_InputManager, m_Renderer, m_Streaming, m_EntityManager, m_MapLoader));
			this->PushMessage(new SystemMessage(m_Editor));

			ScriptingEngine *manager = ScriptingEngine::getSingletonPtr();

			manager->RegisterGlobalObject("System system", this);
			manager->RegisterGlobalObject("StreamingManager streamer", m_Streaming);
			manager->RegisterGlobalObject("EntityManager entity_manager", m_EntityManager);
			manager->RegisterGlobalObject("Editor editor", m_Editor.get());

			m_EntityFactory->SetScriptingManager(manager, "main");
			m_EntityFactory->SetScriptedEntityPath("Entities/");

			ClientOptions gameOptions(L"gameconfig.xml", "gameconfig");

			m_PhysWorld = new PhysicalWorld();
			m_PhysWorld->SetGraphicContext(m_Renderer->GetGraphicContext());

			gameOptions.GetOption("startup_entity", &m_StartupEntity);
		}

		m_EntityFactory->LoadScriptedType(m_StartupEntity);
		m_EntityFactory->AddInstancer("Simple", EntityInstancerPtr(new SimpleInstancerTest()));

		// Load map entitites
		std::string maps, line;
		OpenString_PhysFS(maps, L"maps.txt"); // maps.txt contains a list of map files used by the game, seperated by newlines
		std::string::size_type lineBegin = 0, lineEnd;
		while (true)
		{
			if (lineBegin >= maps.length())
				break;

			lineEnd = maps.find("\n", lineBegin);
			if (lineEnd == std::string::npos)
				break;

			if (lineEnd != lineBegin+1) // ignore empty lines
			{
				line = fe_trim( maps.substr(lineBegin, lineEnd-lineBegin) ); // Get the line, trimming out the crap
				// Open the map file
				try
				{
					CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
					CL_IODevice mapFile = dir.open_file(fe_widen(line), CL_File::open_existing, CL_File::access_read);
					// Load entity types into the factory (the map loader has an EntityManager pointer
					//  from which it can get an EntityFactory pointer, on which it calls LoadScriptedType()
					//  for each entity found in the mapFile)
					m_MapLoader->LoadEntityTypes(mapFile);
				}
				catch (CL_Exception &)
				{
					SendToConsole("MapLoader - Reading maps.txt: Couldn't load " + line + ", file doesn't exist or can't be read.");
				}
			}

			lineBegin = lineEnd+1;
		}

		return true;
	}

	//PHYSFS_File *file = PHYSFS_openRead();
		//std::string buffer(256);
		//PHYSFS_sint64 count;
		//std::string currentLine
		//do
		//{
		//	count = PHYSFS_read(file, (void*)&buffer[0], 1, 256);
		//	std::string::size_type pos = 0;
		//	while (pos != std::string::npos)
		//	{
		//		pos = buffer.find("\n", pos);
		//	}
		//} while (count >= 256)

	void OntologicalSystem::CleanUp()
	{
		for (size_t i = 0; i < g_MaxLocalPlayers; ++i)
		{
			if (m_AddPlayerCallbacks[i].object != NULL)
			{
				m_AddPlayerCallbacks[i].object->Release();
				m_AddPlayerCallbacks[i].object = NULL;
			}
		}

		m_Viewports.clear();

		if (m_EntityManager != NULL)
		{
			delete m_MapLoader;
			delete m_EntityManager;
			delete m_EntityFactory;
			delete m_Streaming;
			delete m_EntitySyncroniser;

			delete m_PhysWorld;

			m_EntityManager = NULL;

			m_PhysWorld = NULL;
		}
	}

	void OntologicalSystem::Update(float split)
	{
		//m_EntitySyncroniser->BeginPacket();

		m_PhysWorld->Step(split);

		m_EntityManager->Update(split);

		//m_EntitySyncroniser->EndPacket();
		//m_EntitySyncroniser->Send();

		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			ViewportPtr &viewport = *it;
			const CameraPtr &camera = viewport->GetCamera();
			if (camera)
				camera->Update(split);
		}

		m_Streaming->Update();
	}

	void OntologicalSystem::Draw()
	{
		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			//m_Renderer->Draw(m_EntityManager->GetDomain(GAME_DOMAIN), *it, 0);
			m_EntityManager->Draw(m_Renderer, *it, 0);
		}
	}

	void OntologicalSystem::HandlePacket(IPacket *packet)
	{
		Network *network = m_NetworkSystem->GetNetwork();
		NetHandle originHandle = packet->GetSystemHandle();

		if (packet->GetType() == MTID_ADDPLAYER)
		{
			if (PlayerRegistry::ArbitratorIsLocal())
			{
				unsigned int playerIndex;
				{
					RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), false);
					bitStream.Read(playerIndex);
				}

				ObjectID netIndex = getNextPlayerIndex();
				PlayerRegistry::AddPlayer(netIndex, originHandle);

				RakNetHandle rakHandle = std::tr1::dynamic_pointer_cast<RakNetHandleImpl>(originHandle);
				{
					RakNet::BitStream bitStream;
					bitStream.Write1();
					bitStream.Write(netIndex);
					bitStream.Write(playerIndex);
					network->Send(
						false,
						MTID_ADDPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
						originHandle);
				}
				{
					SystemAddress systemAddress = rakHandle->Address;
					RakNetGUID guid = rakHandle->SystemIdent;

					RakNet::BitStream bitStream;
					bitStream.Write0();
					bitStream.Write(netIndex);
					bitStream.Write(systemAddress);
					bitStream.Write(guid);
					network->Send(
						false,
						MTID_ADDPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
						MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
						originHandle, true);
				}
			}
			else
			{
				RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), false);

				bool localPlayer = bitStream.ReadBit();
				ObjectID netIndex;
				bitStream.Read(netIndex);

				if (localPlayer)
				{
					unsigned int localIndex;
					bitStream.Read(localIndex);
					PlayerRegistry::AddPlayer(netIndex, localIndex);

					onGetNetIndex(localIndex, netIndex);
				}
				else
				{
					RakNetGUID guid;
					SystemAddress address;
					bitStream.Read(address);
					bitStream.Read(guid);

					RakNetHandle handle(new RakNetHandleImpl(guid, address));
					PlayerRegistry::AddPlayer(netIndex, handle);
				}
			}
		}

		else if (packet->GetType() == MTID_REMOVEPLAYER)
		{
			RakNet::BitStream bitStream((unsigned char*)packet->GetData(), packet->GetLength(), false);

			ObjectID netIndex;
			bitStream.Read(netIndex);

			if (PlayerRegistry::ArbitratorIsLocal())
			{
				PlayerRegistry::RemovePlayer(netIndex);
				releasePlayerIndex(netIndex);

				//network->Send(
				//	false,
				//	MTID_REMOVEPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
				//	MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
				//	originHandle, true);
			}
			else
			{
				PlayerRegistry::RemovePlayer(netIndex);
			}
		}
	}

	void OntologicalSystem::AddViewport(ViewportPtr viewport)
	{
		m_Viewports.push_back(viewport);
	}

	void OntologicalSystem::RemoveViewport(ViewportPtr viewport)
	{
		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			if (*it == viewport)
			{
				m_Viewports.erase(it);
				break;
			}
		}
	}

	void OntologicalSystem::RemoveAllViewports()
	{
		m_Viewports.clear();
	}

	OntologicalSystem::ViewportArray &OntologicalSystem::GetViewports()
	{
		return m_Viewports;
	}

	EntityManager * const OntologicalSystem::GetEntityManager()
	{
		return m_EntityManager;
	}

	void OntologicalSystem::SetModule(const ModulePtr &module)
	{
		m_Module = module;

		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&OntologicalSystem::OnModuleRebuild, this, _1) );
	}

	void OntologicalSystem::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			// Anything to do here?
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			EntityPtr entity = m_EntityManager->InstanceEntity(m_StartupEntity, "startup");

			if (entity)
			{
				entity->Spawn();
				// Force stream-in
				entity->StreamIn();
			}
		}
	}

	void OntologicalSystem::Quit()
	{
		PushMessage(new SystemMessage(SystemMessage::QUIT));
	}

	void OntologicalSystem::Save(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		CL_IODevice out = vdir.open_file(filename.c_str(), CL_File::create_always, CL_File::access_write);

		m_MapLoader->SaveGame(out);
	}

	void OntologicalSystem::Pause()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL & ~DS_ENTITYUPDATE);
	}

	void OntologicalSystem::Resume()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);
	}

	unsigned int OntologicalSystem::AddPlayer(asIScriptObject *callback_obj, const std::string &callback_decl)
	{
		int numPlayers;
		m_Options->GetOption("num_local_players", &numPlayers);
		if (numPlayers < g_MaxLocalPlayers)
		{
			unsigned int playerIndex = (unsigned)numPlayers;

			Network *network = m_NetworkSystem->GetNetwork();

			// Validate & store the callback method
			int fnId;
			if (callback_obj != NULL)
				fnId = callback_obj->GetObjectType()->GetMethodIdByName(callback_decl.c_str());
			else
				fnId = m_Module->GetASModule()->GetFunctionIdByDecl(callback_decl.c_str());

			asIScriptFunction *callback_fn = m_Module->GetASModule()->GetFunctionDescriptorById(fnId);
			if (callback_fn != NULL && callback_fn->GetParamCount() == 2 && callback_fn->GetParamTypeId(0) == asTYPEID_UINT16 && callback_fn->GetParamTypeId(1) == asTYPEID_UINT16)
			{
				//callback_obj->AddRef();
				// Note that fn->GetDecl...() is used here (rather than callback_decl), because
				//  this is definately a valid decl, whereas callback_decl may just be a fn. name
				m_AddPlayerCallbacks[playerIndex] = CallbackDecl(callback_obj, callback_fn->GetDeclaration(false));
			}
			else
				SendToConsole("system.addPlayer(): " + callback_decl + " is not a valid Add-Player callback - signature must be 'void (uint16, uint16)'");

			if (PlayerRegistry::ArbitratorIsLocal())
			{
				ObjectID netIndex = getNextPlayerIndex();

				// Add the player
				PlayerRegistry::AddPlayer(netIndex, playerIndex);

				// Call the script callback
				onGetNetIndex(playerIndex, netIndex);

				RakNet::BitStream bitStream;
				bitStream.Write0();
				bitStream.Write(netIndex);

				// Notify other systems 
				network->Send(
					false,
					MTID_ADDPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
					MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM, NetHandle(), true);
			}
			else
			{
				// Request a Net-Index if this peer isn't the arbitrator
				RakNet::BitStream bitStream;
				bitStream.Write(playerIndex);

				network->Send(
					false,
					MTID_ADDPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
					MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
					PlayerRegistry::GetArbitratingPlayer().System);
			}

			m_Options->SetOption("num_local_players", CL_StringHelp::int_to_local8(++numPlayers).c_str());

			return playerIndex;
		}

		else return g_MaxLocalPlayers;
	}

	void OntologicalSystem::RemovePlayer(unsigned int index)
	{
		Network *network = m_NetworkSystem->GetNetwork();

		const PlayerRegistry::PlayerInfo &playerInfo = PlayerRegistry::GetPlayerByLocalIndex(index);

		RakNet::BitStream bitStream;
		bitStream.Write(playerInfo.NetIndex);

		network->Send(
			false,
			MTID_REMOVEPLAYER, bitStream.GetData(), bitStream.GetNumberOfBytesUsed(),
			MEDIUM_PRIORITY, RELIABLE_ORDERED, CID_SYSTEM,
			NetHandle(new RakNetHandleImpl()), true);

		if (PlayerRegistry::ArbitratorIsLocal())
			releasePlayerIndex(playerInfo.NetIndex);
		PlayerRegistry::RemovePlayer(index);
	}

	int OntologicalSystem::GetScreenWidth() const
	{
		return m_Renderer->GetContextWidth();
	}

	int OntologicalSystem::GetScreenHeight() const
	{
		return m_Renderer->GetContextHeight();
	}

	void OntologicalSystem::EnablePhysicsDebugDraw(ViewportPtr viewport)
	{
		m_PhysWorld->EnableDebugDraw();
		m_PhysWorld->SetDebugDrawViewport(viewport);
	}

	void OntologicalSystem::DisablePhysicsDebugDraw()
	{
		m_PhysWorld->EnableDebugDraw(false);
	}


	void OntologicalSystem::onGetNetIndex(unsigned int local_idx, ObjectID net_idx)
	{
		// Call the script callback, if there is one
		CallbackDecl &decl = m_AddPlayerCallbacks[local_idx];
		if (!decl.method.empty())
		{
			ScriptUtils::Calling::Caller f;
			// If the object is null, it is implied that the callback is to a global method (or there's bug, but whatever...)
			if (decl.object == NULL)
				f = m_Module->GetCaller(decl.method); // Global method
			else
			{
				f = ScriptUtils::Calling::Caller(decl.object, decl.method.c_str()); // Object method
				decl.object->Release();
			}

			// Call the callback
			if (f.ok())
				f(local_idx, net_idx);

			decl.object = NULL;
			decl.method.clear();
		}
	}

	void OntologicalSystem::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<OntologicalSystem>("System", engine);
		r = engine->RegisterObjectMethod("System", "void quit()", asMETHOD(OntologicalSystem, Quit), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void save(const string &in)",
			asMETHOD(OntologicalSystem, Save), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"uint addPlayer(IEntity@, const string &in)",
			asMETHOD(OntologicalSystem, AddPlayer), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removePlayer(uint)",
			asMETHOD(OntologicalSystem, RemovePlayer), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"void addViewport(Viewport@)",
			asMETHOD(OntologicalSystem, AddViewport), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removeViewport(Viewport@)",
			asMETHOD(OntologicalSystem, RemoveViewport), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removeAllViewports()",
			asMETHOD(OntologicalSystem, RemoveAllViewports), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"int getScreenWidth()",
			asMETHOD(OntologicalSystem, GetScreenWidth), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"int getScreenHeight()",
			asMETHOD(OntologicalSystem, GetScreenHeight), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"void enablePhysicsDebugDraw(Viewport@)",
			asMETHOD(OntologicalSystem, EnablePhysicsDebugDraw), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void disablePhysicsDebugDraw()",
			asMETHOD(OntologicalSystem, DisablePhysicsDebugDraw), asCALL_THISCALL); FSN_ASSERT(r >= 0);
	}

	ObjectID OntologicalSystem::getNextPlayerIndex()
	{
		if (m_FreePlayerIndicies.empty())
			return m_NextPlayerIndex++;
		else
		{
			ObjectID freePlayerIndex = m_FreePlayerIndicies.front();
			m_FreePlayerIndicies.pop_front();
			return freePlayerIndex;
		}
	}

	void OntologicalSystem::releasePlayerIndex(ObjectID net_index)
	{
		if (m_NextPlayerIndex == 0 || net_index == m_NextPlayerIndex-1)
		{
			--m_NextPlayerIndex;
		}
		else
		{
			m_FreePlayerIndicies.push_back(net_index);
		}
	}

}
