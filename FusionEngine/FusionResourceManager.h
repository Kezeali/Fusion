/*
  Copyright (c) 2006-2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_ResourceManager
#define Header_FusionEngine_ResourceManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <queue>
#ifdef _WIN32
#include <ClanLib/core.h>
#endif

// Inherited
#include "FusionSingleton.h"

// Fusion
#include "FusionResource.h"
#include "FusionResourceLoader.h"


namespace FusionEngine
{

	/*!
	 * \brief
	 * Loads and stores resources for gameplay.
	 *
	 * \todo Add XMLloader to default loaders
	 *
	 * \sa Resource | ResourcePointer | ResourceLoader
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

		//! Assigns the given ResourceLoader to its relavant resource type
		void AddResourceLoader(const ResourceLoader& resourceLoader);
		//! Creates a new resource loader using the given callbacks
		void AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, void* userData);
		//! Creates a new resource loader using the given callbacks
		void AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, resource_unload qlDataUnloadFn, void* userData);

		//! Starts loading resources in the background
		/*!
		* \param[in] shared_gc
		* The GC to pass to the worker thread. What this should be is platform dependant.
		*/
		void StartLoaderThread(CL_GraphicContext &shared_gc);
		//! Stops loading resources in the background
		void StopLoaderThread();

		//! Loads / unloads resources as they are added to the each queue
		/*!
		* <p>This method is to be run in a thread - call StartLoaderThread rather than calling this method directly.</p>
		* Loads resources listed in the ToLoad list,
		* unloads resources listed in the ToUnload list.
		*/
		void Load(CL_GraphicContext *gc
#ifdef _WIN32
			// Since WGL requires the worker-GC to be created in the worker thread
			//  an event must be passed here so the main thread can wait until the worker
			//  GC has been created.
			, CL_Event worker_gc_created
#endif
			);

		//! Invokes the ResourceLoadedFn for each loaded resource
		void DeliverLoadedResources();

		//! Adds all resources that are no longer used to the ToUnload queue
		void UnloadUnreferencedResources();

		//! Loads / gets a resource
		bsig2::connection GetResource(const std::string& type, const std::string& path, const ResourceContainer::LoadedFn &on_load_callback, int priority = 0);

		//! Unloads the given resource
		/*!
		* \param[in] path
		* Ident. of the resource to unload
		* \param[in] gc
		* GraphicContext for the current thread
		*/
		void UnloadResource(const std::string &path, CL_GraphicContext &gc);

		//! Unloads and deletes all resources
		/*!
		* \param[in] gc
		* GraphicContext for the current thread
		*/
		void DeleteAllResources(CL_GraphicContext &gc);

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
		typedef std::unordered_map<std::string, ResourceDataPtr> ResourceMap;
		//! Maps Resource types to ResourceLoader factory methods
		typedef std::unordered_map<std::string, ResourceLoader> ResourceLoaderMap;

		typedef std::vector<ResourceDataPtr> ResourceList;
		typedef std::set<ResourceContainer*> UnreferencedResourceSet;

		struct ResourceToLoadData
		{
			int priority;
			ResourceDataPtr resource;

			ResourceToLoadData() {}

			ResourceToLoadData(int _priority, const ResourceDataPtr &_resource)
				: priority(_priority),
				resource(_resource)
			{}

			bool operator< (const ResourceToLoadData& rhs) const
			{
				return priority < rhs.priority;
			}   
		};

		typedef std::priority_queue<ResourceToLoadData> ToLoadQueue;
		typedef std::vector<ResourceDataPtr> ToUnloadList;

		CL_Event m_StopEvent; // Set to stop the worker thread
		CL_Event m_ToLoadEvent; // Set when there is more data to load
		CL_Event m_ToUnloadEvent;
		CL_Thread m_Worker;

		CL_GraphicContext m_GC;

		CL_Mutex m_ToLoadMutex;
		ToLoadQueue m_ToLoad;

		CL_Mutex m_ToUnloadMutex;
		ToUnloadList m_ToUnload;

		// Resources
		ResourceMap m_Resources;

		bool m_Clearing;

		UnreferencedResourceSet m_Unreferenced;

		CL_Mutex m_ToDeliverMutex;
		ResourceList m_ToDeliver;

		CL_Mutex m_LoaderMutex;
		// ResourceLoader factory methods
		ResourceLoaderMap m_ResourceLoaders;

		void loadResource(ResourceDataPtr &resource, CL_GraphicContext &gc);

		void getAndUnloadResource(const std::string &path, CL_GraphicContext &gc);
		void unloadResource(ResourceDataPtr &resource, CL_GraphicContext &gc, bool unload_quickload_data = false);
	};

}

#endif
