/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionOntologicalSystem.h"

#include <boost/lexical_cast.hpp>
#include <Bitstream.h>
#include <ScriptUtils/Calling/Caller.h>
#include <ScriptUtils/Inheritance/TypeTraits.h>

#include "FusionClientOptions.h"
#include "FusionEditor.h"
#include "FusionEntityManager.h"
#include "FusionEntityFactory.h"
#include "FusionGameMapLoader.h"
#include "FusionInputHandler.h"
#include "FusionInstanceSynchroniser.h"
#include "FusionNetworkSystem.h"
#include "FusionNetworkManager.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionPhysFSIODeviceProvider.h"
#include "FusionPhysicalEntityManager.h"
#include "FusionPlayerRegistry.h"
#include "FusionScriptManager.h"
#include "FusionRakNetwork.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionXml.h"

#include "FusionScriptedEntity.h"

#include "scriptarray.h"


namespace FusionEngine
{

	//class SimpleEntity : public PhysicalEntity
	//{
	//public:
	//	SimpleEntity(const std::string &name)
	//		: PhysicalEntity(name)
	//	{
	//		ScriptManager *manager = ScriptManager::getSingletonPtr();
	//		if (manager != NULL && manager->GetEnginePtr() != NULL)
	//			manager->GetEnginePtr()->NotifyGarbageCollectorOfNewObject(this, s_TypeId);
	//	}

	//	virtual ~SimpleEntity()
	//	{
	//		SendToConsole(GetName() + " was deleted");
	//	}

	//	virtual std::string GetType() const { return "Simple"; }

	//	virtual void OnSpawn()
	//	{
	//		SendToConsole(GetName() + " spawned");
	//	}
	//	virtual void Update(float split)
	//	{
	//	}

	//	virtual int GetPropertyType(unsigned int index) const
	//	{
	//		return pt_none;
	//	}

	//	virtual unsigned int GetPropertyArraySize(unsigned int index) const
	//	{
	//		return 0;
	//	}

	//	virtual void* GetAddressOfProperty(unsigned int index, unsigned int array_index) const
	//	{
	//		return NULL;
	//	}

	//	virtual void OnStreamIn()
	//	{
	//		SendToConsole(GetName() + " streamed in");
	//	}
	//	virtual void OnStreamOut()
	//	{
	//		SendToConsole(GetName() + " streamed out");
	//	}

	//	//! Save state to buffer
	//	virtual void SerialiseState(SerialisedData &state, bool local) const
	//	{
	//	}

	//	//! Read state from buffer
	//	virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
	//	{
	//		return 0;
	//	}
	//};

	//class SimpleInstancerTest : public EntityInstancer
	//{
	//public:
	//	SimpleInstancerTest()
	//		: EntityInstancer("Simple")
	//	{
	//	}

	//	//! Returns a Simple object
	//	virtual Entity *InstanceEntity(const std::string &name)
	//	{
	//		return new SimpleEntity(name);
	//	}
	//};

	const std::string s_OntologicalSystemName = "Entities";

	OntologicalSystem::OntologicalSystem(Renderer *renderer, InstancingSynchroniser *instance_sync, PhysicalWorld *phys, StreamingManager *streaming_manager, GameMapLoader *map_loader, EntityManager *entity_manager)
		: m_Renderer(renderer),
		m_InstancingSynchroniser(instance_sync),
		m_PhysWorld(phys),
		m_Streaming(streaming_manager),
		m_MapLoader(map_loader),
		m_EntityManager(entity_manager)
	{
		ScriptManager *manager = ScriptManager::getSingletonPtr();
		manager->RegisterGlobalObject("System system", this);

		m_PlayerAddedConnection = PlayerRegistry::ConnectToPlayerAdded(std::bind(&OntologicalSystem::onPlayerAdded, this, std::placeholders::_1));
		m_PlayerManager = new PlayerManager();
	}

	OntologicalSystem::~OntologicalSystem()
	{
		CleanUp();

		m_PlayerAddedConnection.disconnect();
		if (m_PlayerManager)
			delete m_PlayerManager;
		m_PlayerManager = nullptr;
	}

	const std::string &OntologicalSystem::GetName() const
	{
		return s_OntologicalSystemName;
	}

	bool OntologicalSystem::Initialise()
	{
		return true;
	}

	void OntologicalSystem::CleanUp()
	{
		//for (size_t i = 0; i < s_MaxLocalPlayers; ++i)
		//{
		//	if (m_AddPlayerCallbacks[i].object != NULL)
		//	{
		//		m_AddPlayerCallbacks[i].object->Release();
		//		m_AddPlayerCallbacks[i].object = NULL;
		//	}
		//}

		m_Viewports.clear();
	}

	void OntologicalSystem::Update(float split)
	{
		//m_EntitySynchroniser->BeginPacket();

		//m_PhysWorld->Step(split);

		//m_EntityManager->Update(split);

		//m_EntitySynchroniser->EndPacket();
		//m_EntitySynchroniser->Send();

		m_Streaming->Update();
	}

	void OntologicalSystem::Draw()
	{
		for (ViewportArray::iterator it = m_Viewports.begin(), end = m_Viewports.end(); it != end; ++it)
		{
			m_EntityManager->Draw(m_Renderer, *it, 0);
		}
	}

	void OntologicalSystem::HandlePacket(RakNet::Packet *packet)
	{
		RakNet::BitStream bitStream(packet->data, packet->length, false);
		unsigned char packetType;
		bitStream.Read(packetType);

		switch (packetType)
		{
		case ID_NEW_INCOMING_CONNECTION:
			if (m_Host.empty() || NetworkManager::ArbitratorIsLocal())
			{
			}
			break;
		case MTID_JOINSETUP:
			break;
		}
	}

	void OntologicalSystem::Clear()
	{
		RemoveAllViewports();
		m_EntityManager->Clear();
	}

	void OntologicalSystem::Start()
	{
		this->PushMessage(SystemMessage(SystemMessage::SHOW));
		this->PushMessage(SystemMessage(SystemMessage::RESUME));

		m_EntityManager->SetDomainState(SYSTEM_DOMAIN, DS_ENTITYUPDATE | DS_SYNCH);
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);

		if (!m_StartupMap.empty())
		{
			CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
			m_MapLoader->LoadMap("Maps/" + m_StartupMap, dir, m_InstancingSynchroniser);
		}

		NetworkManager::getSingleton().Subscribe(MTID_JOINSETUP, this);
	}

	void OntologicalSystem::Stop()
	{
		this->PushMessage(SystemMessage(SystemMessage::PAUSE));
		this->PushMessage(SystemMessage(SystemMessage::HIDE));
		NetworkManager::getSingleton().Unsubscribe(MTID_JOINSETUP, this);
		NetworkManager::getSingleton().GetNetwork()->Disconnect();
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

		//m_ModuleConnection.disconnect();
		//m_ModuleConnection = module->ConnectToBuild( std::bind(&OntologicalSystem::OnModuleRebuild, this, _1) );
	}

	void OntologicalSystem::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			ClientOptions gameOptions("gameconfig.xml", "gameconfig");

			//unsigned short port = boost::lexical_cast<unsigned short>(gameOptions.GetOption_str("port"));
			//if (!NetworkManager::getSingleton().GetNetwork()->Startup(port))
			//	AddLogEntry("Network", "Failed to start network", LOG_CRITICAL);

			gameOptions.GetOption("startup_map", &m_StartupMap);

			// Load map entities
			//  Startup map (the map initially loaded)
			if (!m_StartupMap.empty())
			{
				CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				m_MapLoader->LoadEntityTypes("Maps/" + m_StartupMap, dir);
			}

			gameOptions.GetOption("host", &m_Host);
			if (!m_Host.empty())
			{
				if (!NetworkManager::getSingleton().GetNetwork()->Startup(22888))
					AddLogEntry("Network", "Failed to start network", LOG_CRITICAL);

				NetworkManager::getSingleton().GetNetwork()->Connect(m_Host, 23505);
				m_StartupMap.clear();
			}
			else
			{
				if (!NetworkManager::getSingleton().GetNetwork()->Startup(23505))
					AddLogEntry("Network", "Failed to start network", LOG_CRITICAL);
			}
		}

		else if (ev.type == BuildModuleEvent::PostBuild)
		{
			
		}
	}

	void OntologicalSystem::Connect(const std::string& host, const unsigned short port)
	{
		// It's ok to connect immediately even if this is called by an Entity because the
		//  map wont be reloaded until the next step (ie. when received packets are processed)
		//  at the earliest
		NetworkManager::getSingleton().GetNetwork()->Connect(host, port);
	}

	void OntologicalSystem::Quit()
	{
		PushMessage(SystemMessage(SystemMessage::QUIT));
	}

	void OntologicalSystem::Save(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		m_MapLoader->SaveGame(s_SavePath + filename, vdir);
	}

	void OntologicalSystem::Load(const std::string &filename)
	{
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		m_MapLoader->LoadSavedGame(s_SavePath + filename, vdir, m_InstancingSynchroniser);
	}

	void OntologicalSystem::Pause()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL & ~DS_ENTITYUPDATE);
	}

	void OntologicalSystem::Resume()
	{
		m_EntityManager->SetDomainState(GAME_DOMAIN, DS_ALL);
	}

	//bool OntologicalSystem::createScriptCallback(OntologicalSystem::CallbackDecl &out, asIScriptObject *callback_obj, const std::string &callback_decl)
	//{
	//	int fnId;
	//	if (callback_obj != NULL)
	//		fnId = callback_obj->GetObjectType()->GetMethodIdByName(callback_decl.c_str());
	//	else
	//		fnId = m_Module->GetASModule()->GetFunctionIdByDecl(callback_decl.c_str());

	//	asIScriptFunction *callback_fn = m_Module->GetASModule()->GetFunctionDescriptorById(fnId);
	//	if (callback_fn != NULL && callback_fn->GetParamCount() == 2 && callback_fn->GetParamTypeId(0) == asTYPEID_UINT16 && callback_fn->GetParamTypeId(1) == asTYPEID_UINT16)
	//	{
	//		//callback_obj->AddRef();
	//		// Note that fn->GetDecl...() is used here (rather than callback_decl), because
	//		//  this is definately a valid decl, whereas callback_decl may just be a fn. name
	//		out = CallbackDecl(callback_obj, callback_fn->GetDeclaration(false));
	//		return true;
	//	}
	//	else
	//		return false;
	//}

	void OntologicalSystem::onPlayerAdded(const PlayerInfo& player_info)
	{
		m_EntityManager->OnPlayerAdded(player_info.LocalIndex, player_info.NetID);
		//auto call_callback = [this](CallbackDecl &callback_decl, const PlayerInfo& player_info)
		//{
		//	if (!callback_decl.method.empty())
		//	{
		//		ScriptUtils::Calling::Caller f;
		//		// If the object is null, it is implied that the callback is to a global method (or there's bug, but whatever...)
		//		if (callback_decl.object == NULL)
		//			f = m_Module->GetCaller(callback_decl.method); // Global method
		//		else
		//		{
		//			f = ScriptUtils::Calling::Caller(callback_decl.object, callback_decl.method.c_str()); // Object method
		//			callback_decl.object->Release();
		//		}

		//		// Call the callback
		//		if (f)
		//			f(player_info.LocalIndex, player_info.NetID);

		//		callback_decl.object = NULL;
		//		callback_decl.method.clear();
		//	}
		//};

		//call_callback(m_AddPlayerCallbacks[player_info.LocalIndex], player_info);
		//call_callback(m_AddAnyPlayerCallback, player_info);
	}

	char OntologicalSystem::AddNamedDomain(const std::string& name)
	{
		return 0;
	}

	unsigned int OntologicalSystem::AddPlayer()
	{
		return m_PlayerManager->RequestNewPlayer();
	}

	bool OntologicalSystem::AddPlayer(unsigned int index)
	{
		return m_PlayerManager->RequestNewPlayer(index);
	}

	void OntologicalSystem::RemovePlayer(unsigned int index)
	{
		m_PlayerManager->RemovePlayer(index);
	}

	void OntologicalSystem::RequestInstance(EntityPtr requester, bool synced, const std::string& type, const std::string& name, PlayerID owner)
	{
		m_InstancingSynchroniser->RequestInstance(requester, synced, type, name, Vector2::zero(), 0.f, owner);
	}

	void OntologicalSystem::RemoveInstance(EntityPtr entity)
	{
		m_InstancingSynchroniser->RemoveInstance(entity);
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

	void OntologicalSystem::SetSplitScreenOrder(CScriptArray* player_order)
	{
		if (player_order->GetElementTypeId() == asTYPEID_UINT32)
		{
			// Copy from the script type to the application array type
			PlayerOrderArray appArray;
			for (size_t i = 0, end = player_order->GetSize(); i < end; ++i)
			{
				int *player = static_cast<int*>( player_order->At(i) );
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
		//m_PhysWorld->EnableDebugDraw();
		//m_PhysWorld->SetDebugDrawViewport(viewport);
	}

	void OntologicalSystem::DisablePhysicsDebugDraw()
	{
		//m_PhysWorld->EnableDebugDraw(false);
	}


	//void OntologicalSystem::onGetNetIndex(unsigned int local_idx, ObjectID net_idx)
	//{
	//	// Call the script callback, if there is one
	//	CallbackDecl &decl = m_AddPlayerCallbacks[local_idx];
	//	if (!decl.method.empty())
	//	{
	//		ScriptUtils::Calling::Caller f;
	//		// If the object is null, it is implied that the callback is to a global method (or there's bug, but whatever...)
	//		if (decl.object == NULL)
	//			f = m_Module->GetCaller(decl.method); // Global method
	//		else
	//		{
	//			f = ScriptUtils::Calling::Caller(decl.object, decl.method.c_str()); // Object method
	//			decl.object->Release();
	//		}

	//		// Call the callback
	//		if (f)
	//			f(local_idx, net_idx);

	//		decl.object = NULL;
	//		decl.method.clear();
	//	}
	//}

	void OntologicalSystem_RequestInstance(bool synced, const std::string& type, const std::string& name, PlayerID owner, OntologicalSystem *obj)
	{
		EntityPtr requester;
		asIScriptContext* ctx = asGetActiveContext();
		//if (ctx != nullptr)
		//{
		//	asIObjectType* entityBaseType = ctx->GetEngine()->GetObjectTypeById(ScriptedEntity::s_ScriptEntityTypeId);;
		//	asIScriptObject* thisObj = static_cast<asIScriptObject*>( ctx->GetThisPointer() );
		//	if (ScriptUtils::Inheritance::is_base_of(entityBaseType, thisObj->GetObjectType()))
		//		requester = ScriptedEntity::GetAppObject(thisObj);
		//}
		obj->RequestInstance(requester, synced, type, name, owner);
	}

	void OntologicalSystem_RequestInstance(bool synced, const std::string& type, PlayerID owner, OntologicalSystem *obj)
	{
		OntologicalSystem_RequestInstance(synced, type, std::string(), owner, obj);
	}

	void OntologicalSystem_RequestInstance(bool synced, const std::string& type, const std::string& name, OntologicalSystem *obj)
	{
		OntologicalSystem_RequestInstance(synced, type, name, 0, obj);
	}

	void OntologicalSystem_RequestInstance(bool synced, const std::string& type, OntologicalSystem *obj)
	{
		OntologicalSystem_RequestInstance(synced, type, std::string(), 0, obj);
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
			"int8 addDomain(const string &in)",
			asMETHOD(OntologicalSystem, AddNamedDomain), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"uint addPlayer()",
			asMETHODPR(OntologicalSystem, AddPlayer, (void), unsigned int), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"bool addPlayer(uint)",
			asMETHODPR(OntologicalSystem, AddPlayer, (unsigned int), bool), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void removePlayer(uint)",
			asMETHOD(OntologicalSystem, RemovePlayer), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("System",
			"void requestInstance(Entity@, bool, const string &in, const string &in, uint8)",
			asMETHODPR(OntologicalSystem, RequestInstance, (EntityPtr, bool, const std::string&, const std::string&, PlayerID), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void requestInstance(bool, const string &in, const string &in, uint8)",
			asFUNCTIONPR(OntologicalSystem_RequestInstance, (bool, const std::string&, const std::string&, PlayerID, OntologicalSystem*), void), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void requestInstance(bool, const string &in, uint8)",
			asFUNCTIONPR(OntologicalSystem_RequestInstance, (bool, const std::string&, PlayerID, OntologicalSystem*), void), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void requestInstance(bool, const string &in, const string &in)",
			asFUNCTIONPR(OntologicalSystem_RequestInstance, (bool, const std::string&, const std::string&, OntologicalSystem*), void), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("System",
			"void requestInstance(bool, const string &in)",
			asFUNCTIONPR(OntologicalSystem_RequestInstance, (bool, const std::string&, OntologicalSystem*), void), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

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

	//ObjectID OntologicalSystem::getNextPlayerIndex()
	//{
	//	if (m_FreePlayerIndicies.empty())
	//		return m_NextPlayerIndex++;
	//	else
	//	{
	//		ObjectID freePlayerIndex = m_FreePlayerIndicies.front();
	//		m_FreePlayerIndicies.pop_front();
	//		return freePlayerIndex;
	//	}
	//}

	//void OntologicalSystem::releasePlayerIndex(ObjectID net_index)
	//{
	//	if (m_NextPlayerIndex == 0 || net_index == m_NextPlayerIndex-1)
	//	{
	//		--m_NextPlayerIndex;
	//	}
	//	else
	//	{
	//		m_FreePlayerIndicies.push_back(net_index);
	//	}
	//}

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
