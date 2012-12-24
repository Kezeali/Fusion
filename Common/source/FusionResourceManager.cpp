/*
*  Copyright (c) 2006-2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionResourceManager.h"

#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/regexp.h>

#include "FusionConsole.h"
#include "FusionExceptionFactory.h"
#include "FusionLogger.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionProfiling.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionScriptManager.h"

namespace FusionEngine
{

	ResourceManager::ResourceManager(const CL_GraphicContext &gc)
		: m_StopEvent(false),
		m_ToLoadEvent(false),
		m_ToUnloadEvent(true),
		m_CheckForChangesEvent(false),
		m_Running(false),
		m_Clearing(false),
		m_FinishLoadingBeforeStopping(false),
		m_ForceCheckForChanges(false),
		m_GC(gc),
		m_HotReloadingAllowed(false)
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
		if (!m_Running)
		{
			// Loaders with prereqs must also have a validatePrereqReload function
			//  and conversely validatePrereqReload is pointless if you don't have
			//  any prereqs
			if((loader.listPrereq == nullptr) != (loader.validatePrereqReload == nullptr))
			{
				if (loader.listPrereq)
					FSN_EXCEPT(InvalidArgumentException, "Resource Loader " + loader.type + " has a listPrereq function, but no validatePrereqReload function. Use both or neither");
				else
					FSN_EXCEPT(InvalidArgumentException, "Resource Loader " + loader.type + " has a validatePrereqReload function, but no listPrereq function. Use both or neither");
			}

			m_ResourceLoaders[loader.type] = loader;
		}
		else
			FSN_EXCEPT(InvalidArgumentException, "Can't add resource loaders after the loader thread has started");
	}

	std::vector<std::string> ResourceManager::GetResourceLoaderTypes() const
	{
		std::vector<std::string> types;
		for (auto it = m_ResourceLoaders.begin(); it != m_ResourceLoaders.end(); ++it)
			types.push_back(it->first);
		return types;
	}

	void ResourceManager::SetHotReloadingAllowed(bool allowed)
	{
		m_HotReloadingAllowed = allowed;
	}

	void ResourceManager::CheckForChanges()
	{
		if (m_HotReloadingAllowed)
		{
			m_ForceCheckForChanges = false;
			m_CheckForChangesEvent.set();
		}
	}

	void ResourceManager::CheckForChangesForced()
	{
		if (m_HotReloadingAllowed)
		{
			m_ForceCheckForChanges = true;
			m_CheckForChangesEvent.set();
		}
	}

	void ResourceManager::StartLoaderThread()
	{
		m_Running = true;
		m_ToUnloadEvent.reset();
		m_Thread.start(this, &ResourceManager::Load);
	}

	void ResourceManager::StopLoaderThread()
	{
		m_FinishLoadingBeforeStopping = false;
		m_StopEvent.set();
		m_Thread.join();
		m_Running = false;
		m_ToUnloadEvent.reset();
	}

	void ResourceManager::StopLoaderThreadWhenDone()
	{
		m_FinishLoadingBeforeStopping = true;
		m_StopEvent.set();
		m_Thread.join();
		m_Running = false;
	}

	void ResourceManager::loadResourceAndDeps(const ResourceDataPtr& resource, unsigned int depth_limit)
	{
		if (!resource->IsLoaded() && !resource->RequiresGC())
		{
			if (depth_limit == 0)
			{
				FSN_EXCEPT(FileSystemException, "Dependency tree for '" + resource->GetPath() + "' is too deep: it's probably circular");
			}

			auto _where = m_ResourceLoaders.find(resource->GetType());
			if (_where == m_ResourceLoaders.end())
			{
				FSN_EXCEPT(FileTypeException, "Attempted to load unknown resource type '" + resource->GetType() + "'");
			}
			ActiveResourceLoader& loader = _where->second;

			if (loader.listPrereq != nullptr)
			{
				DepsList prereqs;
				loader.listPrereq(resource.get(), prereqs, loader.userData);

				for (auto it = prereqs.begin(), end = prereqs.end(); it != end; ++it)
				{
					ResourceDataPtr dep;
					obtainResource(dep, it->first, it->second);
					loadResourceAndDeps(dep, depth_limit - 1);

					resource->AttachDependency(dep);
					// Configure a proxy for the hot-reload signal
					using namespace std::placeholders;
					dep->SigHotReloadEvents.connect(std::bind(&ResourceManager::validatePrereqReload, this, resource, _1, _2));
				}
			}

			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the load function
			loader.load(resource.get(), vdir, loader.userData);
		}
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

		std::string lastResourceChecked; // for hot-reload
		bool checkingResourcesForReload = false;

		while (true)
		{
			// Wait until there is more to load, or a stop event is received
			int receivedEvent = CL_Event::wait(m_StopEvent, m_ToUnloadEvent, m_ToLoadEvent, m_CheckForChangesEvent);
			// 0 = stop event, -1 = error; otherwise, it is
			//  a ToLoad / ToUnload Event, meaning the load loop below should resume
			if (receivedEvent <= 0 && !m_FinishLoadingBeforeStopping)
				break;

			bool unloadTriggered = false;
			if (receivedEvent == 1)
			{
				unloadTriggered = true;
				m_ToUnloadEvent.reset();
			}

			// Load
			{
				ResourceToLoadData toLoadData;
				while (m_ToLoad.try_pop(toLoadData))
				{
					// In GetResource, if IsQueuedToUnload is true, IsQueuedToLoad is checked and
					//  and if it isn't set this resource is re-enqueued to load
					//  So QueuedToUnload is set to false here to avoid this unnecessary
					//  re-enqueueation (which turns out to happen quite often without this
					//  workaround)
					toLoadData.resource->setQueuedToUnload(false);

					try
					{
						logfile->AddEntry("Loading " + toLoadData.resource->GetPath(), LOG_INFO);
						// Dependency tree can be no deeper than 6 (just an artificial way to detect circular
						//  trees, since having an actual resource with that much abstraction is unlikely)
						loadResourceAndDeps(toLoadData.resource, 6);
					}
					catch (FileSystemException &ex)
					{
						//! \todo Call error handler ('m_ErrorHandler' should be in ResourceManager)
						logfile->AddEntry(ex.GetDescription(), LOG_NORMAL);
					}

					toLoadData.resource->setQueuedToLoad(false);

					// Push this on to the outgoing queue (even if it failed to load)
					m_ToDeliver.push(toLoadData.resource);
				}
			}

			// Unload
			if (unloadTriggered || receivedEvent == 0)
			{
				//CL_MutexSection toUnloadMutexSection(&m_ToUnloadMutex);
				//for (auto it = m_ToUnload.begin(), end = m_ToUnload.end(); it != end; ++it)
				ResourceContainer* res;
				while (m_ToUnload.try_pop(res))
				{
					if (!res->IsQueuedToLoad())
					{
						if (res->Unused())
						{
							try
							{
								logfile->AddEntry("Unloading " + res->GetPath(), LOG_INFO);
								if (!unloadResource(res))
								{
									logfile->AddEntry("Unload paused on resource type " + res->GetType() + ". Will unload later.", LOG_TRIVIAL);
									m_ToUnload.push(res);
									m_ToUnloadEvent.set();
									continue;
								}
							}
							catch (FileSystemException& ex)
							{
								logfile->AddEntry(ex.GetDescription(), LOG_NORMAL);
							}
						}
					}
					else
					{
						logfile->AddEntry("Accidentally unloaded " + res->GetPath() + ", reloading", LOG_NORMAL);
						m_ToLoad.push(ResourceToLoadData(0, res));
						m_ToLoadEvent.set();
					}
					res->setQueuedToUnload(false);
				}
				//m_ToUnload.clear();
			}

			// Perform validated reloads
			if (m_HotReloadingAllowed)
			{
				ResourceDataPtr resource;
				while (m_ToReload.try_pop(resource))
				{
					// Resource map entry + ToReload entry = 2 refs
					if (resource->ReferenceCount() == 2 && unloadResource(resource))
					{
						loadResource(resource);

						//m_ToDeliver.push(resource);

						ReloadEventQueueEntry entry;
						entry.resource = resource;
						entry.eventToFire = ResourceContainer::HotReloadEvent::PostReload;
						m_HotReloadEventsQueue.push(entry);
					}
					else
					{
						// Resource has been retrieved by a new user. This isn't a massive problem: it will
						//  be reloaded next time a changes check is run (assuming /another/ new user doesn't
						//  pop up)
						logfile->AddEntry("Hot-reload of " + resource->GetPath() + " aborted: resource has become used again.", LOG_NORMAL);
					}

					resource->SetMarkedToReload(false);
				}
			}

			if ((receivedEvent == 3 || m_ForceCheckForChanges) && m_HotReloadingAllowed)
				checkingResourcesForReload = true;

			// Check for changes, push changed resources into the hot-reload pipeline
			if (checkingResourcesForReload && m_HotReloadingAllowed)
			{
				const size_t maxChecked = 100;
				size_t resourcesChecked = 0;

				// Attempt to continue iteration from the last resource checked
				ResourceMap::const_iterator it = m_Resources.find(lastResourceChecked);
				if (it != m_Resources.end())
					++it;
				else
					it = m_Resources.cbegin();

				for (; it != m_Resources.cend(); ++it)
				{
					const auto& resource = it->second;
					// Check for changes
					if (!resource->IsMarkedToReload() && shouldReload(resource))
					{
						resource->SetMarkedToReload(true);

						ReloadEventQueueEntry entry;
						entry.resource = resource;
						entry.eventToFire = ResourceContainer::HotReloadEvent::Validate;
						m_HotReloadEventsQueue.push(entry);
					}

					if (!m_ForceCheckForChanges && resourcesChecked++ >= maxChecked)
					{
						lastResourceChecked = it->first;
						break;
					}
				}

				if (it == m_Resources.cend())
				{
					checkingResourcesForReload = false; // done
					lastResourceChecked.clear();
				}
			}

			// Stop when stop even is set
			if (receivedEvent == 0)
				break;
		}

		//asThreadCleanup();
	}

	void ResourceManager::DeliverLoadedResources(float time_limit)
	{
		FSN_PROFILE("DeliverLoadedResources");
		auto startTime = tbb::tick_count::now();
		ResourceDataPtr res;
		while ((tbb::tick_count::now() - startTime).seconds() < time_limit && m_ToDeliver.try_pop(res))
		{
			if (!res->IsLoaded() && res->RequiresGC())
			{
				//CL_MutexSection lock(&m_LoaderMutex);
				auto _where = m_ResourceLoaders.find(res->GetType());
				if (_where != m_ResourceLoaders.end())
				{
					FSN_PROFILE("GCLoad " + res->GetType());
					ActiveResourceLoader& loader = _where->second;
					loader.gcload(res.get(), m_GC, loader.userData);
				}
			}

			if (res->IsLoaded())
			{
				FSN_PROFILE("SigLoaded " + res->GetType());
#ifdef FSN_PROFILING_ENABLED
				Profiling::getSingleton().AddTime("~SigLoaded " + res->GetType(), double(res->SigLoaded->num_slots()));
#endif
				(*res->SigLoaded)(res);
				res->SigLoaded->disconnect_all_slots();
				std::shared_ptr<ResourceContainer::LoadedSignal> sigLoaded;
				while ((tbb::tick_count::now() - startTime).seconds() < time_limit && res->SigLoadedExt.try_pop(sigLoaded))
				{
#ifdef FSN_PROFILING_ENABLED
					Profiling::getSingleton().AddTime("~SigLoaded " + res->GetType(), double(sigLoaded->num_slots()));
#endif
					(*sigLoaded)(res);
				}
				
				if (res->SigLoadedExt.empty())
					SignalResourceLoaded(res);
				else
					m_ToDeliver.push(res); // If there are more listeners to signal, do so later
			}
			else
			{
				// TODO: notify listeners of failure (perhaps like this)
				//res->SigLoaded(res);
				res->SigLoaded->disconnect_all_slots();
				res->SigLoadedExt.clear();
			}
		}

		ReloadEventQueueEntry entry;
		while ((tbb::tick_count::now() - startTime).seconds() < time_limit && m_HotReloadEventsQueue.try_pop(entry))
		{
			const auto& resource = entry.resource;

			switch (entry.eventToFire)
			{
			case ResourceContainer::HotReloadEvent::Validate:
				{
					const auto validated = resource->SigHotReloadEvents(resource.get(), ResourceContainer::HotReloadEvent::Validate);

					if (validated && resource->ReferenceCount() == 2)
					{
						resource->SigHotReloadEvents(resource.get(), ResourceContainer::HotReloadEvent::PreReload);
						m_ToReload.push(resource);
					}
					else
					{
						resource->SetMarkedToReload(false);
						AddLogEntry("ResourceManager", "Hot-reload of " + resource->GetPath() + " aborted: one more more users didn't allow the reload", LOG_NORMAL);
					}
				}
				break;
			case ResourceContainer::HotReloadEvent::PostReload:
				resource->SigHotReloadEvents(resource.get(), ResourceContainer::HotReloadEvent::PostReload);
				break;
			}
		}
	}

	void ResourceManager::UnloadUnreferencedResources()
	{
		FSN_PROFILE("UnloadUnreferenceResources");
		{
			ResourceContainer* res;
			while (m_ToUnloadUsingGC.try_pop(res))
			{
				FSN_ASSERT(res->RequiresGC());
				if (res->Unused())
					unloadResource(res);
			}
		}
		m_ToUnloadEvent.set();
	}

	void ResourceManager::CancelAllDeliveries()
	{
		ResourceDataPtr res;
		while (m_ToDeliver.try_pop(res))
		{
			res->SigLoaded->disconnect_all_slots();
			res->SigLoadedExt.clear();
		}
	}

	boost::signals2::connection ResourceManager::GetResource(const std::string& type, const std::string& path, const ResourceContainer::LoadedFn &on_load_callback, int priority)
	{
		ResourceDataPtr resource;

		// Get or create the resource in the map
		obtainResource(resource, type, path);

		boost::signals2::connection onLoadConnection;

		if (!resource->IsQueuedToUnload() && resource->IsLoaded()) 
		{
			if (on_load_callback)
			{
				on_load_callback(resource);
			}
		}
		else // is QueuedToUnload || !Loaded
		{
			if (on_load_callback)
			{
				if (resource->SigLoaded->num_slots() > 32)
				{
					// Create a new signal so that notifications can be delayed
					auto newSignal = std::make_shared<ResourceContainer::LoadedSignal>();
					std::swap(resource->SigLoaded, newSignal);
					resource->SigLoadedExt.push(newSignal);
				}
				onLoadConnection = resource->SigLoaded->connect(on_load_callback);
			}

			if (!resource->setQueuedToLoad(true))
			{
				// IsLoaded is checked again in case the resource was loaded between here and the previous check
				if (!resource->IsLoaded() || resource->IsQueuedToUnload())
				{
					AddLogEntry("ResourceRequests", "Requested " + resource->GetPath(), LOG_INFO);
					m_ToLoad.push(ResourceToLoadData(priority, resource));
					m_ToLoadEvent.set();
				}
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
		m_ToLoad.clear();
		m_ToUnload.clear();
		for (ResourceMap::iterator it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			unloadResource(it->second);
			it->second->NoReferences = ResourceContainer::ReleasedFn();
		}
		// There may still be ResourcePointers holding shared_ptrs to the
		//  ResourceContainers held in m_Resources, but they will simply
		//  be invalidated by the previous step. In any case, the following
		//  step means all ResourceContainers will be deleted ASAP.
		m_Resources.clear();
		//CL_MutexSection lock(&m_UnreferencedMutex);
		//m_Unreferenced.clear();
		m_Clearing = false;
	}

	void ResourceManager::PauseUnload(const std::string& resource_type)
	{
		auto& resourceLoader = m_ResourceLoaders[resource_type];
		resourceLoader.activeOperations = resourceLoader.activeOperations ^ ActiveResourceLoader::ActiveOperation::Unload;
	}

	void ResourceManager::ResumeUnload(const std::string& resource_type)
	{
		auto& resourceLoader = m_ResourceLoaders[resource_type];
		resourceLoader.activeOperations = resourceLoader.activeOperations | ActiveResourceLoader::ActiveOperation::Unload;
	}

	bool ResourceManager::VerifyResources(const bool allow_queued) const
	{
		bool success = true;
		for (auto it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			if (!it->second)
			{
				success = false;
				SendToConsole(it->first + " is uninitialised (resource entry is null.)");
			}
			if (it->second->Unused() && it->second->IsLoaded() && !(allow_queued && it->second->IsQueuedToUnload()))
			{
				success = false;
				SendToConsole(it->second->GetPath() + " is loaded but unused.");
			}
			else if (!it->second->Unused() && !it->second->IsLoaded() && !(allow_queued && it->second->IsQueuedToLoad()))
			{
				success = false;
				SendToConsole(it->second->GetPath() + " is not loaded even though it is used.");
			}
		}
		return success;
	}

	std::vector<std::string> ResourceManager::ListLoadedResources() const
	{
		std::vector<std::string> list;
		for (auto it = m_Resources.begin(), end = m_Resources.end(); it != end; ++it)
		{
			if (it->second && it->second->IsLoaded())
				list.push_back(it->second->GetPath());
		}
		return list;
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
			AddLogEntry("ResourceRequests", "---Unused " + resource->GetPath(), LOG_INFO);
			if (!resource->setQueuedToUnload(true))
			{
				if (!resource->RequiresGC())
					m_ToUnload.push(resource);
				else
					m_ToUnloadUsingGC.push(resource);
				resource->NoReferences = ResourceContainer::ReleasedFn();
			}
		}
	}

	////////////
	/// Private:
	void ResourceManager::obtainResource(ResourceDataPtr &resource, const std::string& type, const std::string& path)
	{
		using namespace std::placeholders;

		// Check whether the given resource already exists
		auto insertionResult = m_Resources.insert(std::make_pair(path, ResourceDataPtr(new ResourceContainer(type, path, nullptr))));
		auto existingEntry = insertionResult.first;
		if (!insertionResult.second && existingEntry->second && existingEntry->second->GetPath() == path)
		{
			const bool wasUnused = existingEntry->second->Unused();
			resource = existingEntry->second;

			// Remove matching entry from Unreferenced
			if (wasUnused)
			{
				resource->NoReferences = std::bind(&ResourceManager::_resourceUnreferenced, this, _1);
			}
		}
		else // No existing entry or invalid existing entry
		{
			resource = existingEntry->second;
			resource->NoReferences = std::bind(&ResourceManager::_resourceUnreferenced, this, _1);
		}
	}

	void ResourceManager::loadResource(const ResourceDataPtr &resource)
	{
		if (!resource->IsLoaded())
		{
			//CL_MutexSection loaderMutexSection(&m_LoaderMutex);
			ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());
			if (_where == m_ResourceLoaders.end())
			{
				FSN_EXCEPT(ExCode::FileType, "Attempted to load unknown resource type '" + resource->GetType() + "'");
			}

			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the load function
			ActiveResourceLoader& loader = _where->second;
			loader.load(resource.get(), vdir, loader.userData);
		}
	}

	void ResourceManager::getAndUnloadResource(const std::string &path)
	{
		ResourceMap::iterator _where = m_Resources.find(path);
		if (_where != m_Resources.end())
			unloadResource(_where->second);
	}

	bool ResourceManager::unloadResource(const ResourceDataPtr& resource)
	{
		//CL_MutexSection loaderMutexSection(&m_LoaderMutex);

		ResourceLoaderMap::iterator _where = m_ResourceLoaders.find(resource->GetType());
		if (_where != m_ResourceLoaders.end())
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the load function
			ActiveResourceLoader& loader = _where->second;
			if (loader.activeOperations & ActiveResourceLoader::ActiveOperation::Unload)
				loader.unload(resource.get(), vdir, loader.userData);
			else
				return false;
		}
		return true;
	}

	bool ResourceManager::hasChanged(const ResourceDataPtr& resource)
	{
		ResourceLoaderMap::iterator entry = m_ResourceLoaders.find(resource->GetType());
		if (entry != m_ResourceLoaders.end())
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			// Run the function
			ActiveResourceLoader& loader = entry->second;
			if (loader.hasChanged)
				return loader.hasChanged(resource.get(), vdir, loader.userData);
			else
				return false;
		}
		else
			return false;
	}

	bool ResourceManager::shouldReload(const ResourceDataPtr& resource)
	{
		ResourceLoaderMap::iterator entry = m_ResourceLoaders.find(resource->GetType());
		if (entry != m_ResourceLoaders.end())
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");

			ActiveResourceLoader& loader = entry->second;
			// Only run if the loader has a hasChanged function, and Reload is allowed for this type at the moment
			if (loader.hasChanged && loader.activeOperations & ActiveResourceLoader::ActiveOperation::Reload && loader.activeOperations & ActiveResourceLoader::ActiveOperation::Load)
			{
				return loader.hasChanged(resource.get(), vdir, loader.userData);
			}
			else
				return false;
		}
		else
			return false;
	}
	
	bool ResourceManager::validatePrereqReload(const ResourceDataPtr& resource, const ResourceDataPtr& prereq_that_wants_to_reload, ResourceContainer::HotReloadEvent ev)
	{
		ResourceLoaderMap::iterator entry = m_ResourceLoaders.find(resource->GetType());
		if (entry != m_ResourceLoaders.end())
		{
			// Run the function
			ActiveResourceLoader& loader = entry->second;
			return loader.validatePrereqReload(resource.get(), prereq_that_wants_to_reload.get(), ev, loader.userData);
		}
		else
		{
			FSN_EXCEPT(InvalidArgumentException, "Can't validate prerequisite reload: missing resource loader for type " + resource->GetType());
		}
	}

}
