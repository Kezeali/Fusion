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
#include "../FusionEngine/FusionScriptManager.h"

#include "../FusionEngine/FusionContextMenu.h"
#include "../FusionEngine/FusionElementUndoMenu.h"
#include "../FusionEngine/FusionEntityManager.h"
#include "../FusionEngine/FusionEntityFactory.h"
#include "../FusionEngine/FusionExceptionFactory.h"
#include "../FusionEngine/FusionScriptedConsoleCommand.h"
#include "../FusionEngine/FusionRenderer.h"
#include "../FusionEngine/FusionScriptModule.h"
#include "../FusionEngine/FusionScriptSound.h"

#include <ClanLib/application.h>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>
#include <ClanLib/vorbis.h>

namespace FusionEngine
{

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

				//logger->ActivateConsoleLogging();

				////////////////////
				// Script Manager
				auto scriptManager = std::make_shared<ScriptManager>();
				asIScriptEngine* asEngine = scriptManager->GetEnginePtr();

				Console::Register(scriptManager.get());
				RegisterScriptedConsoleCommand(asEngine);
				GUI::Register(scriptManager.get());
				ContextMenu::Register(asEngine);

				// Component types
				RegisterComponentInterfaceType<ITransform>(asEngine);
				RegisterComponentInterfaceType<IRigidBody>(asEngine);
				RegisterComponentInterfaceType<IFixture>(asEngine);
				RegisterComponentInterfaceType<ICircleShape>(asEngine);
				RegisterComponentInterfaceType<IPolygonShape>(asEngine);

				RegisterComponentInterfaceType<ISprite>(asEngine);

				RegisterComponentInterfaceType<IScript>(asEngine);

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
				
				const std::unique_ptr<Box2DSystem> box2dSystem(new Box2DSystem());
				auto box2dWorld = box2dSystem->CreateWorld();
				ontology.push_back(box2dWorld);
				
				static_cast<CLRenderWorld*>(renderWorld)->SetPhysWorld(static_cast<Box2DWorld*>(box2dWorld)->Getb2World());

				const std::unique_ptr<AngelScriptSystem> asSystem(new AngelScriptSystem(scriptManager));
				auto asWorld = asSystem->CreateWorld();
				ontology.push_back(asWorld);

				scheduler->SetOntology(ontology);


				std::vector<std::shared_ptr<Entity>> entities;

				float xtent = 500;
				Vector2 position(ToSimUnits(-xtent), ToSimUnits(-xtent));
				for (unsigned int i = 0; i < 500; ++i)
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

					std::shared_ptr<IComponent> asScript;
					if (i < 200)
					{
						asScript = asWorld->InstantiateComponent("ASScript");
						entity->AddComponent(asScript, "script_a");
					}

					//auto asScript2 = asWorld->InstantiateComponent("ASScript");
					//entity->AddComponent(asScript2, "script_b");

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
						script->ScriptPath.Set("Entities/Test/test_script.as");

						//script = entity->GetComponent<IScript>("script_b");
						//script->ScriptPath.Set("Entities/Test/test_script.as");
					}
					entity->SynchroniseParallelEdits();
					const auto& components = entity->GetComponents();
					for (auto it = components.begin(), end = components.end(); it != end; ++it)
						(*it)->FireSignals();

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
					//asWorld->OnActivation(asScript2);
				}

				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				dynamic_cast<CLRenderWorld*>(renderWorld)->AddViewport(viewport);

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
					
					scheduler->Execute();

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_SPACE))
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

					for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
					{
						auto& entity = *it;
						entity->SynchroniseParallelEdits();

						const auto& components = entity->GetComponents();
						for (auto it = components.begin(), end = components.end(); it != end; ++it)
							(*it)->FireSignals();
					}

					//scriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

					dispWindow.flip(0);
					gc.clear();

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
