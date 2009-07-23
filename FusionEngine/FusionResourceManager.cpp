
#include "Common.h"

/// Class
#include "FusionResourceManager.h"

#include "FusionResourceManagerScriptFunctions.h"

/// Fusion
#include "FusionConsole.h"
#include "FusionLogger.h"

#include "FusionPaths.h"
#include "FusionPhysFS.h"

#include "FusionImageLoader.h"
#include "FusionAudioLoader.h"

#include "FusionScriptingEngine.h"
#include "FusionScriptTypeRegistrationUtils.h"

namespace FusionEngine
{

	ResourceManager::ResourceManager(const CL_GraphicContext &gc)
		: m_GC(gc),
		m_PhysFSConfigured(false),
		m_StopEvent(false),
		m_ToLoadEvent(false),
		m_ToUnloadEvent(false)
	{
		bool ok = SetupPhysFS::init(fe_narrow(CL_System::get_exe_path()).c_str());
		FSN_ASSERT(ok);

		Configure();
	}

	ResourceManager::ResourceManager(const CL_GraphicContext &gc, char *arg0)
		: m_GC(gc),
		m_PhysFSConfigured(false),
		m_StopEvent(false),
		m_ToLoadEvent(false),
		m_ToUnloadEvent(false)
	{
		int ok = SetupPhysFS::init(arg0);
		FSN_ASSERT(ok);

		Configure();
	}

	ResourceManager::~ResourceManager()
	{
		StopBackgroundPreloadThread();

		ClearAll();

		SetupPhysFS::deinit();
	}

	// Look, nice formatting :D
	///////////
	/// Public:
	void ResourceManager::Configure()
	{
		SetupPhysFS::configure("Pom", "Fusion", "7z");
		if (!SetupPhysFS::mount(s_PackagesPath, "", "7z", false))
			SendToConsole("Default resource path could not be located");

		m_PhysFSConfigured = true;

		AddDefaultLoaders();
	}

	void ResourceManager::StartBackgroundPreloadThread()
	{
		m_Worker.start(this, &ResourceManager::BackgroundPreload, m_GC);
	}

	void ResourceManager::StopBackgroundPreloadThread()
	{
		m_StopEvent.set();
		m_Worker.join();
	}

	StringVector ResourceManager::ListFiles()
	{
		StringVector list;

		// Native (OS) filesystem (removed, should only be used if PhysFS is not present)
		//! \todo Check if physfs is available and use native directory scanner if not
		//CL_DirectoryScanner scanner;
		//if (scanner.scan(PackagesPath, "*.xml"))
		//{
		//	while (scanner.next())
		//	{
		//		list.push_back(scanner.get_name());
		//	}
		//}

		// List all files (remember, physfs will also find files in the native filesystem)
		char **files = PHYSFS_enumerateFiles("");
		if (files != NULL)
		{
			int file_count;
			char **i;
			for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				list.push_back(std::string(*i));

			PHYSFS_freeList(files);
		}

		return list;
	}

	void ResourceManager::DisposeUnusedResources()
	{
		// not yet implemented
#ifdef RESMAN_USEGARBAGELIST
		ResourceGarbageList::iterator it = m_Garbage.begin();
		for (; it != m_Garbage.end(); ++it)
		{
			Resource res = m_Resources.find(*it);
			if (res != m_Resources.end())
				m_Resources.erase(res);
		}
		m_Garbage.clear();
#else

		CL_MutexSection resourcesLock(&m_ResourcesMutex);
		for (ResourceMap::iterator it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			ResourceSpt& res = (*it).second;

#if defined(FSN_RESOURCEPOINTER_USE_WEAKPTR)

			if ( !res->IsReferenced() )
			{

#else

			if (res.unique())
			{

#endif
				unloadResource(res, m_GC);
			}
		}

#endif
	}

	void ResourceManager::DeleteResources()
	{
		CL_MutexSection resourcesLock(&m_ResourcesMutex);
		for (ResourceMap::iterator it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			unloadResource(it->second, m_GC, true);
		}
		// There may still be ResourcePointers holding shared_ptrs to the
		//  ResourceContainers held in m_Resources, but they will simply
		//  be invalidated by the previous step. In any case, the following
		//  step means all ResourceContainers will be deleated ASAP.
		m_Resources.clear();
	}

	void ResourceManager::ClearAll()
	{
		DeleteResources();
	}

	std::string ResourceManager::FindFirst(const std::string &expression, bool recursive, bool case_sensitive)
	{
		return FindFirst("", expression, 0, case_sensitive, recursive);
	}

	std::string ResourceManager::FindFirst(const std::string &path, const std::string &expression, int depth, bool recursive, bool case_sensitive)
	{
		if (m_PhysFSConfigured)
		{
			// Compile the regular expression
			CL_RegExp regexp(expression.c_str(), (case_sensitive ? 0 : CL_RegExp::compile_caseless));

			char **files = PHYSFS_enumerateFiles(path.c_str());
			if (files != NULL)
			{
				char **i;
				for (i = files; *i != NULL; i++)
				{
					std::string file(*i);
					if (regexp.search(file.c_str(), file.length()).is_match())
						return file;
				}

				// If recursive is set (or depth > 0), search within sub-folders
				if (recursive || depth--)
				{
					for (i = files; *i != NULL; i++)
					{
						if (PHYSFS_isDirectory(*i))
						{
							std::string subMatch = FindFirst(std::string(*i), expression, depth, case_sensitive, recursive);
							if (!subMatch.empty())
								return subMatch;
						}
					}
				}

				PHYSFS_freeList(files);
			}
		}

		return "";
	}

	StringVector ResourceManager::Find(const std::string &expression, bool recursive, bool case_sensitive)
	{
		return Find("", expression, 0, case_sensitive, recursive);
	}

	StringVector ResourceManager::Find(const std::string &path, const std::string &expression, int depth, bool recursive, bool case_sensitive)
	{
		StringVector list;

		if (m_PhysFSConfigured)
		{
			// Compile the regular expression
			CL_RegExp regexp(expression.c_str(), (case_sensitive ? 0 : CL_RegExp::compile_caseless));

			char **files = PHYSFS_enumerateFiles(path.c_str());
			if (files != NULL)
			{
				int file_count;
				char **i;
				for (i = files, file_count = 0; *i != NULL; i++, file_count++)
				{
					// If recursive is set (or depth > 0), search within sub-folders
					if ((recursive || depth--) && PHYSFS_isDirectory(*i))
					{
						StringVector subList = Find(std::string(*i), expression, depth, case_sensitive, recursive);
						list.insert(list.end(), subList.begin(), subList.end());
					}

					std::string file(*i);
					if (regexp.search(file.c_str(), file.length()).is_match())
						list.push_back(file);
				}

				PHYSFS_freeList(files);
			}
		}

		return list;
	}

	void ResourceManager::AddDefaultLoaders()
	{
		AddResourceLoader("IMAGE", &LoadImageResource, &UnloadImageResouce, NULL);
		AddResourceLoader("AUDIO", &LoadAudio, &UnloadAudio, NULL);
		AddResourceLoader("AUDIO:STREAM", &LoadAudioStream, &UnloadAudio, NULL); // Uses the same unload method
	}

	void ResourceManager::AddResourceLoader(const ResourceLoader &loader)
	{
		m_LoaderMutex.lock();
		m_ResourceLoaders[loader.type] = loader;
		m_LoaderMutex.unlock();
	}

	void ResourceManager::AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, void* userData)
	{
		m_LoaderMutex.lock();
		m_ResourceLoaders[type] = ResourceLoader(type, loadFn, unloadFn, userData);
		m_LoaderMutex.unlock();
	}

	void ResourceManager::AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, resource_unload qlDataUnloadFn, void* userData)
	{
		m_LoaderMutex.lock();
		m_ResourceLoaders[type] = ResourceLoader(type, loadFn, unloadFn, qlDataUnloadFn, userData);
		m_LoaderMutex.unlock();
	}


	void ResourceManager::BackgroundPreload(CL_GraphicContext gc)
	{
		CL_SharedGCData::add_ref();
		CL_GraphicContext loadingGC = gc.create_worker_gc();
		while (true)
		{
			// Wait until there is more to load, or a stop event is received
			int receivedEvent = CL_Event::wait(m_StopEvent, m_ToLoadEvent, m_ToUnloadEvent);
			// 0 = stop event, -1 = error (otherwise it is
			//  a ToLoadEvent, meaning the load loop below should resume)
			if (receivedEvent <= 0)
				break;

			// Load
			CL_MutexSection toLoadMutexSection(&m_ToLoadMutex);
			while (!m_ToLoad.empty())
			{
				ResourceToLoadData toLoadData = m_ToLoad.top();

				toLoadMutexSection.unlock();

				// Don't bother loading if it has been added to the unload list already
				CL_MutexSection toUnloadMutexSection(&m_ToUnloadMutex);
				ToUnloadList::iterator _where = m_ToUnload.find(toLoadData.tag);
				if (_where == m_ToUnload.end())
				{
					try
					{
						//CL_SharedGCData::add_ref();
						TagResource(toLoadData.type, toLoadData.path, toLoadData.tag, &loadingGC);
						//CL_SharedGCData::release_ref();
					}
					catch (FileSystemException &ex)
					{
						//! \todo Call error handler ('m_ErrorHandler' should be in ResourceManager)
						Logger::getSingleton().Add(ex.GetDescription(), "ResourceManager", LOG_NORMAL);
					}
				}

				toLoadMutexSection.lock();
				m_ToLoad.pop();
			}

			// Unload
			CL_MutexSection toUnloadMutexSection(&m_ToUnloadMutex);
			for (ToUnloadList::iterator it = m_ToUnload.begin(), end = m_ToUnload.end(); it != end; ++it)
			{
				UnloadResource(*it);
			}
			m_ToUnload.clear();
		}

		asThreadCleanup();
		CL_SharedGCData::release_ref();
	}

	ResourceSpt ResourceManager::TagResource(const std::string& type, const std::wstring& path, const ResourceTag& tag, CL_GraphicContext *gc)
	{
		CL_MutexSection resourcesLock(&m_ResourcesMutex);

		ResourceSpt resource;

		// Check whether there is an existing tag pointing to the same resource
		ResourceMap::iterator existing = m_Resources.find(tag);
		if (existing != m_Resources.end() && existing->second->GetPath() == path)
		{
			resource = existing->second; // Tag points to the same resource
		}
		else
		{
			// Create a new resource container
			resource = ResourceSpt(new ResourceContainer(type, tag, path, NULL));
		}

		if (gc != NULL)
			loadResource(resource, *gc);
		else
			loadResource(resource, m_GC);

		return m_Resources[tag] = resource;
	}

	ResourceSpt ResourceManager::PreloadResource(const std::string& type, const std::wstring& path)
	{
		return TagResource(type, path, path);
	}

	ResourceSpt ResourceManager::PreloadResource_Background(const std::string& type, const std::wstring& path, int priority)
	{
		//ResourceSpt resource = ResourceSpt(new ResourceContainer(type, path, path, NULL));
		//m_Resources[path] = resource;
		ResourceSpt &resource = GetResourceDefault(path, type);

		m_ToLoadMutex.lock();
		m_ToLoad.push(ResourceToLoadData(type, path, path, priority));
		m_ToLoadMutex.unlock();
		
		m_ToLoadEvent.set();

		return resource;
	}

	void ResourceManager::UnloadResource(const std::wstring &path)
	{
		CL_MutexSection resourcesMutexSection(&m_ResourcesMutex);
		ResourceMap::iterator _where = m_Resources.find(path);
		if (_where != m_Resources.end())
			unloadResource(_where->second, m_GC);
	}

	void ResourceManager::UnloadResource_Background(const std::wstring &path)
	{
		m_ToUnloadMutex.lock();
		m_ToUnload.insert(path);
		m_ToUnloadMutex.unlock();

		m_ToUnloadEvent.set();
	}

	ResourceSpt &ResourceManager::GetResourceDefault(const ResourceTag &tag, const std::string &type)
	{
		ResourceMap::iterator _where = m_Resources.find(tag);
		if (_where != m_Resources.end())
			return _where->second;
		else
		{ 
			return m_Resources[tag] = ResourceSpt(new ResourceContainer(type, tag, tag, NULL));
		}
	}

	//void ResourceManager::RegisterScriptElements(ScriptingEngine* manager)
	//{
	//	asIScriptEngine* engine = manager->GetEnginePtr();
	//	int r;
	//	r = engine->RegisterObjectType("ResourceManager", sizeof(ResourceManager), asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
	//	r = engine->RegisterObjectMethod("ResourceManager", "void Cache(string &in, string &in)", asMETHOD(ResourceManager, PreloadResource), asCALL_THISCALL); assert( r >= 0 );

	//	// TODO: put all this type registration stuff into another header, then implement a 'plugin' type system
	//	//  where new script types can be passed in a simmilar way to ResourceLoaders
	//	registerXMLType(engine);
	//	registerImageType(engine);
	//	registerSoundType(engine);

	//	r = engine->RegisterObjectMethod("ResourceManager", "Image GetImage(string &in)", asFUNCTION(ResourceManager_GetImage), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	//	r = engine->RegisterObjectMethod("ResourceManager", "Sound GetSound(string &in)", asFUNCTION(ResourceManager_GetSound), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	//	r = engine->RegisterObjectMethod("ResourceManager", "XmlDocument GetXML(string &in)", asFUNCTION(ResourceManager_GetXml), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	//	r = engine->RegisterObjectMethod("ResourceManager", "string GetText(string &in)", asFUNCTION(ResourceManager_GetText), asCALL_CDECL_OBJLAST); assert( r >= 0 );


	//	r = engine->RegisterGlobalProperty("ResourceManager resource_manager", this);
	//	r = engine->RegisterGlobalProperty("ResourceManager file", this);
	//}

	//template<typename T>
	//ResourcePointer<T> ResourceManager::GetResource(const ResourceTag &tag)
	//{
	//	PreloadResource(GetResourceType(T).c_str(), tag);

	//	Resource& res = (*m_Resources[tag]);
	//	if (!res.IsValid())
	//	{
	//		m_ResourceLoaders[res.GetType()]->ReloadResource(res);
	//	}

	//	return ResourcePointer<T>(res);
	//}

	////////////
	/// Private:
	void ResourceManager::loadResource(ResourceSpt &resource, CL_GraphicContext &gc)
	{
		CL_MutexSection loaderMutexSection(&m_LoaderMutex);
		ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());
		if (_where == m_ResourceLoaders.end())
		{
			FSN_EXCEPT(ExCode::FileType, "ResourceManager::PreloadResource", "Attempted to load unknown resource type '" + resource->GetType() + "'");
		}

		// Initialize a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		// Run the load function
		ResourceLoader &loader = _where->second;
		loader.load(resource.get(), vdir, gc, loader.userData);
	}

	void ResourceManager::unloadResource(ResourceSpt &resource, CL_GraphicContext &gc, bool quickload)
	{
		CL_MutexSection loaderMutexSection(&m_LoaderMutex);
		ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());

		if (_where != m_ResourceLoaders.end())
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the load function
			ResourceLoader &loader = _where->second;
			loader.unload(resource.get(), vdir, gc, loader.userData);
			if (quickload)
				loader.unloadQLData(resource.get(), vdir, gc, loader.userData);
		}
	}

	//void ResourceManager::registerXMLType(asIScriptEngine* engine)
	//{
	//	int r;

	//	RegisterResourcePointer<TiXmlDocument>("XmlDocument", engine);

	//	RegisterTypePOD<TiXmlNode>("XmlNode", engine);

	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"string find(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"string xpath_string(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"double xpath_double(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_Double, (std::string&), double),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"float xpath_float(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_Float, (std::string&), float),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"int xpath_int(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&), int),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath_int(string& in)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");

	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, string &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedString, (std::string&, std::string&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, double &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedDouble, (std::string&, double&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, float &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedFloat, (std::string&, float&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, int &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_Int, (std::string&, int&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, uint &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedUInt, (std::string&, unsigned int&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");
	//	r = engine->RegisterObjectMethod("XmlDocument",
	//		"bool xpath(string &in, bool &out)",
	//		asFUNCTIONPR(XML_ExecuteXPathExpr_CheckedBool, (std::string&, bool&), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register a method");

	//	//r = engine->RegisterObjectMethod("XmlDocument",
	//	//	"string& get_root_name()",
	//	//	asFUNCTIONPR(XML_GetRootName, (std::string&), std::string&),
	//	//	asCALL_CDECL_OBJFIRST);

	//	//r = engine->RegisterObjectMethod("XmlDocument",
	//	//	"XmlNode get_root()",
	//	//	asFUNCTIONPR(XML_GetRoot, (void), TiXmlNode),
	//	//	asCALL_CDECL_OBJFIRST);

	//	//r = engine->RegisterObjectMethod("XmlDocument",
	//	//	"XmlNode xpath_node(string &in)",
	//	//	asFUNCTIONPR(XML_GetRoot, (void), TiXmlNode),
	//	//	asCALL_CDECL_OBJFIRST);
	//	//r = engine->RegisterObjectMethod("XmlDocument",
	//	//	"string& find(XmlNode &in, string& in)",
	//	//	asFUNCTIONPR(XML_ExecuteXPathExpr_String, (std::string&), std::string&),
	//	//	asCALL_CDECL_OBJFIRST);
	//	// /document/element will get the element name
	//	// /document/element/ will get the text child (if one exists)


	//	//r = engine->RegisterObjectMethod("XmlNode",
	//	//	"string& get_text()",
	//	//	asFUNCTIONPR(XMLNode_GetText, (void), std::string&),
	//	//	asCALL_CDECL_OBJFIRST);
	//	//assert(r >= 0 && "Failed to register get_text()");

	//	//r = engine->RegisterObjectMethod("XmlNode",
	//	//	"string& get_attribute(string &in)",
	//	//	asFUNCTIONPR(TiXmlNode, (std::string&), std::string&),
	//	//	asCALL_CDECL_OBJFIRST);
	//	//assert(r >= 0 && "Failed to register get_text()");
	//}

	//void ResourceManager::registerImageType(asIScriptEngine* engine)
	//{
	//	RegisterResourcePointer<CL_Texture>("Image", engine);

	//	int r;
	//	// draw(x, y)
	//	//r = engine->RegisterObjectMethod("Image",
	//	//	"void draw(float, float)",
	//	//	asFUNCTIONPR(Image_Draw, (float, float), void),
	//	//	asCALL_CDECL_OBJFIRST);
	//	//assert(r >= 0 && "Failed to register draw()");
	//	//// draw(x, y, angle)
	//	//r = engine->RegisterObjectMethod("Image",
	//	//	"void draw(float, float, float)",
	//	//	asFUNCTIONPR(Image_Draw_Angle, (float, float, float), void),
	//	//	asCALL_CDECL_OBJFIRST);
	//	//assert(r >= 0 && "Failed to register draw()");
	//}

	//void ResourceManager::registerSoundType(asIScriptEngine* engine)
	//{
	//	RegisterType<CL_SoundBuffer_Session>("SoundSession", engine);

	//	int r;
	//	r = engine->RegisterObjectMethod("SoundSession",
	//		"void play()",
	//		asMETHOD(CL_SoundBuffer_Session, play),
	//		asCALL_THISCALL);
	//	assert(r >= 0 && "Failed to register play()");
	//	r = engine->RegisterObjectMethod("SoundSession",
	//		"void stop()",
	//		asMETHOD(CL_SoundBuffer_Session, stop),
	//		asCALL_THISCALL);
	//	assert(r >= 0 && "Failed to register stop()");
	//	r = engine->RegisterObjectMethod("SoundSession",
	//		"bool is_playing()",
	//		asMETHOD(CL_SoundBuffer_Session, is_playing),
	//		asCALL_THISCALL);
	//	assert(r >= 0 && "Failed to register is_playing()");


	//	RegisterResourcePointer<CL_SoundBuffer>("Sound", engine);

	//	r = engine->RegisterObjectMethod("Sound",
	//		"SoundSession prepare(bool)",
	//		asFUNCTIONPR(Sound_Prepare, (bool), CL_SoundBuffer_Session),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register prepare()");
	//	r = engine->RegisterObjectMethod("Sound",
	//		"SoundSession play(bool)",
	//		asFUNCTIONPR(Sound_Play, (bool), CL_SoundBuffer_Session),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register play()");
	//	r = engine->RegisterObjectMethod("Sound",
	//		"void stop()",
	//		asFUNCTIONPR(Sound_Stop, (void), void),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register stop()");
	//	r = engine->RegisterObjectMethod("Sound",
	//		"bool is_playing()",
	//		asFUNCTIONPR(Sound_IsPlaying, (void), bool),
	//		asCALL_CDECL_OBJFIRST);
	//	assert(r >= 0 && "Failed to register is_playing()");
	//}

	bool ResourceManager::checkInList(const std::string &filename, std::vector<std::string> filelist)
	{
		std::vector<std::string>::iterator it;
		for (it = filelist.begin(); it != filelist.end(); ++it)
		{
			if (filename == (*it))
				return true;
		}

		return false;
	}


	//TiXmlDocument* ResourceManager::OpenPackage(const std::string &name)
	//{
	//	if (m_PhysFSConfigured)
	//	{
	//		Find("*.xml");
	//	}
	//}

	//bool ResourceManager::LoadPackage(const std::string &name)
	//{
	//	TiXmlDocument* doc = OpenPackage(name);

	//	TiXmlNode* root = doc->RootElement();
	//	m_RootRNode.AddChildNode(createResourceNode(root));

	//	// The root node is simply called "" (nothing), thus the paths for finding
	//	//  resources always start with "/Fusion/", the initial "/" signifying the 
	//	//  trailing slash after the nameless root node ;)

	//	return true;
	//}

	//bool ResourceManager::LoadPackages(StringVector names)
	//{
	//	StringVector::iterator it;
	//	for (it = names.begin(); it != names.end(); ++it)
	//	{
	//		if (!LoadPackage(*it))
	//			return false;
	//	}

	//	return true;
	//}

	//bool ResourceManager::LoadVerified()
	//{
	//	return LoadPackages(m_VerifiedPackages);
	//}

	//void ResourceManager::ResetVerified()
	//{
	//	m_VerifiedPackages.clear();
	//}


	//RNode ResourceManager::createResourceNode(TiXmlElement* xmlNode)
	//{
	//	RNode* node;

	//	TiXmlAttribute* typeAttr = xmlNode->Attribute("type");
	//	if (typeAttr != NULL)
	//		loadResource(typeAttr->Value(), xmlNode->FirstChild()->Value());

	//	TiXmlElement *child;
	//	for (child = xmlNode->FirstChildElement(); child; child = child->NextSiblingElement())
	//	{
	//		node.AddChildNode( createResourceNode(child) );
	//	}

	//	return node;
	//}

	//void ResourceManager::loadResource(const char* type, const char* text)
	//{
	//}

	//CL_Point ResourceManager::getPoint(const CL_DomElement *element)
	//{
	//	// Return a zero point if the data is incomplete
	//	if (!(element->has_attribute("x") & element->has_attribute("y")))
	//		return CL_Point(0, 0);

	//	int x = CL_String::to_int(element->get_attribute("x"));
	//	int y = CL_String::to_int(element->get_attribute("y"));

	//	return CL_Point(x, y);
	//}


}
