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
#include "FusionPaths.h"
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

			m_PhysWorld = new PhysicalWorld();
			m_PhysWorld->SetGraphicContext(m_Renderer->GetGraphicContext());

			m_Streaming->SetRange(2000);

			m_Editor.reset(new Editor(m_InputManager, m_Renderer, m_EntityFactory, m_PhysWorld, m_Streaming, m_MapLoader));
			this->PushMessage(new SystemMessage(m_Editor));

			ScriptingEngine *manager = ScriptingEngine::getSingletonPtr();

			manager->RegisterGlobalObject("System system", this);
			manager->RegisterGlobalObject("StreamingManager streamer", m_Streaming);
			manager->RegisterGlobalObject("EntityManager entity_manager", m_EntityManager);
			manager->RegisterGlobalObject("Editor editor", m_Editor.get());

			m_EntityFactory->SetScriptingManager(manager);
			m_EntityFactory->SetScriptedEntityPath("Entities/");

			ClientOptions gameOptions(L"gameconfig.xml", "gameconfig");

			gameOptions.GetOption("startup_entity", &m_StartupEntity);
			gameOptions.GetOption("startup_map", &m_StartupMap);
		}

		if (!m_StartupEntity.empty())
			m_EntityFactory->LoadScriptedType(m_StartupEntity);

		// Load map entities
		//  Startup map (the map initially loaded)
		if (!m_StartupMap.empty())
		{
			CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
			m_MapLoader->LoadEntityTypes("Maps/" + m_StartupMap, dir);
		}
		//  maps.txt
		std::string maps, line;
		try {
			OpenString_PhysFS(maps, L"Maps/maps.txt"); // maps.txt contains a list of map files used by the game, seperated by newlines
		} catch (FileSystemException &e) {
			SendToConsole("Couldn't open maps.txt: If maps are not listed in Data/Maps/maps.txt they may fail to load.");
		}
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
					CL_IODevice mapFile = dir.open_file(fe_widen("Maps/" + line), CL_File::open_existing, CL_File::access_read);
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

		if (m_StartupEntity.empty() && m_StartupMap.empty())
		{
			m_EntityManager->GetFactory()->LoadAllScriptedTypes();
		}

		return true;
	}

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

			this->PushMessage(new SystemMessage(m_Editor, false)); // Remove the editor system
			m_Editor.reset();

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
		m_EntityFactory->SetModule(module);

		m_Module = module;

		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&OntologicalSystem::OnModuleRebuild, this, _1) );
	}

	void OntologicalSystem::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			if (!m_StartupEntity.empty())
			{
				EntityPtr entity = m_EntityManager->InstanceEntity(m_StartupEntity, "startup");
				if (entity)
				{
					entity->Spawn();
					// Force stream-in
					entity->StreamIn();
				}
			}

			if (!m_StartupMap.empty())
			{
				CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				m_MapLoader->LoadMap("Maps/" + m_StartupMap, dir);
			}

			if (m_StartupEntity.empty() && m_StartupMap.empty())
			{
				m_Editor->Enable();
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
		m_MapLoader->SaveGame(s_SavePath + filename, vdir);
	}

	void OntologicalSystem::Load(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		m_MapLoader->LoadSavedGame(s_SavePath + filename, vdir);
	}

	void OntologicalSystem::Pause()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL & ~DS_ENTITYUPDATE);
	}

	void OntologicalSystem::Resume()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);
	}

	bool OntologicalSystem::createScriptCallback(OntologicalSystem::CallbackDecl &out, asIScriptObject *callback_obj, const std::string &callback_decl)
	{
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
			out = CallbackDecl(callback_obj, callback_fn->GetDeclaration(false));
			return true;
		}
		else
			return false;
	}

	unsigned int OntologicalSystem::AddPlayer()
	{
		return AddPlayer(NULL, std::string());
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
			if (!callback_decl.empty() && createScriptCallback(m_AddPlayerCallbacks[playerIndex], callback_obj, callback_decl))
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

	void OntologicalSystem::SetAddPlayerCallback(asIScriptObject *callback_obj, const std::string &callback_decl)
	{
		createScriptCallback(m_AddAnyPlayerCallback, callback_obj, callback_decl);
	}

	void OntologicalSystem::SetAddPlayerCallback(unsigned int player, asIScriptObject *callback_obj, const std::string &callback_decl)
	{
		if (player < g_MaxLocalPlayers)
			createScriptCallback(m_AddPlayerCallbacks[player], callback_obj, callback_decl);
	}

	void OntologicalSystem::SetSplitScreenArea(const CL_Rectf &area)
	{
		if (m_SplitScreenArea != area)
			fitSplitScreenViewports();
		m_SplitScreenArea = area;
	}

	ViewportPtr OntologicalSystem::AddSplitScreenViewport(unsigned int player)
	{
		if (m_SplitScreenViewports.size() < s_MaximumSplitScreenViewports)
		{
			SplitScreenViewport splitPort(ViewportPtr(new Viewport()), player);
			m_SplitScreenViewports.push_back(splitPort);
			fitSplitScreenViewports();
			return splitPort.first;
		}
		else
			return ViewportPtr();
	}

	void OntologicalSystem::RemoveSplitScreenViewport(unsigned int player)
	{
		for (SplitScreenViewportArray::iterator it = m_SplitScreenViewports.begin(), end = m_SplitScreenViewports.end(); it != end; ++it)
		{
			if (it->second == player)
			{
				m_SplitScreenViewports.erase(it);
				break;
			}
		}
		fitSplitScreenViewports();
	}

	void OntologicalSystem::SetSplitScreenOrder(const PlayerOrderArray &player_order)
	{
		SplitScreenViewportArray sortedArray;
		for (PlayerOrderArray::const_iterator order_it = player_order.begin(), ord_end = player_order.end(); order_it != ord_end; ++order_it)
		{
			for (SplitScreenViewportArray::iterator it = m_SplitScreenViewports.begin(), end = m_SplitScreenViewports.end(); it != end; ++it)
			{
				if (it->second == *order_it)
					sortedArray.push_back(*it);
			}
		}
		m_SplitScreenViewports.swap(sortedArray);
		fitSplitScreenViewports();
	}

	void OntologicalSystem::SetSplitScreenOrder(asIScriptArray *player_order)
	{
		if (player_order->GetElementTypeId() == asTYPEID_UINT32)
		{
			// Copy from the script type to the application array type
			PlayerOrderArray appArray;
			for (size_t i = 0, end = player_order->GetElementCount(); i < end; ++i)
			{
				int *player = static_cast<int*>( player_order->GetElementPointer(i) );
				appArray[i] = *player;
			}
			SetSplitScreenOrder(appArray);
		}
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

	void OntologicalSystem_SetSplitScreenArea(float x, float y, float w, float h, OntologicalSystem *obj)
	{
		obj->SetSplitScreenArea(CL_Rectf(x, y, x+w, y+h));
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
			"void load(const string &in)",
			asMETHOD(OntologicalSystem, Load), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"uint addPlayer()",
			asMETHODPR(OntologicalSystem, AddPlayer, (void), size_t), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"uint addPlayer(IEntity@, const string &in)",
			asMETHODPR(OntologicalSystem, AddPlayer, (asIScriptObject*, const std::string&), size_t), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removePlayer(uint)",
			asMETHOD(OntologicalSystem, RemovePlayer), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"void setAddPlayerCallback(IEntity@, const string &in)",
			asMETHODPR(OntologicalSystem, SetAddPlayerCallback, (asIScriptObject*, const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void setAddPlayerCallback(uint, IEntity@, const string &in)",
			asMETHODPR(OntologicalSystem, SetAddPlayerCallback, (size_t, asIScriptObject*, const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"void setSplitScreenArea(float, float, float, float)",
			asFUNCTION(OntologicalSystem_SetSplitScreenArea), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"Viewport@ addSplitScreenViewport(uint)",
			asMETHOD(OntologicalSystem, AddSplitScreenViewport), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removeSplitScreenViewport(uint)",
			asMETHOD(OntologicalSystem, RemoveSplitScreenViewport), asCALL_THISCALL); FSN_ASSERT(r >= 0);

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

	inline void middle(float &mid, const float &from, const float &to)
	{
		mid = from + (to - from) * 0.5f;
	}

	void OntologicalSystem::fitSplitScreenViewports()
	{
		size_t viewports = m_SplitScreenViewports.size();

		if (viewports == 1)
		{
			ViewportPtr &viewport = m_SplitScreenViewports[0].first;
			viewport->SetArea(m_SplitScreenArea);
		}

		else if (viewports == 2)
		{
			ViewportPtr &viewport1 = m_SplitScreenViewports[0].first;
			ViewportPtr &viewport2 = m_SplitScreenViewports[1].first;

			if (m_SplitType == HorizontalFirst)
			{
				float midHoriz;
				middle(midHoriz, m_SplitScreenArea.left, m_SplitScreenArea.right);

				viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, midHoriz, m_SplitScreenArea.bottom);
				viewport2->SetArea(midHoriz, m_SplitScreenArea.top, m_SplitScreenArea.right, m_SplitScreenArea.bottom);
			}

			else // VerticalFirst
			{
				float midVert;
				middle(midVert, m_SplitScreenArea.top, m_SplitScreenArea.bottom);

				viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, m_SplitScreenArea.right, midVert);
				viewport2->SetArea(m_SplitScreenArea.left, midVert, m_SplitScreenArea.right, m_SplitScreenArea.bottom);
			}
		}

		else if (viewports == 3)
		{
			ViewportPtr &viewport1 = m_SplitScreenViewports[0].first;
			ViewportPtr &viewport2 = m_SplitScreenViewports[1].first;
			ViewportPtr &viewport3 = m_SplitScreenViewports[2].first;
			if (m_SplitType == HorizontalFirst)
			{
				float midHoriz, midVert;
				middle(midHoriz, m_SplitScreenArea.left, m_SplitScreenArea.right);
				middle(midVert, m_SplitScreenArea.top, m_SplitScreenArea.bottom);

				//  _ _
				// | |_|
				// |_|_|
				//

				viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, midHoriz, m_SplitScreenArea.bottom);
				viewport2->SetArea(midHoriz, m_SplitScreenArea.top, m_SplitScreenArea.right, midVert);
				viewport3->SetArea(midHoriz, midVert, m_SplitScreenArea.right, m_SplitScreenArea.bottom);
			}

			else if (m_SplitType == VerticalFirst)
			{
				float midHoriz, midVert;
				middle(midHoriz, m_SplitScreenArea.left, m_SplitScreenArea.right);
				middle(midVert, m_SplitScreenArea.top, m_SplitScreenArea.bottom);

				//  ___
				// |___|
				// |_|_|
				//

				viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, m_SplitScreenArea.right, midVert);
				viewport2->SetArea(m_SplitScreenArea.left, midVert, midHoriz, m_SplitScreenArea.bottom);
				viewport3->SetArea(midHoriz, midVert, m_SplitScreenArea.right, m_SplitScreenArea.bottom);
			}

			else // AlwaysQuaters
			{
				float midHoriz, midVert;
				middle(midHoriz, m_SplitScreenArea.left, m_SplitScreenArea.right);
				middle(midVert, m_SplitScreenArea.top, m_SplitScreenArea.bottom);

				//  _ _
				// |_|_|
				// |_|
				//

				viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, midHoriz, midVert);
				viewport2->SetArea(midHoriz, m_SplitScreenArea.top, m_SplitScreenArea.right, midVert);
				viewport3->SetArea(m_SplitScreenArea.left, midVert, midHoriz, m_SplitScreenArea.bottom);
			}
		}

		else if (viewports == 4)
		{
			ViewportPtr &viewport1 = m_SplitScreenViewports[0].first;
			ViewportPtr &viewport2 = m_SplitScreenViewports[1].first;
			ViewportPtr &viewport3 = m_SplitScreenViewports[2].first;
			ViewportPtr &viewport4 = m_SplitScreenViewports[3].first;

			float midHoriz, midVert;
			middle(midHoriz, m_SplitScreenArea.left, m_SplitScreenArea.right);
			middle(midVert, m_SplitScreenArea.top, m_SplitScreenArea.bottom);

			//  _ _
			// |_|_|
			// |_|_|
			//

			viewport1->SetArea(m_SplitScreenArea.left, m_SplitScreenArea.top, midHoriz, midVert);
			viewport2->SetArea(midHoriz, m_SplitScreenArea.top, m_SplitScreenArea.right, midVert);
			viewport3->SetArea(m_SplitScreenArea.left, midVert, midHoriz, m_SplitScreenArea.bottom);
			viewport4->SetArea(midHoriz, midVert, m_SplitScreenArea.right, m_SplitScreenArea.bottom);
		}
	}

}
