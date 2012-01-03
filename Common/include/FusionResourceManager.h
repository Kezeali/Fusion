/*
*  Copyright (c) 2006-2011 Fusion Project Team
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

#ifndef H_FusionEngine_ResourceManager
#define H_FusionEngine_ResourceManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// TODO: why is this required for WIN32?
#ifdef _WIN32
#include <ClanLib/core.h>
#endif
#include <queue>
#include <unordered_map>

// Inherited
#include "FusionSingleton.h"

// Fusion
#include "FusionResource.h"
#include "FusionResourceLoader.h"

#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Loads and stores resources.
	 *
	 * \sa ResourceContainer | ResourcePointer | ResourceLoader
	 */
	class ResourceManager : public Singleton<ResourceManager>
	{
	public:
		//! Constructor
		/*!
		* \param[in] gc
		* GraphicContext of the current thread
		*/
		ResourceManager(const CL_GraphicContext &gc);
		//! Destructor
		~ResourceManager();

		const CL_GraphicContext& GetGC() const;

		//! Assigns the given ResourceLoader to its relavant resource type
		void AddResourceLoader(const ResourceLoader& resourceLoader);
		//! Creates a new resource loader using the given callbacks
		void AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, void* userData);

		//! Starts loading resources in the background
		void StartLoaderThread();
		//! Stops loading resources in the background
		void StopLoaderThread();
		//! Stops loading resources in the background
		void StopLoaderThreadWhenDone();

		//! Loads / unloads resources as they are added to the each queue
		/*!
		* <p>This method is to be run in a thread - call StartLoaderThread rather than calling this method directly.</p>
		* Loads resources listed in the ToLoad list,
		* unloads resources listed in the ToUnload list.
		*/
		void Load();

		//! Invokes the ResourceLoadedFn (callback) for each loaded resource
		void DeliverLoadedResources();

		//! Adds all resources that are no longer used (refcount is zero) to the ToUnload queue
		void UnloadUnreferencedResources();

		//! Loads / gets a resource (asynchronous)
		boost::signals2::connection GetResource(const std::string& type, const std::string& path, const ResourceContainer::LoadedFn &on_load_callback, int priority = 0);

		//! Unloads the given resource
		/*!
		* \param[in] path
		* Ident. of the resource to unload
		* \param[in] gc
		* GraphicContext for the render thread
		*/
		void UnloadResource(const std::string &path, CL_GraphicContext &gc);

		//! Unloads and deletes all resources
		/*!
		* \param[in] gc
		* GraphicContext for the render thread
		*/
		void DeleteAllResources(CL_GraphicContext &gc);

		//! Make sure resources with references are loaded, and resources without aren't
		/*
		* \param allow_queued
		* If this is set to true, resources will be considered valid if they are at least in
		* the correct queue (to be loaded / unloaded.) This should usually be set to true if
		* the load thread is running.
		*/
		bool VerifyResources(const bool allow_queued) const;
		//! Returns the path of each currently loaded resource
		std::vector<std::string> ListLoadedResources() const;

		//! Checks the filesystem and returns the filenames of all found
		StringVector ListFiles();

		//! Returns the first matching string
		std::string FindFirst(const std::string &expression, bool recursive = false, bool case_sensitive = true);
		std::string FindFirst(const std::string &path, const std::string &expression, int depth, bool recursive = true, bool case_sensitive = true);

		//! Lists filenames matching the given expression
		StringVector Find(const std::string &expression, bool recursive = false, bool case_sensitive = true);

		//! Lists filenames matching the given expression, below the given path
		/*!
		 * If depth is set >1 and recursive is false, this method will go 'depth' folders deep in the
		 * filesystem below the Search Path. If recursive is true, it will ignore 'depth' and go as 
		 * deep as possible.
		 */
		StringVector Find(const std::string &path, const std::string &expression, int depth, bool recursive = true, bool case_sensitive = true);

		//! Internal method (called by resources when they go out of use)
		void _resourceUnreferenced(ResourceContainer *resource);

	private:
		//! ResourceLoader pointer
		typedef std::shared_ptr<ResourceLoader> ResourceLoaderSpt;
		//! Maps ResourceTag keys to Resource ptrs
		typedef tbb::concurrent_unordered_map<std::string, ResourceDataPtr> ResourceMap;
		//! Maps Resource types to ResourceLoader factory methods
		typedef std::unordered_map<std::string, ResourceLoader> ResourceLoaderMap;

		typedef tbb::concurrent_queue<ResourceDataPtr> ResourceQueue;
		typedef tbb::concurrent_unordered_set<ResourceContainer*> UnreferencedResourceSet;

		struct ResourceToLoadData
		{
			int priority;
			ResourceDataPtr resource;

			ResourceToLoadData() {}

			ResourceToLoadData(int _priority, const ResourceDataPtr& _resource)
				: priority(_priority),
				resource(_resource)
			{}

			ResourceToLoadData(const ResourceToLoadData& other)
				: priority(other.priority),
				resource(other.resource)
			{}

			ResourceToLoadData(ResourceToLoadData&& other)
				: priority(other.priority),
				resource(std::move(other.resource))
			{}

			bool operator< (const ResourceToLoadData& rhs) const
			{
				return priority < rhs.priority;
			}   
		};

		typedef tbb::concurrent_priority_queue<ResourceToLoadData> ToLoadQueue;
		typedef tbb::concurrent_queue<ResourceContainer*> ToUnloadQueue;

		CL_Event m_StopEvent; // Set to stop the worker thread
		CL_Event m_ToLoadEvent; // Set when there is more data to load
		CL_Event m_ToUnloadEvent;
		CL_Thread m_Thread;
		bool m_Running;
		bool m_Clearing;
		bool m_FinishLoadingBeforeStopping;

		CL_GraphicContext m_GC;

		//CL_Mutex m_ToLoadMutex;
		ToLoadQueue m_ToLoad;

		//CL_Mutex m_ToUnloadMutex;
		ToUnloadQueue m_ToUnload;
		ToUnloadQueue m_ToUnloadUsingGC;

		// Resources
		ResourceMap m_Resources;

		CL_Mutex m_UnreferencedMutex;
		
		UnreferencedResourceSet m_Unreferenced;

		//CL_Mutex m_ToDeliverMutex;
		ResourceQueue m_ToDeliver;

		//CL_Mutex m_LoaderMutex;
		// ResourceLoader factory methods
		ResourceLoaderMap m_ResourceLoaders;

		//! Gets / makes a ResourceContainer in the resource map (m_Resources), without loading it
		void obtainResource(ResourceDataPtr& out, const std::string& type, const std::string& path);
		// TODO: use this, since intrusive_ptr supports move assignment (rvalue)
		//ResourceDataPtr obtainResource(const std::string& type, const std::string& path);

		void loadResource(const ResourceDataPtr &resource);

		void loadResourceAndDeps(const ResourceDataPtr& resource, unsigned int depth_limit);

		void getAndUnloadResource(const std::string &path);
		void unloadResource(const ResourceDataPtr& resource);
	};

}

#endif
