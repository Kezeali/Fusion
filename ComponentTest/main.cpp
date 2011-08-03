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
#include "../FusionEngine/FusionScriptModule.h"
#include "../FusionEngine/FusionScriptSound.h"

#include "../FusionEngine/FusionAngelScriptComponent.h"

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
	struct iface##_##prop { static ThreadSafeProperty<type> &get_ ## prop(void *obj) { return GetIface<iface>(obj)->prop; } };\
	ThreadSafeProperty<type>::RegisterProp(engine, scriptType);\
	{int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "Property_" scriptType "_ @get_" #prop "()", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "void set_" #prop "(Property_" scriptType "_ @)", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
	FSN_ASSERT(r >= 0);}

#define FSN_REGISTER_PROP_ACCESSOR_R(iface, type, scriptType, prop) \
	struct iface##_##prop { static ThreadSafeProperty<type, NullWriter<type>> &get_ ## prop(void *obj) { return GetIface<iface>(obj)->prop; } };\
	ThreadSafeProperty<type, NullWriter<type>>::RegisterProp(engine, scriptType);\
	{int r = engine->RegisterObjectMethod(iface::GetTypeName().c_str(), "const ReadonlyProperty_" scriptType "_ &get_" #prop "() const", asFUNCTION(iface##_##prop :: get_ ## prop ), asCALL_CDECL_OBJLAST);\
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

				// Entity management / instantiation
				std::unique_ptr<EntityFactory> entityFactory(new EntityFactory());
				std::unique_ptr<EntitySynchroniser> entitySynchroniser(new EntitySynchroniser(inputMgr.get()));
				std::unique_ptr<StreamingManager> streamingMgr(new StreamingManager());
				std::unique_ptr<EntityManager> entityManager(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get()));
				std::unique_ptr<InstancingSynchroniser> instantiationSynchroniser(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));

				entityManager->m_EntityFactory = entityFactory.get();

				// Component systems
				const std::unique_ptr<TaskManager> taskManager(new TaskManager());
				const std::unique_ptr<TaskScheduler> scheduler(new TaskScheduler(taskManager.get()));

#ifdef PROFILE_BUILD
				scheduler->SetFramerateLimiter(false);
				scheduler->SetUnlimited(true);
#else
				scheduler->SetFramerateLimiter(true);
#endif

				std::vector<ISystemWorld*> ontology;

				const std::unique_ptr<CLRenderSystem> clRenderSystem(new CLRenderSystem(gc));
				auto renderWorld = clRenderSystem->CreateWorld();
				ontology.push_back(renderWorld);

				entityFactory->AddInstancer(renderWorld);
				
				const std::unique_ptr<Box2DSystem> box2dSystem(new Box2DSystem());
				auto box2dWorld = box2dSystem->CreateWorld();
				ontology.push_back(box2dWorld);

				entityFactory->AddInstancer(box2dWorld);
				
				static_cast<CLRenderWorld*>(renderWorld)->SetPhysWorld(static_cast<Box2DWorld*>(box2dWorld)->Getb2World());

				const std::unique_ptr<AngelScriptSystem> asSystem(new AngelScriptSystem(scriptManager));
				auto asWorld = asSystem->CreateWorld();
				ontology.push_back(asWorld);

				entityFactory->AddInstancer(asWorld);

				scheduler->SetOntology(ontology);


				std::vector<std::shared_ptr<Entity>> entities;

				tbb::concurrent_queue<IComponentProperty*> &propChangedQueue = entityManager->m_PropChangedQueue;

				float xtent = 720;
				Vector2 position(ToSimUnits(-xtent), ToSimUnits(-xtent));
#ifdef _DEBUG
				xtent = 20;
				for (unsigned int i = 0; i < 1; ++i)
#else
				for (unsigned int i = 0; i < 1500; ++i)
#endif
				{
					position.x += ToSimUnits(50.f);
					if (position.x >= ToSimUnits(xtent))
					{
						position.x = ToSimUnits(-xtent);
						position.y += ToSimUnits(50.f);
					}

					auto entity = std::make_shared<Entity>();
					std::stringstream str;
					str << i;
					entity->_setName("entity" + str.str());

					entity->SetPropChangedQueue(&entityManager->m_PropChangedQueue);
					
					std::shared_ptr<IComponent> b2CircleFixture;
					std::shared_ptr<IComponent> b2BodyCom;
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

						if (i == 0)
							b2BodyCom = box2dWorld->InstantiateComponent("b2Kinematic", position, 0.f, nullptr, nullptr);
						else
							b2BodyCom = box2dWorld->InstantiateComponent((i < 30) ? "b2RigidBody" : "b2Static", position, 0.f, nullptr, nullptr);

						entity->AddComponent(b2BodyCom);
					}
					else
					{
						auto transformCom = box2dWorld->InstantiateComponent("StaticTransform", position, 0.f, nullptr, nullptr);
						entity->AddComponent(transformCom);
					}

					auto clSprite = renderWorld->InstantiateComponent("CLSprite");
					entity->AddComponent(clSprite);

					std::shared_ptr<IComponent> asScript, asScript2;
					if (i < 200)
					{
						asScript = asWorld->InstantiateComponent("ASScript");
						entity->AddComponent(asScript, "script_a");

						asScript2 = asWorld->InstantiateComponent("ASScript");
						entity->AddComponent(asScript2, "script_b");
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
						script->ScriptPath.Set("Scripts/test_script.as");

						script = entity->GetComponent<IScript>("script_b");
						if (script)
							script->ScriptPath.Set("Scripts/TestB.as");
					}
					entity->SynchroniseParallelEdits();

					if (b2BodyCom)
					{
						auto body = entity->GetComponent<IRigidBody>();
						//body->ApplyTorque(10.f);
						//body->ApplyForce(Vector2(2000, 0), body->GetCenterOfMass() + Vector2(2, -1));
						//body->AngularVelocity.Set(CL_Angle(180, cl_degrees).to_radians());
						body->LinearDamping.Set(0.1f);
						body->AngularDamping.Set(0.9f);
					}

					entities.push_back(entity);

					//entity->StreamIn();
					if (b2BodyCom)
						box2dWorld->OnActivation(b2BodyCom);
					if (b2CircleFixture)
						box2dWorld->OnActivation(b2CircleFixture);
					renderWorld->OnActivation(clSprite);
					if (asScript)
						asWorld->OnActivation(asScript);
					if (asScript2)
						asWorld->OnActivation(asScript2);
				}

				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				dynamic_cast<CLRenderWorld*>(renderWorld)->AddViewport(viewport);

				std::map<int, bool> pressed;

				auto keyhandlerSlot = dispWindow.get_ic().get_keyboard().sig_key_up().connect_functor([&](const CL_InputEvent& ev, const CL_InputState&)
				{
					bool dtup = ev.id == CL_KEY_PRIOR;
					bool dtdown = ev.id == CL_KEY_NEXT;
					if (dtup || dtdown)
					{
						pressed[CL_KEY_PRIOR] = true;
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

					if (ev.id == CL_KEY_E)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Dynamic);

							body->AngularDamping.Set(0.f);

							auto vel = body->AngularVelocity.Get();
							if (ev.shift)
								vel = std::max(b2_pi / 2.f, vel);
							vel += b2_pi * 0.25;
							body->AngularVelocity.Set(CL_Angle(vel, cl_radians).to_radians());
						}
					}
					if (ev.id == CL_KEY_Q)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Dynamic);

							body->AngularDamping.Set(0.f);

							auto vel = body->AngularVelocity.Get();
							if (ev.shift)
								vel = std::min(-(b2_pi / 2.f), vel);
							vel -= b2_pi * 0.25;
							body->AngularVelocity.Set(CL_Angle(vel, cl_radians).to_radians());
						}
					}
					if (ev.id == CL_KEY_X)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Dynamic);

							//body->AngularVelocity.Set(CL_Angle(0.0f, cl_radians).to_radians());
							body->AngularDamping.Set(0.9f);
						}
					}

					bool w = ev.id == CL_KEY_W;
					bool s = ev.id == CL_KEY_S;
					bool a = ev.id == CL_KEY_A;
					bool d = ev.id == CL_KEY_D;
					if (w || s || a || d)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Dynamic);

							auto vel = body->Velocity.Get();
							v2Multiply(vel, Vector2((a || d) ? 0.f : 1.f, (w || s) ? 0.f : 1.f), vel);
							body->Velocity.Set(vel);
							//body->AngularVelocity.Set(CL_Angle(45, cl_degrees).to_radians());
						}
					}

					if (ev.id == CL_KEY_SPACE)
					{
						const float invmax = 1.0f / RAND_MAX;
						auto entity = entities.at((size_t)(std::rand() * invmax * 30/*entities.size()*/));
						auto body = entity->GetComponent<IRigidBody>();
						if (body && body->GetBodyType() == IRigidBody::Dynamic)
						{
							//body->ApplyForce(Vector2(ToSimUnits(1000.f), 0.f), body->CenterOfMass.Get() + Vector2(0.f, ToSimUnits(std::rand() * invmax * 6.f - 3.f)));
							
							Vector2 force(std::rand() * invmax * 1000 - 500, std::rand() * invmax * 1000 - 500);
							body->ApplyForce(Vector2(ToSimUnits(force.x), ToSimUnits(force.y)));
						}
						//body->AngularVelocity.Set(CL_Angle(45, cl_degrees).to_radians());
					}

					bool k = ev.id == CL_KEY_K;
					bool j = ev.id == CL_KEY_J;
					if (k || j)
					{
						auto entity = entities[0];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Kinematic);

							const float xvel = k ? 0.5f : -0.5f;
							body->Velocity.Set(Vector2(xvel, 0.0f));
							//body->AngularVelocity.Set(CL_Angle(45, cl_degrees).to_radians());
						}
					}
					if (ev.id == CL_KEY_M)
					{
						auto entity = entities[0];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Kinematic);

							body->Velocity.Set(Vector2::zero());
							//body->AngularVelocity.Set(CL_Angle(45, cl_degrees).to_radians());
						}
					}
				});

				auto keydownhandlerSlot = dispWindow.get_ic().get_keyboard().sig_key_down().connect_functor([&](const CL_InputEvent& ev, const CL_InputState&)
				{
					if (ev.id == CL_KEY_I)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							body->Interpolate.Set(!body->Interpolate.Get());
						}
					}
				});

				unsigned int lastframe = CL_System::get_time();
				unsigned int delta = 0;
				float seconds = 0.f;

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

					bool up = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_UP);
					bool down = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_DOWN);
					bool left = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_LEFT);
					bool right = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_RIGHT);
					if (up || down || left || right)
					{
						auto camDelta = delta / 10.f;
						auto pos = camera->GetPosition();
						if (up)
							pos.y -= camDelta;
						if (down)
							pos.y += camDelta;
						if (right)
							pos.x += camDelta;
						if (left)
							pos.x -= camDelta;
						camera->SetPosition(pos.x, pos.y);
					}
					
					const auto rendered = scheduler->Execute();

					entityManager->Update(delta * 0.001f);

					if (rendered & SystemType::Rendering)
					{
						dispWindow.flip(0);
						gc.clear();
					}

					bool w = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_W);
					bool s = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_S);
					bool a = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_A);
					bool d = dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_D);
					if (w || s || a || d)
					{
						auto entity = entities[1];
						auto body = entity->GetComponent<IRigidBody>();
						if (body)
						{
							FSN_ASSERT(body->GetBodyType() == IRigidBody::Dynamic);

							entity->SynchroniseParallelEdits();

							const float speed = 0.8f;
							Vector2 vel = body->Velocity.Get();
							if (w)
								vel.y = -speed;
							else if (s)
								vel.y = speed;
							else if (w && s)
								vel.y = 0;
							if (a)
								vel.x = -speed;
							else if (d)
								vel.x = speed;
							else if (a && d)
								vel.x = 0;

							body->Velocity.Set(vel);
							//body->AngularVelocity.Set(CL_Angle(45, cl_degrees).to_radians());
						}
					}

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_CONTROL))
					{
						//const size_t numents = (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_CONTROL)) ? entities.size() : 300u;
						for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
							//for (size_t i = 0; i < numents; ++i)
						{
							//auto& entity = entities[i];
							auto& entity = *it;
							entity->SynchroniseParallelEdits();

							const auto& components = entity->GetComponents();
							for (auto it = components.begin(), end = components.end(); it != end; ++it)
								(*it)->FireSignals();
						}
					}
					else
					{
						IComponentProperty *changed;
						while (propChangedQueue.try_pop(changed))
						{
							changed->Synchronise();
							changed->FireSignal();
						}
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
