#include "../FusionEngine/FusionStableHeaders.h"
#include "../FusionEngine/FusionPrerequisites.h"

// Logging
#include "../FusionEngine/FusionConsole.h"
#include "../FusionEngine/FusionConsoleStdOutWriter.h"
#include "../FusionEngine/FusionLogger.h"

// Filesystem
#include "../FusionEngine/FusionPaths.h"
#include "../FusionEngine/FusionPhysFS.h"
#include "../FusionEngine/FusionVirtualFileSource_PhysFS.h"

// Resource Loading
#include "../FusionEngine/FusionResourceManager.h"
#include "../FusionEngine/FusionAudioLoader.h"
#include "../FusionEngine/FusionImageLoader.h"

// Network
#include "../FusionEngine/FusionRakNetwork.h"
#include "../FusionEngine/FusionPacketDispatcher.h"
#include "../FusionEngine/FusionPlayerRegistry.h"

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
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionEntityFactory.h"
#include "../FusionEngine/FusionExceptionFactory.h"
#include "../FusionEngine/FusionInstanceSynchroniser.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionRenderer.h"
#include "../FusionEngine/FusionScriptInputEvent.h"
#include "../FusionEngine/FusionScriptModule.h"
#include "../FusionEngine/FusionScriptSound.h"

#include "../FusionEngine/FusionAngelScriptComponent.h"

#include <boost/thread.hpp>

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>

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

	class SimpleCellArchiver : public CellArchiver
	{
	public:
		SimpleCellArchiver()
			: m_NewData(false)
		{
		}

		~SimpleCellArchiver()
		{
			Stop();
		}

		void Enqueue(Cell* cell, size_t i)
		{
			if (cell->waiting.fetch_and_store(true) && cell->loaded)
			{
				m_WriteQueue.push(std::make_tuple(cell, i));
				m_NewData.set();
			}
		}

		void Retrieve(Cell* cell, size_t i)
		{
			if (cell->waiting.fetch_and_store(true) && !cell->loaded)
			{
				m_ReadQueue.push(std::make_tuple(cell, i));
				m_NewData.set();
			}
		}

		boost::thread m_Thread;

		void Start()
		{
			m_Quit.reset();
			m_Thread = boost::thread(&SimpleCellArchiver::Run, this);
#ifdef _WIN32
			SetThreadPriority(m_Thread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
		}

		void Stop()
		{
			m_Quit.set();
			m_Thread.join();
		}

		void Run()
		{
			bool retrying = false;
			while (CL_Event::wait(m_Quit, m_NewData, retrying ? 200 : -1) != 0)
			{
				std::list<std::tuple<Cell*, size_t>> stuffToRetry;
				{
					std::tuple<Cell*, size_t> toWrite;
					while (m_WriteQueue.try_pop(toWrite))
					{
						Cell*& cell = std::get<0>(toWrite);
						size_t& i = std::get<1>(toWrite);
						if (cell->mutex.try_lock())
						{
							cell->waiting = false;
							try
							{
								std::vector<EntityPtr> newArchive;
								for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
								{
									if (it->first->IsActive())
									{
										stuffToRetry.push_back(std::move(toWrite));
										break;
									}
									//if (it->first->IsSyncedEntity())
									{
										newArchive.push_back(it->first->shared_from_this());
									}
								}
								if (stuffToRetry.empty())
								{
									m_Archived[i] = std::move(newArchive);
									cell->objects.clear();
									cell->loaded = false;
								}
							}
							catch (...)
							{
							}
							cell->mutex.unlock();
						}
					}
				}
				{
					std::tuple<Cell*, size_t> toRead;
					while (m_ReadQueue.try_pop(toRead))
					{
						Cell*& cell = std::get<0>(toRead);
						size_t& i = std::get<1>(toRead);//, y = std::get<1>(toRead);
						cell->mutex.lock();
						{
							cell->waiting = false;
							try
							{
								auto archivedCellEntry = m_Archived.find(i);

								if (archivedCellEntry != m_Archived.end())
								{
									for (auto it = archivedCellEntry->second.begin(), end = archivedCellEntry->second.end(); it != end; ++it)
									{
										auto& archivedEntity = *it;
										Vector2 pos = archivedEntity->GetPosition();
										// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
										CellEntry entry;
										entry.x = ToGameUnits(pos.x); entry.y = ToGameUnits(pos.y);
										cell->objects.push_back(std::make_pair(archivedEntity.get(), std::move(entry)));
									}

									cell->loaded = true;

									m_Archived.erase(archivedCellEntry);
								}
								else
									cell->loaded = true; // No data to load
							}
							catch (...)
							{
							}
							cell->mutex.unlock();
						}
					}
				}
				if (!stuffToRetry.empty())
				{
					retrying = true;
					for (auto it = stuffToRetry.begin(), end = stuffToRetry.end(); it != end; ++it)
						m_WriteQueue.push(*it);
					stuffToRetry.clear();
				}
				else
					retrying = false;
			}
		}

		std::map<size_t, std::vector<EntityPtr>> m_Archived;

		tbb::concurrent_queue<std::tuple<Cell*, size_t>> m_WriteQueue;
		tbb::concurrent_queue<std::tuple<Cell*, size_t>> m_ReadQueue;

		CL_Event m_NewData;
		CL_Event m_Quit;
	};

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
		FSN_ASSERT(SetupPhysFS::is_init());

		CL_ConsoleWindow conWindow("Component Test Console", 80, 10);
		CL_DisplayWindow dispWindow("Component Test", 800, 600);

		{
			std::unique_ptr<Logger> logger;
			std::unique_ptr<ConsoleStdOutWriter> coutWriter;
			std::unique_ptr<Console> console;

			try
			{
				console.reset(new Console());
				coutWriter.reset(new ConsoleStdOutWriter());
				coutWriter->Enable();
				logger.reset(new Logger());
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
#ifdef _DEBUG
				// Clear cache
				SetupPhysFS::clear_temp();
#endif

				logger->ActivateConsoleLogging();

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

				// Entity generation
				Entity::Register(asEngine);
				ASScript::Register(asEngine);
				InstancingSynchroniser::Register(asEngine);

				Camera::Register(asEngine);
				StreamingManager::Register(asEngine);

				CLRenderWorld::Register(asEngine);

				// Console singleton
				scriptManager->RegisterGlobalObject("Console console", Console::getSingletonPtr());

				/////////////////////////////////////
				// Script SoundOutput wrapper object
				auto script_SoundOutput = std::make_shared<SoundOutput>(sound_output);

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
					FSN_EXCEPT_CS(ExCode::IO, "startup", "InputManager couldn't find a keyboard device.");
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

				std::unique_ptr<SimpleCellArchiver> cellArchivist(new SimpleCellArchiver());

				// Entity management / instantiation
				std::unique_ptr<EntityFactory> entityFactory(new EntityFactory());
				std::unique_ptr<EntitySynchroniser> entitySynchroniser(new EntitySynchroniser(inputMgr.get()));
				std::unique_ptr<StreamingManager> streamingMgr(new StreamingManager(cellArchivist.get()));
				std::unique_ptr<EntityManager> entityManager(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get()));
				std::unique_ptr<InstancingSynchroniser> instantiationSynchroniser(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));

				cellArchivist->Start();

				scriptManager->RegisterGlobalObject("StreamingManager streaming", streamingMgr.get());

				entityManager->m_EntityFactory = entityFactory.get();

				// Component systems
				const std::unique_ptr<TaskManager> taskManager(new TaskManager());
				const std::unique_ptr<TaskScheduler> scheduler(new TaskScheduler(taskManager.get(), entityManager.get()));

#ifdef PROFILE_BUILD
				scheduler->SetFramerateLimiter(false);
				scheduler->SetUnlimited(true);
#else
				scheduler->SetFramerateLimiter(true);
#endif

				std::vector<std::shared_ptr<ISystemWorld>> ontology;

				const std::unique_ptr<CLRenderSystem> clRenderSystem(new CLRenderSystem(gc));
				auto renderWorld = clRenderSystem->CreateWorld();
				ontology.push_back(renderWorld);

				entityFactory->AddInstancer(renderWorld);

				scriptManager->RegisterGlobalObject("Renderer renderer", renderWorld.get());
				
				const std::unique_ptr<IComponentSystem> box2dSystem(new Box2DSystem());
				auto box2dWorld = box2dSystem->CreateWorld();
				ontology.push_back(box2dWorld);

				entityFactory->AddInstancer(box2dWorld);
				
				static_cast<CLRenderWorld*>(renderWorld.get())->SetPhysWorld(static_cast<Box2DWorld*>(box2dWorld.get())->Getb2World());

				const std::unique_ptr<AngelScriptSystem> asSystem(new AngelScriptSystem(scriptManager, entityFactory.get()));
				auto asWorld = asSystem->CreateWorld();
				ontology.push_back(asWorld);

				// TODO: add some sort of Init method, to be called by the scheduler (?) when the ontology is set (?)
				static_cast<AngelScriptWorld*>(asWorld.get())->BuildScripts();

				entityFactory->AddInstancer(asWorld);

				scheduler->SetOntology(ontology);


				std::vector<std::shared_ptr<Entity>> entities;

				tbb::concurrent_queue<IComponentProperty*> &propChangedQueue = entityManager->m_PropChangedQueue;

				float xtent = 1.f;
				Vector2 position(0.0f, 0.0f);
				{
					unsigned int i = 0;
					//position.x += ToSimUnits(50.f);
					//if (position.x >= ToSimUnits(xtent))
					//{
					//	position.x = ToSimUnits(-xtent);
					//	position.y += ToSimUnits(50.f);
					//}			
					
					std::shared_ptr<IComponent> transformCom;
					if (i < 300)
					{
						if (i == 0)
							transformCom = box2dWorld->InstantiateComponent("b2Kinematic", position, 0.f);
						else
							transformCom = box2dWorld->InstantiateComponent((i < 30) ? "b2RigidBody" : "b2Static", position, 0.f);
					}
					else
					{
						transformCom = box2dWorld->InstantiateComponent("StaticTransform", position, 0.f);
					}

					auto entity = std::make_shared<Entity>(&entityManager->m_PropChangedQueue, transformCom);
					std::stringstream str;
					str << i;
					entity->SetName("initentity" + str.str());

					entityManager->AddEntity(entity);

					std::shared_ptr<IComponent> b2CircleFixture;
					if (i < 300)
					{
						b2CircleFixture = box2dWorld->InstantiateComponent("b2Circle");
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

					auto clSprite = renderWorld->InstantiateComponent("CLSprite");
					entity->AddComponent(clSprite);

					std::shared_ptr<IComponent> asScript, asScript2;
					if (i < 200)
					{
						asScript = asWorld->InstantiateComponent("ASScript");
						entity->AddComponent(asScript, "script_a");

						//asScript2 = asWorld->InstantiateComponent("ASScript");
						//entity->AddComponent(asScript2, "script_b");
					}

					if (i == 1)
					{
						auto transform = entity->GetComponent<ITransform>();
						transform->Depth.Set(1);
					}

					{
						auto sprite = entity->GetComponent<ISprite>();
						if (i > 150 && i < 300)
						{
							sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving.png");
							sprite->AnimationPath.Set("Entities/Test/test_anim.yaml");
						}
						else
							sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving1.png");
					}

					if (i < 200)
					{
						auto script = entity->GetComponent<IScript>("script_a");
						if (script)
							script->ScriptPath.Set("Scripts/test_script.as");

						//script = entity->GetComponent<IScript>("script_b");
						//if (script)
						//	script->ScriptPath.Set("Scripts/TestB.as");
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

					//entities.push_back(entity);

					//entity->StreamIn();
					//if (b2BodyCom)
					//	box2dWorld->OnActivation(b2BodyCom);
					//if (b2CircleFixture)
					//	box2dWorld->OnActivation(b2CircleFixture);
					//renderWorld->OnActivation(clSprite);
					//if (asScript)
					//	asWorld->OnActivation(asScript);
					//if (asScript2)
					//	asWorld->OnActivation(asScript2);
				}

				PlayerRegistry::AddLocalPlayer(1u, 0u);

				// This scope makes viewport hold the only reference to camera: thus camera will be deleted with viewport
				{
				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				dynamic_cast<CLRenderWorld*>(renderWorld.get())->AddViewport(viewport);
				streamingMgr->AddCamera(camera);
				}

				auto keyhandlerSlot = dispWindow.get_ic().get_keyboard().sig_key_up().connect_functor([&](const CL_InputEvent& ev, const CL_InputState&)
				{
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
						if (rangedown && range <= 500)
							range -= 100;
						else if (rangeup && range < 500)
							range += 100;
						else
							range += (rangeup ? 500 : -500);
						fe_clamp(range, 100u, 10000u);
						streamingMgr->SetRange((float)range);
					  std::stringstream str;
					  str << range;
						SendToConsole(str.str());
					}
				});

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

					resourceManager->UnloadUnreferencedResources();
					resourceManager->DeliverLoadedResources();

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_1))
					{
						// Accumulator
						scheduler->SetFramerateLimiter(false);
						scheduler->SetUnlimited(false);
					}
					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_2))
						scheduler->SetFramerateLimiter(true);
					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_3))
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
					}
					
					const auto rendered = scheduler->Execute();

					if (rendered & SystemType::Rendering)
					{
						dispWindow.flip(0);
						gc.clear();
					}
					
					entityManager->ProcessActivationQueues();

					IComponentProperty *changed;
					while (propChangedQueue.try_pop(changed))
					{
						changed->Synchronise();
						changed->FireSignal();
					}

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_ESCAPE))
						keepGoing = false;
				}
				scriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);
			}
			catch (FusionEngine::Exception &ex)
			{
#ifdef _DEBUG
				if (logger)
					logger->Add(ex.ToString());
#endif
				//TODO: Show a OS native GUI messagebox in Release builds
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
				//TODO: Show a OS native GUI messagebox in Release builds
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), ex.what(), "Exception", MB_OK);
#endif
			}

		}

		return 0;
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
