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

	class SimpleCellArchiver : public CellArchiver
	{
	public:
		SimpleCellArchiver(bool edit_mode)
			: m_NewData(false),
			m_Instantiator(nullptr),
			m_Running(false),
			m_EditMode(edit_mode),
			m_BeginIndex(std::numeric_limits<size_t>::max()),
			m_EndIndex(0)
		{
		}

		~SimpleCellArchiver()
		{
			Stop();
		}

		InstancingSynchroniser* m_Instantiator;
		void SetSynchroniser(InstancingSynchroniser* instantiator)
		{
			m_Instantiator = instantiator;
		}

		std::shared_ptr<GameMap> m_Map;
		void SetMap(const std::shared_ptr<GameMap>& map)
		{
			m_Map = map;
		}

		void Enqueue(Cell* cell, size_t i)
		{
			if (cell->waiting.fetch_and_store(Cell::Store) != Cell::Store && cell->loaded)
			{
				cell->AddHist("Enqueued Out");
				m_WriteQueue.push(std::make_tuple(cell, i));
				m_NewData.set();
			}
		}

		bool Retrieve(Cell* cell, size_t i)
		{
			if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
			{
				cell->AddHist("Enqueued In");
				m_ReadQueue.push(std::make_tuple(cell, i));
				m_NewData.set();
				return true;
			}
			return false;
		}

		boost::thread m_Thread;

		void Start()
		{
			m_Running = true;

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

			m_Running = false;
		}

		CL_IODevice GetFile(size_t cell_index, bool write) const
		{
			try
			{
				std::stringstream str; str << cell_index;
				std::string filename = "cache/" + str.str();//"cache/partialsave/"
				if (write || PHYSFS_exists(filename.c_str()))
				{
					CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
					auto file = vdir.open_file(filename, write ? CL_File::create_always : CL_File::open_existing, write ? CL_File::access_write : CL_File::access_read);
					return file;
				}
				else return CL_IODevice();
			}
			catch (CL_Exception& ex)
			{
				SendToConsole(ex.what());
				return CL_IODevice();
			}
		}

		CL_IODevice GetCellData(size_t index) const
		{
			if (m_EditMode && !m_Running)
				return GetFile(index, false);
			else
				FSN_EXCEPT(InvalidArgumentException, "Can't access cell data while running");
		}

		size_t GetDataBegin() const
		{
			if (m_EditMode)
				return m_BeginIndex;
			else
				FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
		}

		size_t GetDataEnd() const
		{
			if (m_EditMode)
				return m_EndIndex;
			else
				FSN_EXCEPT(InvalidArgumentException, "This function is only available in edit mode");
		}

		EntityPtr Load(CL_IODevice& file, bool includes_id)
		{
			return EntitySerialisationUtils::LoadEntity(file, includes_id, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
		}

		void Run()
		{
			using namespace EntitySerialisationUtils;

			bool retrying = false;
			// TODO: make m_NewData not auto-reset
			while (CL_Event::wait(m_Quit, m_NewData, retrying ? 100 : -1) != 0)
			{
				std::list<std::tuple<Cell*, size_t>> writesToRetry;
				std::list<std::tuple<Cell*, size_t>> readsToRetry;
				{
					std::tuple<Cell*, size_t> toWrite;
					while (m_WriteQueue.try_pop(toWrite))
					//while (!m_WriteQueue.empty())
					{
						//toWrite = m_WriteQueue.front();
						//m_WriteQueue.pop();
						Cell*& cell = std::get<0>(toWrite);
						size_t& i = std::get<1>(toWrite);
						Cell::mutex_t::scoped_try_lock lock(cell->mutex);
						if (lock)
						{
							// Check active_entries since the Store request may be stale
							if ((m_EditMode || cell->active_entries == 0) && cell->waiting == Cell::Store)
							{
								try
								{
									FSN_ASSERT(cell->loaded == true); // Don't want to create an inaccurate cache (without entities from the map file)
									
									size_t numSynched = 0;
									size_t numPseudo = 0;
									std::for_each(cell->objects.begin(), cell->objects.end(), [&](const Cell::CellEntryMap::value_type& obj)
									{
										if (!obj.first->IsSyncedEntity())
											++numPseudo;
										else
											++numSynched;
									});

									auto write = [](CL_IODevice& file, const Cell* cell, size_t numEntries, const bool synched)
									{
										using namespace EntitySerialisationUtils;
										file.write(&numEntries, sizeof(size_t));
										for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
										{
											if (it->first->IsSyncedEntity() == synched)
											{
												SaveEntity(file, it->first, synched);
												FSN_ASSERT(numEntries-- > 0);
											}
										}
									};

									if (m_EditMode)
									{
										if (numSynched > 0 || numPseudo > 0)
										{
											auto file = GetFile(i, true);
											if (!file.is_null())
											{
												m_BeginIndex = std::min(i, m_BeginIndex);
												m_EndIndex = std::max(i, m_EndIndex);
												write(file, cell, numPseudo, false);
												write(file, cell, numSynched, true);
											}
											else
												FSN_EXCEPT(FileSystemException, "Failed to open file to dump edit-mode cache");
										}
									}
									else if (numSynched > 0)
									{
										auto file = GetFile(i, true);
										file.write(&numSynched, sizeof(size_t));
										for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
										{
											if (it->first->IsSyncedEntity())
											{
												SaveEntity(file, it->first, true);
												FSN_ASSERT(numSynched-- > 0);
											}
										}
									}

									cell->AddHist("Written and cleared", numSynched);

									//std::stringstream str; str << i;
									//SendToConsole("Cell " + str.str() + " streamed out");

									// TEMP
									if (!m_EditMode || cell->active_entries == 0)
									{
										cell->objects.clear();
										cell->loaded = false;
									}
								}
								catch (...)
								{
									std::stringstream str; str << i;
									SendToConsole("Exception streaming out cell " + str.str());
								}
							}
							else
							{
								//std::stringstream str; str << i;
								//SendToConsole("Cell still in use " + str.str());
								//writesToRetry.push_back(toWrite);
							}
							cell->waiting = Cell::Ready;
							//cell->mutex.unlock();
						}
						else
						{
							cell->AddHist("Cell locked (will retry write later)");
							std::stringstream str; str << i;
							SendToConsole("Retrying write on cell " + str.str());
							writesToRetry.push_back(toWrite);
						}
					}
				}
				{
					std::tuple<Cell*, size_t> toRead;
					while (m_ReadQueue.try_pop(toRead))
					//while (!m_ReadQueue.empty())
					{
						//toRead = m_ReadQueue.front();
						//m_ReadQueue.pop();
						Cell*& cell = std::get<0>(toRead);
						size_t& i = std::get<1>(toRead);//, y = std::get<1>(toRead);
						Cell::mutex_t::scoped_try_lock lock(cell->mutex);
						if (lock)
						{
							// Shouldn't be possible (unloaded cells shouldn't have active entries) - this could indicate
							//  that the Retrieve request got enqueued twice:
							//FSN_ASSERT(cell->active_entries == 0);
							if (cell->active_entries == 0 && cell->waiting == Cell::Retrieve)
							{
								try
								{
									auto file = GetFile(i, false);

									// Last param makes the method load synched entities from the map if the cache file isn't available:
									if (m_Map)
									{
										bool uncached = m_SynchLoaded.insert(i).second;
										m_Map->LoadCell(cell, i, uncached, m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator);
									}

									if (!file.is_null() && file.get_size() > 0)
									{
										auto load = [&](const bool synched)->size_t
										{
										size_t numEntries;
										file.read(&numEntries, sizeof(size_t));
										for (size_t n = 0; n < numEntries; ++n)
										{
											//auto& archivedEntity = *it;
											auto archivedEntity = Load(file, synched);

											Vector2 pos = archivedEntity->GetPosition();
											// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
											CellEntry entry;
											entry.x = ToGameUnits(pos.x); entry.y = ToGameUnits(pos.y);

											archivedEntity->SetStreamingCellIndex(i);

											cell->objects.push_back(std::make_pair(archivedEntity, std::move(entry)));
										}
										return numEntries;
										};
										if (m_EditMode)
											load(false);
										size_t num = load(true);

										//std::stringstream str; str << i;
										//SendToConsole("Cell " + str.str() + " streamed in");

										cell->AddHist("Loaded", num);
										cell->loaded = true;
									}
									else
									{
										cell->loaded = true; // No data to load
										cell->AddHist("Loaded (no data)");
									}
								}
								catch (...)
								{
									//std::stringstream str; str << i;
									//SendToConsole("Exception streaming in cell " + str.str());
									//readsToRetry.push_back(toRead);
								}
							}
							cell->waiting = Cell::Ready;
							//cell->mutex.unlock();
						}
						else
						{
							cell->AddHist("Cell locked (will retry read later)");
							readsToRetry.push_back(toRead);
						}
					}
				}
				retrying = false;
				if (!writesToRetry.empty())
				{
					retrying = true;
					for (auto it = writesToRetry.begin(), end = writesToRetry.end(); it != end; ++it)
						m_WriteQueue.push(*it);
					writesToRetry.clear();
				}
				if (!readsToRetry.empty())
				{
					retrying = true;
					for (auto it = readsToRetry.begin(), end = readsToRetry.end(); it != end; ++it)
						m_ReadQueue.push(*it);
					readsToRetry.clear();
				}

				/*std::stringstream str;
				str << m_Archived.size();
				SendToConsole(str.str() + " cells archived");*/
			}
		}

		bool m_EditMode;
		bool m_Running;

		size_t m_BeginIndex;
		size_t m_EndIndex;

		std::unordered_set<size_t> m_SynchLoaded;

		//boost::mutex m_WriteQueueMutex;
		//boost::mutex m_ReadQueueMutex;/*std::queue*/
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
		CL_DisplayWindow dispWindow("Component Test", 800, 600);

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

				/////////////////////////
				// Load optional settings (set options)
				ClientOptions* options = new ClientOptions("settings.xml", "settings");

				if (varMap.count("connect") == 0 && options->GetOption_bool("console_logging"))
					logger->ActivateConsoleLogging();

				bool editMode = options->GetOption_bool("edit");

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

				// Entity management / instantiation
				std::unique_ptr<EntityFactory> entityFactory(new EntityFactory());
				std::unique_ptr<SimpleCellArchiver> cellArchivist(new SimpleCellArchiver(editMode));
				std::unique_ptr<EntitySynchroniser> entitySynchroniser(new EntitySynchroniser(inputMgr.get()));
				
				std::unique_ptr<StreamingManager> streamingMgr(new StreamingManager(cellArchivist.get(), editMode));
				std::unique_ptr<EntityManager> entityManager(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get()));
				std::unique_ptr<InstancingSynchroniser> instantiationSynchroniser(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));

				try
				{
				std::unique_ptr<GameMapLoader> mapLoader(new GameMapLoader(options));

				cellArchivist->SetSynchroniser(instantiationSynchroniser.get());

				scriptManager->RegisterGlobalObject("StreamingManager streaming", streamingMgr.get());

				entityManager->m_EntityFactory = entityFactory.get();

				if (!editMode && varMap.count("connect") == 0)
				{
					auto map = mapLoader->LoadMap("default.gad", instantiationSynchroniser.get());
					cellArchivist->SetMap(map);

					streamingMgr->Initialise(map->GetMapWidth(), map->GetNumCellsAcross(), map->GetCellSize());
				}

				cellArchivist->Start();

				// Component systems
				const std::unique_ptr<TaskManager> taskManager(new TaskManager());
				const std::unique_ptr<TaskScheduler> scheduler(new TaskScheduler(taskManager.get(), entityManager.get()));

#ifdef PROFILE_BUILD
				scheduler->SetFramerateLimiter(false);
				scheduler->SetUnlimited(true);
#else
				scheduler->SetFramerateLimiter(false);
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
				static_cast<CLRenderWorld*>(renderWorld.get())->SetDebugDraw(false);

				const std::unique_ptr<AngelScriptSystem> asSystem(new AngelScriptSystem(scriptManager, entityFactory.get()));
				auto asWorld = asSystem->CreateWorld();
				ontology.push_back(asWorld);

				// TODO: add some sort of Init method, to be called by the scheduler (?) when the ontology is set (?)
				static_cast<AngelScriptWorld*>(asWorld.get())->BuildScripts();

				entityFactory->AddInstancer(asWorld);

				scheduler->SetOntology(ontology);


				PropChangedQueue &propChangedQueue = entityManager->m_PropChangedQueue;

				// This scope makes viewport hold the only reference to camera: thus camera will be deleted with viewport
				std::shared_ptr<Camera> editCam;
				{
				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				dynamic_cast<CLRenderWorld*>(renderWorld.get())->AddViewport(viewport);
				streamingMgr->AddCamera(camera);
				
				if (editMode)
					editCam = camera;
				}

				std::vector<EntityPtr> entities;
				bool compile = false;

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
						if (ev.shift)
						{
							repeats = 50;
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
							if (pos.x > (repeats / 2) * size)
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

					if (compile && editMode)
					{
						compile = false;

						streamingMgr->DumpAllCells();
						cellArchivist->Stop();
						//std::unique_ptr<VirtualFileSource_PhysFS> fileSource(new VirtualFileSource_PhysFS());
						CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
						auto file = dir.open_file("default.gad", CL_File::create_always, CL_File::access_write);
						GameMap::CompileMap(file, streamingMgr->GetNumCellsAcross(), streamingMgr->GetMapWidth(), streamingMgr->GetCellSize(), cellArchivist.get(), entities);
						cellArchivist->Start();

						streamingMgr->Update(true);
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

					//if (int(rand() / (float)RAND_MAX * 16) == 15)
					//{
					//	auto sleeptime = int(rand() / (float)RAND_MAX * 31);
					//	if (sleeptime > 22)
					//		CL_System::pause(sleeptime);
					//}

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

					Profiling::getSingleton().AddTime("Buffer size", (unsigned long)0);
					Profiling::getSingleton().AddTime("Incomming Packets", (unsigned long)0);
					Profiling::getSingleton().AddTime("Packets Processed", (unsigned long)0);
					
					profiling->AddTime("ActualDT", (unsigned long)delta);
					// Record profiling data
					profiling->StoreTick();

					if (dispWindow.get_ic().get_keyboard().get_keycode(CL_KEY_ESCAPE))
						keepGoing = false;
				}
				}
				catch (...)
				{
					cellArchivist->Stop();
					throw;
				}
				cellArchivist->Stop();
				scriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);
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
			catch(...)
			{
#ifdef _WIN32
				MessageBoxA(dispWindow.get_hwnd(), "Unknown error", "Exception", MB_OK);
#endif
			}

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
			auto id = instantiationSynchroniser->m_WorldIdGenerator.getFreeID();
			entity->SetID(id);

			std::stringstream str;
			str << i << "_" << id;
			entity->SetName("edit" + str.str());
		}
		else
		{
			std::stringstream str;
			str << i;
			entity->SetName("edit" + str.str());
		}

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
