
#include "FusionStableHeaders.h"

#include "FusionResourceManager.h"

#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/regexp.h>

#include "FusionConsole.h"
#include "FusionExceptionFactory.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionScriptManager.h"

namespace FusionEngine
{

	ResourceManager::ResourceManager(const CL_GraphicContext &gc)
		: m_StopEvent(false),
		m_ToLoadEvent(false),
		m_ToUnloadEvent(false),
		m_Clearing(false),
		m_GC(gc)
	{
	}

	ResourceManager::~ResourceManager()
	{
		StopLoaderThread();

		DeleteAllResources(m_GC);
	}

	const CL_GraphicContext& ResourceManager::GetGC() const
	{
		return m_GC;
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

	void ResourceManager::StartLoaderThread()
	{
		m_Worker.start(this, &ResourceManager::Load);
	}

	void ResourceManager::StopLoaderThread()
	{
		m_StopEvent.set();
		m_Worker.join();
	}

	void ResourceManager::loadResourceAndDeps(ResourceDataPtr& resource, unsigned int depth_limit)
	{
		if (depth_limit == 0)
		{
			FSN_EXCEPT(FileSystemException, "Dependency tree for '" + resource->GetPath() + "' is too deep: it's probably circular");
		}

		// TODO: prevent loaders from being added after the Load thread has been started (make this mutex unnecessary)
		//CL_MutexSection loaderMutexSection(&m_LoaderMutex);

		auto _where = m_ResourceLoaders.find(resource->GetType());
		if (_where == m_ResourceLoaders.end())
		{
			FSN_EXCEPT(FileTypeException, "Attempted to load unknown resource type '" + resource->GetType() + "'");
		}
		ResourceLoader& loader = _where->second;

		if (loader.list_prereq != nullptr)
		{
			DepsList prereqs;
			loader.list_prereq(resource.get(), prereqs, loader.userData);

			for (auto it = prereqs.begin(), end = prereqs.end(); it != end; ++it)
			{
				ResourceDataPtr dep;
				obtainResource(dep, it->first, it->second);
				loadResourceAndDeps(dep, depth_limit - 1);

				resource->AttachDependency(dep);
			}
		}

		// Initialize a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		// Run the load function
		loader.load(resource.get(), vdir, loader.userData);
	}

	//void ResourceManager::loadResourceAndDepsIterative()
	//{
	//	ResourceDataPtr toLoadRes;
	//	dependencies.push_back(toLoadData.resource);

	//	CL_MutexSection loaderMutexSection(&m_LoaderMutex);

	//	unsigned int depth = 0;
	//	while (!dependencies.empty());
	//	{
	//		toLoadRes = dependencies.back();
	//		dependencies.pop_back();

	//		auto _where = m_ResourceLoaders.find(toLoadRes->GetType());
	//		if (_where == m_ResourceLoaders.end())
	//		{
	//			FSN_EXCEPT(FileTypeException, "Attempted to load unknown resource type '" + toLoadRes->GetType() + "'");
	//		}
	//		ResourceLoader& loader = _where->second;

	//		if (loader.list_prereq != nullptr)
	//		{
	//			if (++depth > 6)
	//			{
	//				FSN_EXCEPT(FileSystemException, "Dependency tree for '" + toLoadRes->GetPath() + "' is too deep, it's probably circular");
	//			}
	//			DepsList prereqs;
	//			loader.list_prereq(toLoadRes.get(), prereqs, loader.userData);

	//			for (auto it = prereqs.begin(), end = prereqs.end(); it != end; ++it)
	//			{
	//				ResourceDataPtr dep;
	//				obtainResource(dep, it->first, it->second);
	//				dependencies.push_back(dep.get());

	//				toLoadRes->AttachDependency(dep);
	//			}
	//		}
	//	}

	//	// Initialize a vdir
	//	CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

	//	// Run the load function
	//	ResourceLoader &loader = _where->second;
	//	loader.load(resource.get(), vdir, loader.userData);
	//}

	void ResourceManager::Load()
	{
		auto logfile = Logger::getSingleton().OpenLog("ResourceManager");

		while (true)
		{
			// Wait until there is more to load, or a stop event is received
			int receivedEvent = CL_Event::wait(m_StopEvent, m_ToLoadEvent, m_ToUnloadEvent);
			// 0 = stop event, -1 = error; otherwise, it is
			//  a ToLoad / ToUnload Event, meaning the load loop below should resume
			if (receivedEvent <= 0)
				break;

			// Load
			{
				CL_MutexSection toLoadMutexSection(&m_ToLoadMutex);
				while (!m_ToLoad.empty())
				{
					ResourceToLoadData toLoadData = m_ToLoad.top();
					m_ToLoad.pop();

					toLoadMutexSection.unlock();
					try
					{
						// Dependency tree can be no deeper than 6 (just an artificial way to detect circular
						//  trees, since having an actual resource with that much abstraction is unlikely)
						loadResourceAndDeps(toLoadData.resource, 6);
					}
					catch (FileSystemException &ex)
					{
						//! \todo Call error handler ('m_ErrorHandler' should be in ResourceManager)
						logfile->AddEntry(ex.GetDescription(), LOG_NORMAL);
					}
					toLoadMutexSection.lock();
				}
			}

			// Unload
			{
				CL_MutexSection toUnloadMutexSection(&m_ToUnloadMutex);
				for (ToUnloadList::iterator it = m_ToUnload.begin(), end = m_ToUnload.end(); it != end; ++it)
				{
					unloadResource(*it);
					(*it)->_setQueuedToUnload(false);
				}
				m_ToUnload.clear();
			}
		}

		//asThreadCleanup();
	}

	void ResourceManager::DeliverLoadedResources()
	{
		ResourceList::iterator it = m_ToDeliver.begin(), end = m_ToDeliver.end();
		while (it != end)
		{
			ResourceDataPtr &res = *it;
			if (res->IsLoaded())
			{
				if (res->RequiresGC())
				{
					CL_MutexSection lock(&m_LoaderMutex);
					auto _where = m_ResourceLoaders.find(res->GetType());
					if (_where != m_ResourceLoaders.end())
					{
						ResourceLoader& loader = _where->second;
						loader.gcload(res.get(), m_GC, loader.userData);
					}
				}
				res->SigLoaded(res);
				res->SigLoaded.disconnect_all_slots();

				res->_setQueuedToLoad(false);

				it = m_ToDeliver.erase(it);
				end = m_ToDeliver.end();
			}
			else
				++it;
		}
		//m_LoadedResources.clear();
	}

	void ResourceManager::UnloadUnreferencedResources()
	{
		{
			CL_MutexSection lock(&m_ToUnloadMutex);
			for (UnreferencedResourceSet::iterator it = m_Unreferenced.begin(), end = m_Unreferenced.end(); it != end; ++it)
			{
				m_ToUnload.push_back(*it);
				(*it)->_setQueuedToUnload(true);
			}
		}
		m_Unreferenced.clear();
		m_ToUnloadEvent.set();
	}

	boost::signals2::connection ResourceManager::GetResource(const std::string& type, const std::string& path, const ResourceContainer::LoadedFn &on_load_callback, int priority)
	{
		ResourceDataPtr resource;

		// Get or create the resource in the map
		obtainResource(resource, type, path);

		boost::signals2::connection onLoadConnection;

		if (resource->IsLoaded())
		{
			if (on_load_callback)
				on_load_callback(resource);
		}
		else
		{
			if (on_load_callback)
				onLoadConnection = resource->SigLoaded.connect(on_load_callback);

			if (!resource->IsQueuedToLoad())
			{
				m_ToDeliver.push_back(resource);
				resource->_setQueuedToLoad(true);

				// A non-locking alternative would be to add these ToLoad jobs
				//  to another queue only accessed by this thread, then using an
				//  Update(dt) method (called each engine-step) push those entries
				//  onto the ToLoad queue whenever the worker has nothing to do
				//  (some var set to true.) There will have to be
				//  a limit to how many load jobs can be pushed per interval of
				//  time so that the Update method doesn't get bogged down.
				//   Of course, the lock below and its counterpart in the worker
				//  thread only happen for a moment, so it is most likely not
				//  a big enough deal to look at a non-locking solution.
				//  If I were to hazzard a guess, I'd say locking will actually
				//  yeald better performance.
				m_ToLoadMutex.lock();
				m_ToLoad.push(ResourceToLoadData(priority, resource));
				m_ToLoadMutex.unlock();

				m_ToLoadEvent.set();
			}
		}

		return onLoadConnection;
	}

	void ResourceManager::UnloadResource(const std::string &path, CL_GraphicContext &gc)
	{
		getAndUnloadResource(path);
	}

	void ResourceManager::DeleteAllResources(CL_GraphicContext &gc)
	{
		m_Clearing = true;
		for (ResourceMap::iterator it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			unloadResource(it->second, true);
		}
		// There may still be ResourcePointers holding shared_ptrs to the
		//  ResourceContainers held in m_Resources, but they will simply
		//  be invalidated by the previous step. In any case, the following
		//  step means all ResourceContainers will be deleated ASAP.
		m_Resources.clear();
		m_Unreferenced.clear();
		m_Clearing = false;
	}

	StringVector ResourceManager::ListFiles()
	{
		StringVector list;

		// List all files
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

	std::string ResourceManager::FindFirst(const std::string &expression, bool recursive, bool case_sensitive)
	{
		return FindFirst("", expression, 0, case_sensitive, recursive);
	}

	std::string ResourceManager::FindFirst(const std::string &path, const std::string &expression, int depth, bool recursive, bool case_sensitive)
	{
		if (SetupPhysFS::is_init())
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

		if (SetupPhysFS::is_init())
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

	void ResourceManager::_resourceUnreferenced(ResourceContainer* resource)
	{
		if (!m_Clearing)
		{
			m_Unreferenced.insert(resource);
			resource->NoReferences = ResourceContainer::ReleasedFn();
		}
	}

	////////////
	/// Private:
	void ResourceManager::obtainResource(ResourceDataPtr &resource, const std::string& type, const std::string& path)
	{
		using namespace std::placeholders;

		// Check whether the given resource already exists
		ResourceMap::iterator existingEntry = m_Resources.find(path);
		if (existingEntry != m_Resources.end() && existingEntry->second->GetPath() == path)
		{
			const bool wasUnused = existingEntry->second->Unused();
			resource = existingEntry->second;

			// Remove matching entry from Unreferenced
			if (wasUnused)
			{
				m_Unreferenced.erase(resource.get());
				resource->NoReferences = std::bind(&ResourceManager::_resourceUnreferenced, this, _1);
			}
		}
		else
		{
			resource = ResourceDataPtr(new ResourceContainer(type, path, nullptr));
			resource->NoReferences = std::bind(&ResourceManager::_resourceUnreferenced, this, _1);

			m_Resources[path] = resource;
		}

		if (resource->IsQueuedToUnload())
		{
			// Stop the resource form unloading
			CL_MutexSection lock(&m_ToUnloadMutex);
			for (ResourceList::iterator it = m_ToUnload.begin(), end = m_ToUnload.end(); it != end; ++it)
			{
				if (*it == resource)
				{
					(*it)->_setQueuedToUnload(false);
					m_ToUnload.erase(it);
					break;
				}
			}
		}
	}

	void ResourceManager::loadResource(ResourceDataPtr &resource)
	{
		CL_MutexSection loaderMutexSection(&m_LoaderMutex);
		ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());
		if (_where == m_ResourceLoaders.end())
		{
			FSN_EXCEPT(ExCode::FileType, "Attempted to load unknown resource type '" + resource->GetType() + "'");
		}

		// Initialize a vdir
		CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

		// Run the load function
		ResourceLoader &loader = _where->second;
		loader.load(resource.get(), vdir, loader.userData);
	}

	void ResourceManager::getAndUnloadResource(const std::string &path)
	{
		ResourceMap::iterator _where = m_Resources.find(path);
		if (_where != m_Resources.end())
			unloadResource(_where->second);
	}

	void ResourceManager::unloadResource(ResourceDataPtr &resource, bool quickload)
	{
		CL_MutexSection loaderMutexSection(&m_LoaderMutex);

		ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());
		if (_where != m_ResourceLoaders.end())
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the load function
			ResourceLoader &loader = _where->second;
			loader.unload(resource.get(), vdir, loader.userData);
		}
	}

}
