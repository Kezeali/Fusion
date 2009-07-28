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

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionResource.h"
#include "FusionResourcePointer.h"
#include "FusionResourceLoader.h"

/// RakNet
#include <RakNet/Bitstream.h>

namespace FusionEngine
{

	//class ResourceManagerException : public Exception
	//{
	//public:
	//	ResourceManagerException(const std::string& message)
	//		: Exception(Exception::INTERNAL_ERROR, message)
	//	{}
	//};

	/*!
	 * \brief
	 * Returned by GetPackageType().
	 */
	//class PackageType
	//{
	//public:
	//	enum Type { Ship, Level, Weapon, FileNotFound, UnknownType };
	//};

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
		// this is now defined in ResourcePointer:
		//typedef boost::shared_ptr<Resource> ResourceSpt;

		//! ResourceLoader pointer
		typedef std::tr1::shared_ptr<ResourceLoader> ResourceLoaderSpt;
		//! Maps ResourceTag keys to Resource ptrs
		typedef std::tr1::unordered_map<ResourceTag, ResourceSpt> ResourceMap;
		//! Maps Resource types to ResourceLoader factory methods
		typedef std::tr1::unordered_map<std::string, ResourceLoader> ResourceLoaderMap;

		struct ResourceToLoadData
		{
			int priority;
			std::string type;
			std::wstring tag;
			std::wstring path;

			ResourceToLoadData() {}

			ResourceToLoadData(const std::string& _type, const std::wstring& _tag, const std::wstring& _path, int _priority)
				: priority(_priority),
				type(_type),
				tag(_tag),
				path(_path)
			{
			}

			bool operator< (const ResourceToLoadData& rhs) const
			{
				return priority < rhs.priority;
			}   
		};

		typedef std::priority_queue<ResourceToLoadData> ToLoadQueue;
		typedef std::tr1::unordered_set<std::wstring> ToUnloadList;

	public:
		//! Constructor
		ResourceManager(const CL_GraphicContext &gc, char *arg0);
		//! Constructor - gets equivilant of arg0 from CL_System::get_exe_path()
		ResourceManager(const CL_GraphicContext &gc);
		//! Destructor
		~ResourceManager();

	public:
		//! Configures the resource manager
		void Configure();

		//! Starts loading resources in the background
#ifdef _WIN32
		void StartBackgroundPreloadThread(CL_GraphicContext &sharedGC, CL_Event &worker_gc_created);
#else
		void StartBackgroundPreloadThread(CL_GraphicContext &sharedGC);
#endif
		//! Stops loading resources in the background
		void StopBackgroundPreloadThread();

		//! Loads / unloads resources in another thread
		/*!
		 * Loads resources listed in the ToLoad list.<br>
		 * Unloads resources listed in the ToUnload list.
		 */
#ifdef _WIN32
		void BackgroundPreload(CL_GraphicContext *gc, CL_Event worker_gc_created);
#else
		void BackgroundPreload(CL_GraphicContext *gc);
#endif

		////! Checks the filesystem for packages and returns the names of all found
		//StringVector ListAvailablePackages();
		//! Checks the filesystem and returns the filenames of all found
		StringVector ListFiles();

		//! Unloads resource which aren't currently held by any ResourcePointer objects
		void DisposeUnusedResources();

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

		//! Loads a resource, gives it a tag
		ResourceSpt TagResource(const std::string& type, const std::wstring& path, const ResourceTag& tag, CL_GraphicContext *gc = NULL);

		//! Loads a resource
		/*!
		 * \remarks
		 * Though this does the same as TagResource, it must be named differently so that it can be
		 * used directly (through a THISCALL) by AScript, which can't deal with overloaded member fn.s
		 */
		ResourceSpt PreloadResource(const std::string& type, const std::wstring& path);

		//! Loads a resource
		/*!
		 * Loads the resource in another thread
		 */
		ResourceSpt PreloadResource_Background(const std::string& type, const std::wstring& path, int priority = 0);

		void UnloadResource(const std::wstring &path);

		void UnloadResource_Background(const std::wstring &path);

		//! Returns a ResourcePointer to the given Resource (of type T)
		//template<typename T>
		//ResourcePointer<T> GetResource(const ResourceTag& tag);

		//! Gets or creates the given resource
		ResourceSpt &GetResourceDefault(const ResourceTag &tag, const std::string& type);

		template<typename T>
		ResourcePointer<T> GetResource(const ResourceTag &tag)
		{
			ResourceSpt sptRes = m_Resources[tag];
			return ResourcePointer<T>(sptRes);
		}

		template<typename T>
		ResourcePointer<T> GetResource(const std::string &tag, const std::string& type)
		{
			ResourceSpt &resource = GetResourceDefault(fe_widen(tag), type);
			return ResourcePointer<T>(resource);
		}

		template<typename T>
		ResourcePointer<T> GetResource(const ResourceTag &tag, const std::string& type)
		{
			ResourceSpt &resource = GetResourceDefault(tag, type);
			return ResourcePointer<T>(resource);
		}

		//! Optimised version of GetResource
		template<typename T>
		void GetResource(ResourcePointer<T>& out, const ResourceTag &tag)
		{
			ResourceSpt &resource = GetResourceDefault(tag, type);
			out.SetTarget(resource);
			//out = ResourcePointer<T>(resource);
		}

		// How many characters from the beginning before a and b diverge
		std::string::size_type quickCompare(const std::string &a, const std::string &b)
		{
			const char* a_cstr = a.c_str();
			const char* b_cstr = b.c_str();
			std::string::size_type length = fe_min(a.length(), b.length());

			for (std::string::size_type i = 0; i < length; i++)
			{
				if (a_cstr[i] != b_cstr[i]) return i;
			}
		}

		template<typename T>
		ResourcePointer<T> OpenOrCreateResource(const ResourceTag &path)
		{
			std::string match = FindFirst(path);
			if (match.empty()) // No match
			{
				std::string writePath(PHYSFS_getWriteDir());
				// Whether the given 'path' is an existing, absolute path
				if (PHYSFS_isDirectory(CL_String::get_path(path).c_str()))
				{
					// If the given path is existing and absolute, make sure it is within the PhysFS write folder
					if (quickCompare(path, writePath) >= writePath.length())
					{
						FSN_EXCEPT(ExCode::IO, "ResourceManager:OpenOrCreateResource",
							"Can't create '" + path + "' as it is not within the write path (" + writePath + ")");
					}
				}
				else
				{
					// Convert the relative path to an absolute path within the write folder
					writePath = writePath + path;

					std::string folder(CL_String::get_path(writePath));
					if (!PHYSFS_isDirectory(folder.c_str()))
						FSN_EXCEPT(ExCode::IO, "ResourceManager:OpenOrCreateResource",
						"Can't create '" + writePath + "' as '" + folder + "' does not exist");
				}
				// Create the file
				std::ofstream resourceFile;
				resourceFile.open(writePath.c_str(), std::ios::out|std::ios::binary);
				resourceFile.close();

				match = writePath;
			}
			
			return GetResource<T>(match);
		}

		//void RegisterScriptElements(ScriptingEngine* manager);

	private:
		bool m_PhysFSConfigured;

		//! A list of packages which passed verification
		//StringVector m_VerifiedPackages;

		CL_Event m_StopEvent; // Set to stop the worker thread
		CL_Event m_ToLoadEvent; // Set when there is more data to load
		CL_Event m_ToUnloadEvent;
		CL_Thread m_Worker;

		CL_GraphicContext m_GC;

		CL_Mutex m_ToLoadMutex;
		ToLoadQueue m_ToLoad;

		CL_Mutex m_ToUnloadMutex;
		ToUnloadList m_ToUnload;

		CL_Mutex m_ResourcesMutex;
		// Resources
		ResourceMap m_Resources;

		// Garbage
		ResourceMap m_Garbage;

		CL_Mutex m_LoaderMutex;
		// ResourceLoader factory methods
		ResourceLoaderMap m_ResourceLoaders;

	protected:
		ResourceSpt loadResource(const std::wstring &path, CL_GraphicContext &gc);
		void loadResource(ResourceSpt &resource, CL_GraphicContext &gc);

		void unloadResource(const std::wstring &path, CL_GraphicContext &gc);
		void unloadResource(ResourceSpt &resource, CL_GraphicContext &gc, bool unload_quickload = false);

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
