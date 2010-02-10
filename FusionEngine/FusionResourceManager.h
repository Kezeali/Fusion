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

// Inherited
#include "FusionSingleton.h"

// Fusion
#include "FusionResource.h"
//#include "FusionResourcePointer.h"
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
		//! ResourceLoader pointer
		typedef std::tr1::shared_ptr<ResourceLoader> ResourceLoaderSpt;
		//! Maps ResourceTag keys to Resource ptrs
		typedef std::tr1::unordered_map<ResourceTag, ResourceDataPtr> ResourceMap;
		//! Maps Resource types to ResourceLoader factory methods
		typedef std::tr1::unordered_map<std::string, ResourceLoader> ResourceLoaderMap;

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
		//typedef std::tr1::unordered_set<ResourceDataPtr> ToUnloadList;

	public:
		//! Constructor
		ResourceManager(char *arg0);
		//! Constructor - gets eq. of arg0 from CL_System::get_exe_path()
		ResourceManager();
		//! Destructor
		~ResourceManager();

	public:
		//! Sets the GC
		void SetGraphicContext(const CL_GraphicContext &gc);
		//! Gets the GC
		const CL_GraphicContext &GetGraphicContext() const;

		//! Configures the resource manager
		void Configure();

		//! Starts loading resources in the background
#ifdef _WIN32
		void StartBackgroundLoadThread(CL_GraphicContext &sharedGC, CL_Event &worker_gc_created);
#else
		void StartBackgroundLoadThread(CL_GraphicContext &sharedGC);
#endif
		//! Stops loading resources in the background
		void StopBackgroundLoadThread();

		//! Loads / unloads resources in another thread
		/*!
		 * Loads resources listed in the ToLoad list.<br>
		 * Unloads resources listed in the ToUnload list.
		 */
#ifdef _WIN32
		void BackgroundLoad(CL_GraphicContext *gc, CL_Event worker_gc_created);
#else
		void BackgroundLoad(CL_GraphicContext *gc);
#endif

		//! Checks the filesystem and returns the filenames of all found
		StringVector ListFiles();

		//! Deletes all loaded resources
		void DeleteResources();

		//! Clears the resource manager
		/*!
		 * Clears all resources.
		 */
		void ClearAll();

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

		void AddDefaultLoaders();

		//! Assigns the given ResourceLoader to its relavant resource type
		void AddResourceLoader(const ResourceLoader& resourceLoader);

		//! Assigns the given ResourceLoader to its relavant resource type
		void AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, void* userData);

		//! Assigns the given ResourceLoader to its relavant resource type
		void AddResourceLoader(const std::string& type, resource_load loadFn, resource_unload unloadFn, resource_unload qlDataUnloadFn, void* userData);

		//! Invokes the ResourceLoadedFn for each loaded resource
		void DeliverLoadedResources();

		//! Adds all resources that are no longer used to the ToUnload queue
		void UnloadUnreferencedResources();

		//! Loads / gets a resource
		bsig2::connection GetResource(const std::string& type, const std::string& path, const ResourceContainer::LoadedFn &on_load_callback, int priority = 0);

		//! Internal method (called by resources when they go out of use)
		void resourceUnreferenced(ResourceContainer *resource);

		void UnloadResource(const std::string &path);

	private:
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

	protected:
		void loadResource(ResourceDataPtr &resource, CL_GraphicContext &gc);

		//! \todo Should be called getAndUnloadResource
		void unloadResource(const std::string &path, CL_GraphicContext &gc);
		void unloadResource(ResourceDataPtr &resource, CL_GraphicContext &gc, bool unload_quickload_data = false);

		//void registerXMLType(asIScriptEngine* engine);
		//void registerImageType(asIScriptEngine* engine);
		//void registerSoundType(asIScriptEngine* engine);

		/*!
		 * \brief
		 * Gets a point form the 'x' and 'y' attributes of an element.
		 *
		 * \returns
		 * The CL_Point built from the x and y attributes.
		 * If the given element has no x/y attribute, the point (0, 0) is returned.
		 */
		CL_Point getPoint(const CL_DomElement *element);

		/*!
		 * \brief
		 * Checks the given list for the specified file.
		 *
		 * Used for checking if a resource specified in a definition actually exists in the
		 * package!
		 *
		 * \param[in] filename The name of the file to be checked
		 * \param[in] filelist The list to search for the given filename
		 */
		bool checkInList(const std::string &filename, StringVector filelist);
	};

}

#endif
