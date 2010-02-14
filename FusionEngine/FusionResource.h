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

#ifndef Header_FusionEngine_Resource
#define Header_FusionEngine_Resource

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include <boost/signals2/signal.hpp>
#ifndef _WIN32
#include <boost/thread/mutex.hpp>
#endif

#include <functional>

#ifdef _DEBUG
#include "FusionConsole.h"
#endif


namespace FusionEngine
{

	void intrusive_ptr_add_ref(ResourceContainer *ptr);
	void intrusive_ptr_release(ResourceContainer *ptr);

	//! Resource container shared pointer
	typedef boost::intrusive_ptr<ResourceContainer> ResourceDataPtr;

	//! Maintains a pointer and manages access to data for a single resource.
	/*!
	* The resource manager maintains a list of Resources, which can hold any
	* datatype which needs to be loaded from the filesystem.
	*
	* \sa ResourceManager | ResourcePointer | ResourceLoader
	*/
	class ResourceContainer
	{
	protected:
		std::string m_Type;
		//ResourceTag m_Tag;
		std::string m_Path;

		void *m_Data;
		// Data that isn't unloaded until the resource is released
		void *m_QuickLoadData;

		bool m_Loaded;
		bool m_HasQuickLoadData;

		volatile long m_RefCount;

#ifndef _WIN32
		boost::mutex m_Mutex;
#endif

		bool m_ToLoad;
		bool m_ToUnload;

	public:
		boost::signals2::signal<void ()> SigDelete;
		boost::signals2::signal<void ()> SigLoad;
		boost::signals2::signal<void ()> SigUnload;

		typedef boost::signals2::signal<void (ResourceDataPtr)> LoadedSignal;
		LoadedSignal SigLoaded;

		typedef std::function<void (ResourceDataPtr)> LoadedFn;

		typedef std::function<void (ResourceContainer*)> ReleasedFn;
		ReleasedFn NoReferences;

	public:
		//! Constructor
		ResourceContainer();
		//! Constructor
		ResourceContainer(const char* type, const std::string& path, void* ptr);
		//! Constructor (unicode)
		ResourceContainer(const std::string& type, const std::wstring& path, void* ptr);
		//! Constructor
		ResourceContainer(const std::string& type, const std::string& path, void* ptr);

		//! Dtor
		~ResourceContainer();

	public:
		//! Returns the type name (of resource loader to be used for this resource)
		const std::string& GetType() const;
		//! Returns the path property
		const std::string& GetPath() const;

		//! Specifically for StringLoader
		/*!
		* Allows StringLoader to save memory by making the Data property point directly
		* to the Path property (yes, this is very dumb... but I can't think of a better way o_o)
		*/
		std::string *_getTextPtr();

		//! Sets the data
		void SetDataPtr(void* ptr);
		//! Returns the resource ptr
		void* GetDataPtr();

		//! Validates / invalidates this resource
		/*!
		* A resource is valid if the pointer is valid. A resource becomes
		* invalid when it fails to load or it is cleaned up by garbage
		* collection.
		* This method is to be used by a ResourceLoader whenever it validates
		* / invalidates a resource.
		*/
		void _setValid(bool valid);

		//! Returns true if the resource data is valid
		bool IsLoaded() const;

		//! Sets the data
		void SetQuickLoadDataPtr(void* ptr);
		//! Returns the resource ptr
		void* GetQuickLoadDataPtr();

		//! Validates / invalidates this resource
		/*!
		* A resource is valid if the pointer is valid. A resource becomes
		* invalid when it fails to load or it is cleaned up by garbage
		* collection.
		* This method is to be used by a ResourceLoader whenever it validates
		* / invalidates a resource.
		*/
		void _setHasQuickLoadData(bool has_data);
		//! Returns true if the resource data is valid
		bool HasQuickLoadData() const;

		//! Notifies the resource of its queue status
		void _setQueuedToLoad(bool is_queued);
		//! Returns true if the resource is currently queued to load
		bool IsQueuedToLoad() const;

		//! Notifies the resource of its queue status
		void _setQueuedToUnoad(bool is_queued);
		//! Returns true if this resource is currently queued to load
		bool IsQueuedToUnload() const;

		//! Adds a reference
		void AddReference();
		//! Removes a ref.
		/*!
		* If the ref count reaches 1 (i.e. the ResourceManager's reference)
		* the NoReferences callback is called, which should be set to a
		* method which notifies the ResourceManager so it can unload the
		* resource (in it's own time, of course)
		*/
		void RemoveReference();

		//! Returns the current number of references (including the ResourceManager's reference)
		long ReferenceCount() const;

		//! Retures true if the given resource is not used (i.e. only referenced by the manager)
		bool Unused() const;

	};

}

#endif