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
#include "../FusionEngine/FusionGameMapLoader.h"
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
			: m_NewData(false),
			m_Instantiator(nullptr)
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
				if (m_Map)
				{
					cell->AddHist("Enqueued Out");
					m_WriteQueue.push(std::make_tuple(cell, i));
					m_NewData.set();
				}
				else
				{
					cell->waiting = Cell::Ready;
				}
			}
		}

		bool Retrieve(Cell* cell, size_t i)
		{
			if (cell->waiting.fetch_and_store(Cell::Retrieve) != Cell::Retrieve && !cell->loaded)
			{
				if (m_Map)
				{
					cell->AddHist("Enqueued In");
					m_ReadQueue.push(std::make_tuple(cell, i));
					m_NewData.set();
				}
				else
				{
					cell->loaded = true;
					cell->waiting = Cell::Ready;
				}
				return true;
			}
			return false;
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

		void WriteComponent(CL_IODevice& out, IComponent* component)
		{
			RakNet::BitStream stream;
			const bool conData = component->SerialiseContinuous(stream);
			const bool occData = component->SerialiseOccasional(stream, IComponent::All);

			out.write_uint8(conData ? 0xFF : 0x00); // Flag indicating data presence
			out.write_uint8(occData ? 0xFF : 0x00);

			out.write_uint32(stream.GetNumberOfBytesUsed());
			out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
		}

		void ReadComponent(CL_IODevice& in, IComponent* component)
		{
			const bool conData = in.read_uint8() != 0x00;
			const bool occData = in.read_uint8() != 0x00;

			const auto dataLen = in.read_uint32();
			std::vector<unsigned char> data(dataLen);
			in.read(data.data(), data.size());

			RakNet::BitStream stream(data.data(), data.size(), false);

			if (conData)
				component->DeserialiseContinuous(stream);
			if (occData)
				component->DeserialiseOccasional(stream, IComponent::All);

			//stream.AssertStreamEmpty();
			if (stream.GetNumberOfUnreadBits() >= 8)
				SendToConsole("Not all serialised data was used when reading a " + component->GetType());
		}

		void Save(CL_IODevice& out, EntityPtr entity)
		{
			ObjectID id = entity->GetID();
			out.write(&id, sizeof(ObjectID));

			{
				RakNet::BitStream stream;
				entity->SerialiseReferencedEntitiesList(stream);

				out.write_uint32(stream.GetNumberOfBytesUsed());
				out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
			}

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // - transform

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			out.write_string_a(transform->GetType());
			WriteComponent(out, transform);

			out.write(&numComponents, sizeof(size_t));
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				auto& component = *it;
				if (component.get() != transform)
					out.write_string_a(component->GetType());
			}
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				auto& component = *it;
				if (component.get() != transform)
					WriteComponent(out, component.get());
			}
		}

		EntityPtr Load(CL_IODevice& in)
		{
			ObjectID id;
			in.read(&id, sizeof(ObjectID));
			m_Instantiator->TakeID(id);

			const auto dataLen = in.read_uint32();
			std::vector<unsigned char> referencedEntitiesData(dataLen);
			in.read(referencedEntitiesData.data(), referencedEntitiesData.size());

			ComponentPtr transform;
			{
				std::string transformType = in.read_string_a();
				transform = m_Instantiator->m_Factory->InstanceComponent(transformType);

				ReadComponent(in, transform.get());
			}

			auto entity = std::make_shared<Entity>(m_Instantiator->m_EntityManager, &m_Instantiator->m_EntityManager->m_PropChangedQueue, transform);
			entity->SetID(id);

			transform->SynchronisePropertiesNow();

			{
				RakNet::BitStream stream(referencedEntitiesData.data(), referencedEntitiesData.size(), false);
				entity->DeserialiseReferencedEntitiesList(stream, EntityDeserialiser(m_Instantiator->m_EntityManager));
			}

			size_t numComponents;
			in.read(&numComponents, sizeof(size_t));
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string type = in.read_string_a();
				auto& component = m_Instantiator->m_Factory->InstanceComponent(type);
				entity->AddComponent(component);
			}
			if (numComponents != 0)
			{
				auto& components = entity->GetComponents();
				auto it = components.begin(), end = components.end();
				for (++it; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transform);

					ReadComponent(in, component.get());
				}
			}
			
			return entity;
		}

		CL_IODevice GetFile(size_t cell_index, bool write)
		{
			try
			{
				std::stringstream str; str << cell_index;
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				auto file = vdir.open_file("cache/" + str.str(), write ? CL_File::create_always : CL_File::open_existing, write ? CL_File::access_write : CL_File::access_read);//"cache/partialsave/"
				return file;
			}
			catch (CL_Exception&)
			{
				//SendToConsole(ex.what());
				return CL_IODevice();
			}
		}

		void Run()
		{
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
							if (cell->active_entries == 0 && cell->waiting == Cell::Store)
							{
								try
								{
									FSN_ASSERT(cell->loaded == true); // Don't want to create an inaccurate cache (without entities from the map file)
									auto file = GetFile(i, true);
									size_t numEntries = static_cast<size_t>(std::count_if(cell->objects.begin(), cell->objects.end(), [](const Cell::CellEntryMap::value_type& obj) { return obj.first->IsSyncedEntity(); }));
									file.write(&numEntries, sizeof(size_t));
									for (auto it = cell->objects.cbegin(), end = cell->objects.cend(); it != end; ++it)
									{
										if (it->first->IsSyncedEntity())
										{
											Save(file, it->first);
										}
									}

									cell->AddHist("Written and cleared", numEntries);

									//std::stringstream str; str << i;
									//SendToConsole("Cell " + str.str() + " streamed out");

									cell->objects.clear();
									cell->loaded = false;
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
									m_Map->LoadCell(cell, i, file.is_null(), m_Instantiator->m_Factory, m_Instantiator->m_EntityManager, m_Instantiator); 

									if (!file.is_null() && file.get_size() > 0)
									{
										size_t numEntries;
										file.read(&numEntries, sizeof(size_t));
										for (size_t n = 0; n < numEntries; ++n)
										{
											//auto& archivedEntity = *it;
											auto archivedEntity = Load(file);

											Vector2 pos = archivedEntity->GetPosition();
											// TODO: Cell::Add(entity, CellEntry = def) rather than this bullshit
											CellEntry entry;
											entry.x = ToGameUnits(pos.x); entry.y = ToGameUnits(pos.y);

											archivedEntity->SetStreamingCellIndex(i);

											cell->objects.push_back(std::make_pair(archivedEntity, std::move(entry)));
										}

										//std::stringstream str; str << i;
										//SendToConsole("Cell " + str.str() + " streamed in");

										cell->AddHist("Loaded", numEntries);
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

		std::map<size_t, std::vector<EntityPtr>> m_Archived;

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
		FSN_ASSERT(SetupPhysFS::is_init());

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

				if (options->GetOption_bool("console_logging"))
					logger->ActivateConsoleLogging();

				bool editMode = options->GetOption_bool("edit");

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
				std::unique_ptr<SimpleCellArchiver> cellArchivist(new SimpleCellArchiver());
				std::unique_ptr<EntitySynchroniser> entitySynchroniser(new EntitySynchroniser(inputMgr.get()));
				std::unique_ptr<StreamingManager> streamingMgr(new StreamingManager(cellArchivist.get()));
				std::unique_ptr<EntityManager> entityManager(new EntityManager(inputMgr.get(), entitySynchroniser.get(), streamingMgr.get()));
				std::unique_ptr<InstancingSynchroniser> instantiationSynchroniser(new InstancingSynchroniser(entityFactory.get(), entityManager.get()));

				cellArchivist->SetSynchroniser(instantiationSynchroniser.get());

				try
				{
				scriptManager->RegisterGlobalObject("StreamingManager streaming", streamingMgr.get());

				entityManager->m_EntityFactory = entityFactory.get();

				std::unique_ptr<GameMapLoader> mapLoader(new GameMapLoader(options, entityFactory.get(), entityManager.get(), std::make_shared<VirtualFileSource_PhysFS>()));

				if (!options->GetOption_bool("edit"))
				{
					auto map = mapLoader->LoadMap("default.gad", instantiationSynchroniser.get());
					cellArchivist->SetMap(map);
				}

				cellArchivist->Start();

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


				PropChangedQueue &propChangedQueue = entityManager->m_PropChangedQueue;

				PlayerRegistry::AddLocalPlayer(1u, 0u);

				// This scope makes viewport hold the only reference to camera: thus camera will be deleted with viewport
				{
				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				auto viewport = std::make_shared<Viewport>(CL_Rectf(0.f, 0.f, 1.f, 1.f), camera);
				dynamic_cast<CLRenderWorld*>(renderWorld.get())->AddViewport(viewport);
				streamingMgr->AddCamera(camera);
				}

				std::vector<EntityPtr> entities;

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

						auto entity =
							createEntity((unsigned int)(ev.id - CL_KEY_0), pos, instantiationSynchroniser.get(), entityFactory.get(), entityManager.get());
						entities.push_back(entity);
					}

					if (ev.id == CL_KEY_S)
					{
						//std::unique_ptr<VirtualFileSource_PhysFS> fileSource(new VirtualFileSource_PhysFS());
						CL_VirtualDirectory dir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
						auto file = dir.open_file("default.gad", CL_File::create_always, CL_File::access_write);
						GameMap::CompileMap(file, streamingMgr->GetNumCellsAcross(), 500u, entities);
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
					}
					
					const auto executed = scheduler->Execute(editMode ? SystemType::Rendering : (SystemType::Rendering | SystemType::Simulation));

					if (executed & SystemType::Rendering)
					{
						dispWindow.flip(0);
						gc.clear();
					}
					
					if (executed & SystemType::Simulation || editMode)
					{
						// Actually activate / deactivate components
						entityManager->ProcessActivationQueues();

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

	EntityPtr createEntity(unsigned int i, Vector2 position, InstancingSynchroniser* instantiationSynchroniser, EntityFactory* factory, EntityManager* entityManager)
	{
		position.x = ToSimUnits(position.x); position.y = ToSimUnits(position.y);

		ComponentPtr transformCom;
		if (i == 1)
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

		entityManager->AddEntity(entity);

		ComponentPtr b2CircleFixture;
		if (i == 2 || i == 3 || i == 4)
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
			entity->AddComponent(asScript2, "script_b");
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
			auto script = entity->GetComponent<IScript>("script_b");
			if (script)
				script->ScriptPath.Set("Scripts/TestB.as");
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
