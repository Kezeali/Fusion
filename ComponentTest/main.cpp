#include "../FusionEngine/FusionStableHeaders.h"
#include "../FusionEngine/FusionPrerequisites.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

#include "../FusionEngine/FusionProfiling.h"

// Filesystem
#include "../FusionEngine/FusionPaths.h"
#include "../FusionEngine/FusionPhysFS.h"
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"
#include "../FusionEngine/FusionPhysFSIOStream.h"

// Resource Loading
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"

#include "../FusionEngine/FusionClientOptions.h"

// Network
#include "../FusionEngine/FusionRakNetwork.h"
#include "../FusionEngine/FusionPacketDispatcher.h"
#include "../FusionEngine/FusionPlayerRegistry.h"
#include "../FusionEngine/FusionPlayerManager.h"
#include "../FusionEngine/FusionCameraSynchroniser.h"

// System management
#include "../FusionEngine/FusionTaskScheduler.h"

// Systems
#include "../FusionEngine/FusionGUI.h"

#include "../FusionEngine/FusionAngelScriptSystem.h"
#include "../FusionEngine/FusionBox2DSystem.h"
#include "../FusionEngine/FusionCLRenderSystem.h"

#include "../FusionEngine/FusionBox2DComponent.h"
#include "../FusionEngine/FusionPhysicalComponent.h"

#include "../FusionEngine/FusionScriptComponent.h"
#include "../FusionEngine/FusionRender2DComponent.h"

// Various
#include "../FusionEngine/FusionInputHandler.h"
#include "../FusionEngine/FusionNetworkManager.h"
#include "../FusionEngine/FusionScriptManager.h"

#include "../FusionEngine/FusionContextMenu.h"
#include "../FusionEngine/FusionElementUndoMenu.h"
#include "../FusionEngine/FusionEntityFactory.h"
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionEntitySynchroniser.h"
#include "../FusionEngine/FusionEntitySerialisationUtils.h"
#include "../FusionEngine/FusionExceptionFactory.h"
#include "../FusionEngine/FusionGameMapLoader.h"
#include "../FusionEngine/FusionInstanceSynchroniser.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionRegionCellCache.h"
#include "../FusionEngine/FusionRegionMapLoader.h"
#include "../FusionEngine/FusionRenderer.h"
#include "../FusionEngine/FusionScriptInputEvent.h"
#include "../FusionEngine/FusionScriptModule.h"
#include "../FusionEngine/FusionScriptSound.h"

#include "../FusionEngine/FusionAngelScriptComponent.h"

#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>

#include <numeric>

//#define FSN_REGISTER_PROP_ACCESSORA(iface, type, scriptType, prop) \
//	ThreadSafeProperty<type>::RegisterProp(engine, scriptType);\
//	engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "Property_" scriptType "_ &get_" #prop "()", asMETHOD(iface, get_ ## prop ), asCALL_THISCALL)

#define FSN_REGISTER_PROP_ACCESSORA(iface, type, scriptType, prop) \
	ThreadSafeProperty<iface, type>::RegisterProp(engine, scriptType);\
	engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const " scriptType " &get_" #prop "() const", asMETHOD(iface, get_ ## prop ), asCALL_THISCALL)

namespace FusionEngine
{

	template <class IFaceT>
	IFaceT* GetIface(void* obj)
	{
		auto ifaceObj = dynamic_cast<IFaceT*>(static_cast<IComponent*>(obj));
		//FSN_ASSERT_MSG(ifaceObj, "The given component doesn't implement the expected interface");
		if (ifaceObj)
			return ifaceObj;
		else
			return static_cast<IFaceT*>(obj);
	}

}

#define FSN_REGISTER_PROP_ACCESSOR(iface, type, scriptType, prop) \
	struct iface##_##prop { static ThreadSafeProperty<type>* get_ ## prop(void *obj) { return &GetIface<iface>(obj)->prop; } };\
	ThreadSafeProperty<type>::RegisterProp(engine, scriptType);\
	{int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "Property_" scriptType "_@+ get_" #prop "()", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	/*r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "void set_" #prop "(Property_" scriptType "_@+)", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);*/\
	FSN_ASSERT(r >= 0);}

#define FSN_REGISTER_PROP_ACCESSOR_R(iface, type, scriptType, prop) \
	struct iface##_##prop { static ThreadSafeProperty<type, NullWriter<type>> *get_ ## prop(void *obj) { return &GetIface<iface>(obj)->prop; } };\
	ThreadSafeProperty<type, NullWriter<type>>::RegisterProp(engine, scriptType);\
	{int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const ReadonlyProperty_" scriptType "_@+ get_" #prop "() const", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	FSN_ASSERT(r >= 0);}

//#define FSN_REGISTER_PROP_ACCESSOR(iface, type, scriptType, prop) \
//	struct iface##_##prop {\
//	static const type &get_ ## prop(void *obj) { auto com = GetIface<iface>(obj); return com->prop.Get(); }\
//	static void set_ ## prop(const type& value, void *obj) { return GetIface<iface>(obj)->prop.Set(value); } };\
//	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const " scriptType "&get_" #prop "() const", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
//	FSN_ASSERT(r >= 0);\
//	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "void set_" #prop "(const " scriptType " &in)", asFUNCTION(iface##_##prop :: set_ ## prop ), asCALL_CDECL_OBJLAST);\
//	FSN_ASSERT(r >= 0)
//
//#define FSN_REGISTER_PROP_ACCESSOR_R(iface, type, scriptType, prop) \
//	struct iface##_##prop { static const type &get_ ## prop(void *obj) { return GetIface<iface>(obj)->prop.Get(); } };\
//	engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const " scriptType " &get_" #prop "() const", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST)


namespace FusionEngine
{

	void ITransform::RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(ITransform, Vector2, "Vector", Position);
		FSN_REGISTER_PROP_ACCESSOR(ITransform, float, "float", Angle);		
		FSN_REGISTER_PROP_ACCESSOR(ITransform, int, "int", Depth);
		
		//struct iface_Angle {
		//	static ThreadSafeProperty<float> &get_Angle(ITransform &obj) { return obj.Angle; }
		//};
		//ThreadSafeProperty<float>::RegisterProp(engine, "float");
		//engine->RegisterObjectMethod(ITransform::GetTypeName().c_str(), "Property_float_ &get_Angle()", asFUNCTION(iface_Angle::get_Angle), asCALL_CDECL_OBJLAST);

		//struct iface_Angle {
		//	static const float &get_Angle(void* obj) {
		//		auto com = static_cast<IComponent*>(obj);
		//		auto transform = dynamic_cast<ITransform*>(com);
		//		FSN_ASSERT(transform);
		//		return transform->Angle.Get();
		//	}
		//};
		//ThreadSafeProperty<float>::RegisterProp(engine, "float");
		//engine->RegisterObjectMethod(ITransform::GetTypeName().c_str(), "const float &get_Angle()", asFUNCTION(iface_Angle::get_Angle), asCALL_CDECL_OBJLAST);
	}

	void IRigidBody::RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Interpolate);

		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, float, "float", Mass);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, float, "float", Inertia);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, Vector2, "Vector", CenterOfMass);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, Vector2, "Vector", Velocity);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", AngularVelocity);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", LinearDamping);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", AngularDamping);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, float, "float", GravityScale);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Active);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", SleepingAllowed);
		FSN_REGISTER_PROP_ACCESSOR_R(IRigidBody, bool, "bool", Awake);

		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", Bullet);
		FSN_REGISTER_PROP_ACCESSOR(IRigidBody, bool, "bool", FixedRotation);
	}

	void ICircleShape_RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(ICircleShape, Vector2, "Vector", Position);
		FSN_REGISTER_PROP_ACCESSOR(ICircleShape, float, "float", Radius);
	}

	void IRenderCom_RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(IRenderCom, Vector2, "Vector", Offset);
		FSN_REGISTER_PROP_ACCESSOR(IRenderCom, int, "int", LocalDepth);
	}

	void ISprite_RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(ISprite, std::string, "string", ImagePath);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, std::string, "string", AnimationPath);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, CL_Origin, "PointOrigin", AlignmentOrigin);
		//FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2i, "VectorInt", AlignmentOffset);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, CL_Origin, "PointOrigin", RotationOrigin);
		//FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2i, "VectorInt", RotationOffset);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, CL_Colorf, "Colour", Colour);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, float, "float", Alpha);

		FSN_REGISTER_PROP_ACCESSOR(ISprite, Vector2, "Vector", Scale);
		FSN_REGISTER_PROP_ACCESSOR(ISprite, float, "float", BaseAngle);

		FSN_REGISTER_PROP_ACCESSOR_R(ISprite, bool, "bool", AnimationFinished);
	}

	void IScript_RegisterScriptInterface(asIScriptEngine* engine)
	{
		FSN_REGISTER_PROP_ACCESSOR(IScript, std::string, "string", ScriptPath);
	}

// TODO: remove this
}
#include "../FusionEngine/FusionScriptTypeRegistrationUtils.h"
namespace FusionEngine
{
	void ICamera_RegisterScriptInterface(asIScriptEngine* engine)
	{
		engine->RegisterEnum("SyncType");
		engine->RegisterEnumValue("SyncType", "NoSync", ICamera::SyncTypes::NoSync);
		engine->RegisterEnumValue("SyncType", "Owned", ICamera::SyncTypes::NoSync);
		engine->RegisterEnumValue("SyncType", "Shared", ICamera::SyncTypes::NoSync);

		FSN_REGISTER_PROP_ACCESSOR(ICamera, ICamera::SyncTypes, "SyncType", SyncType);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, bool, "bool", ViewportEnabled);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, CL_Rectf, "Rect", ViewportRect);
		FSN_REGISTER_PROP_ACCESSOR(ICamera, bool, "bool", AngleEnabled);
	}

class ComponentTest
{
public:
	virtual int main(const std::vector<CL_String>& args)
	{
		CL_SetupCore core_setup;
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;
		CL_SetupSound sound_setup;
		CL_SetupVorbis voirbis_setup;

		CL_SoundOutput sound_output(44100);

		SetupPhysFS physfs(CL_System::get_exe_path().c_str());
		if (!SetupPhysFS::is_init())
			return 1;

		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
			("connect", po::value<std::string>()->multitoken(), "Connect to a host.")
			("listen_port", po::value<unsigned short>(), "Set the listen port.")
			;

		po::variables_map varMap;
		po::store(po::command_line_parser(std::vector<std::string>(args.begin(), args.end())).options(desc).run(), varMap);
		po::notify(varMap);

		CL_ConsoleWindow conWindow("Component Test Console", 80, 10);
		CL_DisplayWindow dispWindow("Component Test", 800, 600, false, true);

		{
			std::unique_ptr<Logger> logger;
			std::unique_ptr<ConsoleStdOutWriter> coutWriter;
			std::unique_ptr<Console> console;
			std::unique_ptr<Profiling> profiling;

			try
			{
				console.reset(new Console());
				coutWriter.reset(new ConsoleStdOutWriter());
				coutWriter->Enable();
				logger.reset(new Logger());

				profiling.reset(new Profiling());
			}
			catch (FusionEngine::Exception& ex)
			{
#ifdef _DEBUG
				CL_Console::write_line( CL_String(ex.ToString().c_str()) );
				conWindow.display_close_message();
#else
				//TODO: Show a OS native GUI messagebox in Release builds
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), ex.what(), "Exception", MB_OK);
#endif
#endif
			}
			catch (CL_Exception &ex)
			{
#ifdef _DEBUG
				CL_Console::write_line( ex.message );
				CL_Console::write_line( "Stack Trace:" );
				std::vector<CL_String> stack = ex.get_stack_trace();
				for (std::vector<CL_String>::iterator it = stack.begin(), end = stack.end(); it != end; ++it)
				{
					CL_Console::write_line(*it);
				}
				conWindow.display_close_message();
#else
				//TODO: Show a OS native GUI messagebox in Release builds
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), ex.what(), "Exception", MB_OK);
#endif
#endif
			}

			try
			{
				auto gc = dispWindow.get_gc();

				CL_OpenGL::set_active(NULL);

				////////////////////
				// Configure PhysFS
				SetupPhysFS::configure("lastflare", "Fusion", "7z");
				if (!SetupPhysFS::mount(s_PackagesPath, "", "7z", false))
					SendToConsole("Default resource path could not be located");
				SetupPhysFS::mount_archives(s_PackagesPath, "", "zip", false);
//#ifdef _DEBUG
				// Clear cache
				SetupPhysFS::clear_temp();
//#endif

				//logger->ActivateConsoleLogging();

				////////////////////
				// Script Manager
				auto scriptManager = std::make_shared<ScriptManager>();
				asIScriptEngine* asEngine = scriptManager->GetEnginePtr();

				Console::Register(scriptManager.get());
				RegisterScriptedConsoleCommand(asEngine);
				GUI::Register(scriptManager.get());
				ContextMenu::Register(asEngine);
				
				asEngine->RegisterTypedef("PlayerID", "uint8");

				ScriptInputEvent::Register(asEngine);

				{
				int r = asEngine->RegisterGlobalFunction("bool isLocal(PlayerID)",
					asFUNCTION(PlayerRegistry::IsLocal), asCALL_CDECL);
				FSN_ASSERT(r >= 0);
				}

				RegisterValueType<CL_Rectf>("Rect", asEngine, asOBJ_APP_CLASS_CK);
				struct ScriptRect
				{
					static void CTOR1(float left, float top, float right, float bottom, void* ptr)
					{
						new (ptr) CL_Rectf(left, top, right, bottom);
					}
				};
				asEngine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(ScriptRect::CTOR1), asCALL_CDECL_OBJLAST);

				// Component types
				IComponent::RegisterType<IComponent>(asEngine, "IComponent");
				asEngine->RegisterObjectMethod("IComponent", "string getType()", asMETHOD(IComponent, GetType), asCALL_THISCALL);

				RegisterComponentInterfaceType<ITransform>(asEngine);
				ITransform::RegisterScriptInterface(asEngine);
				RegisterComponentInterfaceType<IRigidBody>(asEngine);
				IRigidBody::RegisterScriptInterface(asEngine);
				RegisterComponentInterfaceType<IFixture>(asEngine);
				RegisterComponentInterfaceType<ICircleShape>(asEngine);
				ICircleShape_RegisterScriptInterface(asEngine);
				RegisterComponentInterfaceType<IPolygonShape>(asEngine);

				RegisterComponentInterfaceType<ISprite>(asEngine);
				ISprite_RegisterScriptInterface(asEngine);

				RegisterComponentInterfaceType<IScript>(asEngine);
				//IScript_RegisterScriptInterface(asEngine);

				RegisterComponentInterfaceType<ICamera>(asEngine);
				ICamera_RegisterScriptInterface(asEngine);

				// Entity generation
				Entity::Register(asEngine);
				ASScript::ScriptInterface::Register(asEngine);
				InstancingSynchroniser::Register(asEngine);

				Camera::Register(asEngine);
				StreamingManager::Register(asEngine);

				CLRenderWorld::Register(asEngine);

				// Console singleton
				scriptManager->RegisterGlobalObject("Console console", Console::getSingletonPtr());

				/////////////////////////////////////
				// Script SoundOutput wrapper object
				auto script_SoundOutput = std::make_shared<SoundOutput>(sound_output);

				/////////////////////////
				// Load optional settings (set options, that is, options that have been set)
				ClientOptions* options = new ClientOptions("settings.xml", "settings");
				
				auto loggingLevel = options->GetOption_str("logging_level");
				if (loggingLevel.empty() || loggingLevel == "normal")
					logger->SetDefaultThreshold(LOG_NORMAL);
				else if (loggingLevel == "info")
					logger->SetDefaultThreshold(LOG_INFO);
				else if (loggingLevel == "trivial")
					logger->SetDefaultThreshold(LOG_TRIVIAL);
				else if (loggingLevel == "critical")
					logger->SetDefaultThreshold(LOG_CRITICAL);

				if (varMap.count("connect") == 0 && options->GetOption_bool("console_logging"))
					logger->ActivateConsoleLogging();

				bool editMode = options->GetOption_bool("edit");

				////////////////////
				// Resource Manager
				boost::scoped_ptr<ResourceManager> resourceManager(new ResourceManager(gc));
				resourceManager->AddResourceLoader("IMAGE", &LoadImageResource, &UnloadImageResource, NULL);
				resourceManager->AddResourceLoader(ResourceLoader("TEXTURE", &LoadTextureResource, &UnloadTextureResource, &LoadTextureResourceIntoGC));
				resourceManager->AddResourceLoader(ResourceLoader("ANIMATION", &LoadAnimationResource, &UnloadAnimationResource));
				resourceManager->AddResourceLoader("AUDIO", &LoadAudio, &UnloadAudio, NULL);
				resourceManager->AddResourceLoader("AUDIO:STREAM", &LoadAudioStream, &UnloadAudio, NULL); // Note that this intentionally uses the same unload method

				resourceManager->AddResourceLoader("SPRITE", &LoadSpriteResource, &UnloadSpriteResource, NULL);

				resourceManager->AddResourceLoader("MODULE", &LoadScriptResource, &UnloadScriptResource, NULL);

				resourceManager->StartLoaderThread();

				CL_OpenGL::set_active(gc);

				/////////////////
				// Input Manager
				const std::unique_ptr<InputManager> inputMgr(new InputManager(dispWindow));

				if (!inputMgr->Test())
					FSN_EXCEPT(FileSystemException, "InputManager couldn't find a keyboard device.");
				inputMgr->Initialise();
				SendToConsole("Input manager started successfully");

				////////////
				// Renderer
				//const std::unique_ptr<Renderer> renderer(new Renderer(gc));

				///////////////////
				// Player Registry
				std::shared_ptr<PlayerRegistry> playerRegistry = std::make_shared<PlayerRegistry>();

				// Network
				std::unique_ptr<RakNetwork> network(new RakNetwork());
				std::unique_ptr<PacketDispatcher> packetDispatcher(new PacketDispatcher());
				std::unique_ptr<NetworkManager> networkManager(new NetworkManager(network.get(), packetDispatcher.get()));

				std::unique_ptr<PlayerManager> playerManager(new PlayerManager());

				std::unique_ptr<RegionMapLoader> cellArchivist(new RegionMapLoader(editMode));
				std::unique_ptr<StreamingManager> streamingMgr(new StreamingManager(cellArchivist.get()));

				std::unique_ptr<CameraSynchroniser> cameraSynchroniser(new CameraSynchroniser(streamingMgr.get()));

				// Entity management / instantiation
				std::unique_ptr<EntityFactory> entityFactory(new EntityFactory());
				std::unique_ptr<EntitySynchroniser> entitySynchroniser(new EntitySynchroniser(inputMgr.get(), cameraSynchroniser.get(), streamingMgr.get()));
				
				std::unique_ptr<EntityManager> entityManager(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get(), entityFactory.get(), cellArchivist.get()));
				std::unique_ptr<InstancingSynchroniser> instantiationSynchroniser(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));

				try
				{
				std::unique_ptr<GameMapLoader> mapLoader(new GameMapLoader(options));

				cellArchivist->SetSynchroniser(instantiationSynchroniser.get());

				scriptManager->RegisterGlobalObject("StreamingManager streaming", streamingMgr.get());

				// Component systems
				const std::unique_ptr<TaskManager> taskManager(new TaskManager());
				const std::unique_ptr<TaskScheduler> scheduler(new TaskScheduler(taskManager.get(), entityManager.get(), cellArchivist.get()));

#ifdef PROFILE_BUILD
				scheduler->SetFramerateLimiter(false);
				scheduler->SetUnlimited(true);
#else
				scheduler->SetFramerateLimiter(false);
				int maxFrameskip = 0;
				if (options->GetOption("max_frameskip", &maxFrameskip) && maxFrameskip >= 0)
					scheduler->SetMaxFrameskip((unsigned int)maxFrameskip);
#endif

				std::vector<std::shared_ptr<ISystemWorld>> ontology;

				const std::unique_ptr<CLRenderSystem> clRenderSystem(new CLRenderSystem(gc, cameraSynchroniser.get()));
				auto renderWorld = clRenderSystem->CreateWorld();
				ontology.push_back(renderWorld);

				entityFactory->AddInstancer(renderWorld);

				scriptManager->RegisterGlobalObject("Renderer renderer", renderWorld.get());
				
				const std::unique_ptr<IComponentSystem> box2dSystem(new Box2DSystem());
				auto box2dWorld = box2dSystem->CreateWorld();
				ontology.push_back(box2dWorld);

				entityFactory->AddInstancer(box2dWorld);
				
				static_cast<CLRenderWorld*>(renderWorld.get())->SetPhysWorld(static_cast<Box2DWorld*>(box2dWorld.get())->Getb2World());
				static_cast<CLRenderWorld*>(renderWorld.get())->SetDebugDraw(false);

				const std::unique_ptr<AngelScriptSystem> asSystem(new AngelScriptSystem(scriptManager, entityFactory.get()));
				auto asWorld = asSystem->CreateWorld();
				ontology.push_back(asWorld);

				// TODO: add some sort of Init method, to be called by the scheduler (?) when the ontology is set (?)
				static_cast<AngelScriptWorld*>(asWorld.get())->BuildScripts();

				entityFactory->AddInstancer(asWorld);

				scheduler->SetOntology(ontology);


				PropChangedQueue &propChangedQueue = entityManager->m_PropChangedQueue;

				// Load the map
				std::shared_ptr<GameMap> map;
				if (!editMode && varMap.count("connect") == 0)
				{
					map = mapLoader->LoadMap("Maps/default.gad", instantiationSynchroniser.get());
					cellArchivist->SetMap(map);

					streamingMgr->Initialise(map->GetCellSize());

					map->LoadNonStreamingEntities(true, entityManager.get(), entityFactory.get(), instantiationSynchroniser.get());
				}
				// Start the asynchronous cell loader
				cellArchivist->Start();

				// This scope makes viewport hold the only reference to camera: thus camera will be deleted with viewport
				std::shared_ptr<Camera> editCam;
				if (editMode)
				{
					auto camera = std::make_shared<Camera>();
					camera->SetPosition(0.f, 0.f);
					auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
					dynamic_cast<CLRenderWorld*>(renderWorld.get())->AddViewport(viewport);
					streamingMgr->AddCamera(camera);
				
					editCam = camera;
				}

				std::vector<EntityPtr> entities;
				bool compile = false;
				bool load = false;

				auto keyhandlerSlot = dispWindow.get_ic().get_keyboard().sig_key_up().connect_functor([&](const CL_InputEvent& ev, const CL_InputState&)
				{
					if (ev.id >= CL_KEY_0 && ev.id <= CL_KEY_9)
					{
						auto vps = dynamic_cast<CLRenderWorld*>(renderWorld.get())->GetViewports();
						if (vps.empty())
							return;
						auto vp = vps.front();

						Vector2 pos((float)ev.mouse_pos.x, (float)ev.mouse_pos.y);
						CL_Rectf area;
						Renderer::CalculateScreenArea(gc, area, vp, true);
						pos.x += area.left; pos.y += area.top;

						const bool addToScene = !ev.ctrl;
						unsigned int repeats = 1;
						const float size = 100.f;
						float rightEdge = 0;
						if (ev.shift)
						{
							repeats = 50;
							rightEdge = pos.x + (repeats / 2) * size;
							pos.x -= (repeats / 2) * size;
							pos.y -= (repeats / 2) * size;
						}
						for (unsigned int i = 0; i < repeats * repeats; ++i)
						{
							auto entity =
								createEntity(addToScene, (unsigned int)(ev.id - CL_KEY_0), pos, instantiationSynchroniser.get(), entityFactory.get(), entityManager.get());
							if (entity && entity->GetDomain() == SYSTEM_DOMAIN)
								entities.push_back(entity);
							pos.x += size;
							if (pos.x > rightEdge)
							{
								pos.x -= repeats * size;
								pos.y += size;
							}
						}
					}

					if (ev.id == CL_KEY_S)
					{
						compile = true;
					}

					if (ev.id == CL_KEY_L)
					{
						load = true;
					}

					if (ev.id == CL_KEY_I)
					{
						std::stringstream str;
						str << network->IsConnected();
						SendToConsole(str.str());
					}

					bool dtup = ev.id == CL_KEY_PRIOR;
					bool dtdown = ev.id == CL_KEY_NEXT;
					if (dtup || dtdown)
					{
						unsigned int fps = (unsigned int)(1.0f / scheduler->GetDT() + 0.5f);
						if (dtdown && fps <= 5)
							fps -= 1;
						else if (dtup && fps < 5)
							fps += 1;
						else
							fps += (dtup ? 5 : -5);
						fe_clamp(fps, 1u, 120u);
						scheduler->SetDT(1.0f / (float)fps);
					}

					bool rangeup = ev.id == CL_KEY_HOME;
					bool rangedown = ev.id == CL_KEY_END;
					if (rangeup || rangedown)
					{
						unsigned int range = (unsigned int)(streamingMgr->GetRange() + 0.5f);
						if (rangedown && range <= 4)
							range -= 1;
						else if (rangeup && range < 4)
							range += 1;
						else
							range += (rangeup ? 2 : -2);
						fe_clamp(range, 1u, 64u);
						streamingMgr->SetRange((float)range);
					  std::stringstream str;
					  str << range;
						SendToConsole(str.str());
					}

					if (ev.id == CL_KEY_DECIMAL)
					{
						network->SetDebugLag(60, 4);
					}
					if (ev.id == CL_KEY_NUMPAD3)
					{
						network->SetDebugLag(100, 10);
					}
					if (ev.id == CL_KEY_NUMPAD6)
					{
						network->SetDebugLag(150, 50);
					}
					if (ev.id == CL_KEY_NUMPAD9)
					{
						network->SetDebugLag(200, 100);
					}
					if (ev.id == CL_KEY_NUMPAD8)
					{
						network->SetDebugPacketLoss(0.1f);
					}
					if (ev.id == CL_KEY_NUMPAD2)
					{
						network->SetDebugPacketLoss(0.05f);
					}
					if (ev.id == CL_KEY_NUMPAD1)
					{
						network->SetDebugPacketLoss(0.005f);
					}
					if (ev.id == CL_KEY_NUMPAD5)
					{
						network->SetDebugLag(0, 0);
					}
					if (ev.id == CL_KEY_NUMPAD5)
					{
						network->SetDebugPacketLoss(0);
					}

					if (ev.id == CL_KEY_F3)
						std::dynamic_pointer_cast<CLRenderWorld>(renderWorld)->ToggleDebugDraw();

					if (ev.id == CL_KEY_F4)
					{
						entitySynchroniser->SetUseJitterBuffer(!entitySynchroniser->IsUsingJitterBuffer());
						SendToConsole(std::string("Jitter buffer") + (entitySynchroniser->IsUsingJitterBuffer() ? " enabled" : " disabled"));
					}
				});

				{
					unsigned short listenPort = 11122;
					if (varMap.count("listen_port"))
						listenPort = varMap["listen_port"].as<unsigned short>();
					else
					{
						int opt;
						if (options->GetOption("listen_port", &opt))
							listenPort = (unsigned short)opt;
					}
					network->Startup(listenPort);
				}

				bool connecting = false;
				{
					unsigned short port = 11123;
					std::string host = "localhost";
					if (varMap.count("connect") || options->GetOption("connect", &host))
					{
						if (varMap.count("connect"))
							host = varMap["connect"].as<std::string>();
						if (host.find(':') != std::string::npos)
						{
							auto portStr = host.substr(host.find(':') + 1);
							std::stringstream strStr(portStr);
							strStr >> port;
							host.erase(host.find(':'));
						}
						networkManager->SetHosting(false);
						network->Connect(host, port);
						connecting = true;
						SendToConsole("Connecting: " + host);
					}
					else
					{
						SendToConsole("Hosting");
						auto playerInd = playerManager->RequestNewPlayer();
						playerInd = playerManager->RequestNewPlayer();
						std::stringstream str; str << playerInd;
						SendToConsole("Players: " + str.str());
					}
				}

				unsigned int lastframe = CL_System::get_time();
				unsigned int delta = 0;
				float seconds = 0.f;

				CL_Font debugFont(gc, "Arial", 10);

				bool keepGoing = true;
				while (keepGoing)
				{
					auto timeNow = CL_System::get_time();
					delta = timeNow - lastframe;
					lastframe = timeNow;

					CL_KeepAlive::process();

					if (connecting && network->IsConnected() && network->GetHost() != RakNet::UNASSIGNED_RAKNET_GUID)
					{
						connecting = false;
						playerManager->RequestNewPlayer();
						std::string hostGUID(network->GetHost().ToString());
						SendToConsole(hostGUID);
					}

					if (compile)
					{
						compile = false;

						if (editMode)
						{
							streamingMgr->StoreAllCells(false);
							cellArchivist->Stop();
							try
							{
								IO::PhysFSStream file("default.gad", IO::Write);
								GameMap::CompileMap(file, streamingMgr->GetCellSize(), cellArchivist->GetCellCache(), entities);
								cellArchivist->SaveEntityLocationDB("default.endb");
							}
							catch (FileSystemException& e)
							{
								SendToConsole("Failed to compile map: " + e.GetDescription());
							}
							cellArchivist->Start();

							streamingMgr->Update(true);
						}
						else
						{
							SendToConsole("Preparing to save...");
							streamingMgr->StoreAllCells();
							cellArchivist->Stop();
							try
							{
								if (auto file = cellArchivist->CreateDataFile("active_entities"))
									entityManager->SaveActiveEntities(*file);
								if (auto file = cellArchivist->CreateDataFile("non_streaming_entities"))
									entityManager->SaveNonStreamingEntities(*file);

								entityManager->SaveCurrentReferenceData();
								
								cellArchivist->EnqueueQuickSave("quicksave");
							}
							catch (std::ios::failure& e)
							{
								AddLogEntry(e.what());
							}
							// The first thing the archivist does when it restarts will be perform the save
							cellArchivist->Start();
						}
					}
					if (load)
					{
						load = false;
						if (!editMode)
						{
							SendToConsole("Loading: Stopping archivist...");
							cellArchivist->Stop();
							SendToConsole("Loading: Deactivating entities...");
							entityManager->DeactivateAllEntities(false);
							cameraSynchroniser->Clear();
							SendToConsole("Loading: Clearing entities...");
							//entityManager.reset(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get(), entityFactory.get(), cellArchivist.get()));
							//instantiationSynchroniser.reset(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));
							//cellArchivist->SetSynchroniser(instantiationSynchroniser.get());
							//propChangedQueue = entityManager->m_PropChangedQueue;
							entityManager->Clear();
							instantiationSynchroniser->Reset();
							SendToConsole("Loading: Clearing cells...");
							streamingMgr->Reset();
							SendToConsole("Loading: Garbage-collecting...");
							scriptManager->GetEnginePtr()->GarbageCollect();
							SendToConsole("Loading: Non-streaming entities (from map)...");
							map->LoadNonStreamingEntities(false, entityManager.get(), entityFactory.get(), instantiationSynchroniser.get());
							cellArchivist->Load("quicksave");
							SendToConsole("Loading: Non-streaming entities (from data-file)...");
							if (auto file = cellArchivist->LoadDataFile("non_streaming_entities"))
								entityManager->LoadNonStreamingEntities(*file, instantiationSynchroniser.get());
							cellArchivist->Start();
							SendToConsole("Loading: Activating entities...");
							if (auto file = cellArchivist->LoadDataFile("active_entities"))
								entityManager->LoadActiveEntities(*file);
							// TODO: allow the entity manager to pause the simulation until all these entities are active
						}
					}

					resourceManager->UnloadUnreferencedResources();
					resourceManager->DeliverLoadedResources();

					if (dispWindow.get_ic().get_keyboard().get_keycode(VK_OEM_4))
					{
						// Accumulator
						scheduler->SetFramerateLimiter(false);
						scheduler->SetUnlimited(false);
					}
					if (dispWindow.get_ic().get_keyboard().get_keycode(VK_OEM_6))
						scheduler->SetFramerateLimiter(true);
					if (dispWindow.get_ic().get_keyboard().get_keycode(VK_OEM_5))
					{
						// Profile mode
						scheduler->SetFramerateLimiter(false);
						scheduler->SetUnlimited(true);
					}

					if (delta <= 1000)
					{
						seconds = delta * 0.001f;
						inputMgr->Update(seconds);
						//gui->Update(seconds);
						networkManager->DispatchPackets();

						if (editCam)
						{
							auto camPos = editCam->GetPosition();
							if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP))
								camPos.y -= 400 * seconds;
							if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN))
								camPos.y += 400 * seconds;
							if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT))
								camPos.x -= 400 * seconds;
							if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT))
								camPos.x += 400 * seconds;

							editCam->SetPosition(camPos.x, camPos.y);
						}
					}
					
					const auto executed = scheduler->Execute((editMode || connecting) ? SystemType::Rendering : (SystemType::Rendering | SystemType::Simulation));

					if (executed & SystemType::Rendering)
					{
#ifdef PROFILE_BUILD
						dispWindow.flip(0);
#else
						dispWindow.flip(0);
#endif
						gc.clear();
					}
					
					if (executed & SystemType::Simulation || editMode)
					{
						// Actually activate / deactivate components
						entityManager->ProcessActivationQueues();
						entitySynchroniser->ProcessQueue(entityManager.get(), entityFactory.get());
					}

					// Propagate property changes
					// TODO: throw if properties are changed during Rendering step?
					PropChangedQueue::value_type changed;
					while (propChangedQueue.try_pop(changed))
					{
						auto com = changed.first.lock();
						if (com)
						{
							changed.second->Synchronise();
							changed.second->FireSignal();
						}
					}

					Profiling::getSingleton().AddTime("~Buffer size", 0.0);
					Profiling::getSingleton().AddTime("~Incomming Packets", 0.0);
					Profiling::getSingleton().AddTime("~Packets Processed", 0.0);
					
					profiling->AddTime("ActualDT", delta);
					// Record profiling data
					profiling->StoreTick();

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_ESCAPE))
						keepGoing = false;
				}
				propChangedQueue.clear();
				asWorld.reset();
				cameraSynchroniser.reset();
				entityManager->Clear();
				streamingMgr.reset();
				cellArchivist->Stop();
				entityManager.reset();
				
				entitySynchroniser.reset();
				scriptManager->GetEnginePtr()->GarbageCollect();
				}
				catch (...)
				{
					cellArchivist->Stop();
					throw;
				}
				//cellArchivist->Stop();
				entityFactory.reset();
				scriptManager->GetEnginePtr()->GarbageCollect();
			}
			catch (FusionEngine::Exception &ex)
			{
#ifdef _DEBUG
				if (logger)
					logger->Add(ex.ToString());
#endif
				//TODO: Show a OS native GUI messagebox
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), ex.what(), "Exception", MB_OK);
#endif
			}
			catch (CL_Exception &ex)
			{
				if (logger)
				{
					logger->Add(ex.message);
#ifdef _DEBUG
					// Log a stack trace in DEBUG builds
					std::string stackTrace;
					stackTrace += "Stack Trace:\n  ";
					std::vector<CL_String> stack = ex.get_stack_trace();
					for (std::vector<CL_String>::iterator it = stack.begin(), end = stack.end(); it != end; ++it)
					{
						stackTrace += *it;
						stackTrace += "\n  ";
					}
					logger->Add(stackTrace);
#endif
				}
				//TODO: Show a OS native GUI messagebox
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), ex.what(), "Exception", MB_OK);
#endif
			}
#ifndef _DEBUG
			catch(...)
			{
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), "Unknown error", "Exception", MB_OK);
#endif
			}
#endif

		}

		return 0;
	}

	EntityPtr createEntity(bool add_to_scene, unsigned int i, Vector2 position, InstancingSynchroniser* instantiationSynchroniser, EntityFactory* factory, EntityManager* entityManager)
	{
		position.x = ToSimUnits(position.x); position.y = ToSimUnits(position.y);

		ComponentPtr transformCom;
		if (i == 1 || i == 2)
		{
			transformCom = factory->InstanceComponent("StaticTransform", position, 0.f);
		}
		else if (i == 4)
		{
			transformCom = factory->InstanceComponent("b2Kinematic", position, 0.f);
		}
		else
		{
			transformCom = factory->InstanceComponent("b2RigidBody", position, 0.f);
		}

		auto entity = std::make_shared<Entity>(entityManager, &entityManager->m_PropChangedQueue, transformCom);

		if (i == 2)
		{
			ObjectID id = 0;
			id = instantiationSynchroniser->m_WorldIdGenerator.getFreeID();
			entity->SetID(id);

			std::stringstream str;
			str << i << "_" << id;
			entity->SetName("edit" + str.str());

			entity->SetDomain(SYSTEM_DOMAIN);
		}
		//else
		//{
		//	std::stringstream str;
		//	str << reinterpret_cast<uintptr_t>(entity.get());
		//	entity->SetName("edit" + str.str());
		//}

		if (add_to_scene)
			entityManager->AddEntity(entity);

		ComponentPtr b2CircleFixture;
		if (i == 3 || i == 4)
		{
			b2CircleFixture = factory->InstanceComponent("b2Circle");
			entity->AddComponent(b2CircleFixture);
			{
				auto fixture = entity->GetComponent<FusionEngine::IFixture>();
				fixture->Density.Set(0.8f);
				fixture->Sensor.Set(i > 80);
				auto shape = entity->GetComponent<ICircleShape>();
				shape->Radius.Set(ToSimUnits(50.f / 2.f));
			}
			entity->SynchroniseParallelEdits();
		}

		auto clSprite = factory->InstanceComponent("CLSprite");
		entity->AddComponent(clSprite);

		ComponentPtr asScript, asScript2;
		if (i == 4)
		{
			asScript = factory->InstanceComponent("ASScript");
			entity->AddComponent(asScript, "script_a");
		}

		if (i == 2)
		{
			asScript2 = factory->InstanceComponent("ASScript");
			entity->AddComponent(asScript2, "spawn_script");
		}

		if (i == 4)
		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Depth.Set(1);
		}

		{
			auto sprite = entity->GetComponent<ISprite>();
			if (i == 1)
			{
				sprite->ImagePath.Set("Entities/Dirt.png");
			}
			else if (i == 3)
			{
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving.png");
				sprite->AnimationPath.Set("Entities/Test/test_anim.yaml");
			}
			else
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving1.png");
		}

		if (i == 5)
		{
			auto script = entity->GetComponent<IScript>("script_a");
			if (script)
				script->ScriptPath.Set("Scripts/test_script.as");
		}
		if (i == 2)
		{
			auto script = entity->GetComponent<IScript>("spawn_script");
			if (script)
				script->ScriptPath.Set("Scripts/SpawnPoint.as");
		}
		entity->SynchroniseParallelEdits();

		{
			auto body = entity->GetComponent<IRigidBody>();
			if (body)
			{
				//body->ApplyTorque(10.f);
				//body->ApplyForce(Vector2(2000, 0), body->GetCenterOfMass() + Vector2(2, -1));
				//body->AngularVelocity.Set(CL_Angle(180, cl_degrees).to_radians());
				body->LinearDamping.Set(0.1f);
				body->AngularDamping.Set(0.9f);
			}
		}

		return entity;
	}

	~ComponentTest()
	{
	}

};

} // end of FusionEngine namespace

class EntryPoint
{
public:
	static int main(const std::vector<CL_String> &args)
	{
		FusionEngine::ComponentTest app;
		return app.main(args);
	}
};

CL_ClanApplication app(&EntryPoint::main);
