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

#ifndef H_FusionEngine_Resource
#define H_FusionEngine_Resource

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/signals2/signal.hpp>

#include <functional>

#include <tbb/atomic.h>
#include "tbb/concurrent_queue.h"


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
	private:
		std::string m_Type;
		std::string m_Path;

		tbb::atomic<void*> m_Data;

		bool m_RequiresGC;

		bool m_Loaded;

		tbb::atomic<long int> m_RefCount;

		// These are separate because a resource can be in both at once
		tbb::atomic<bool> m_QueuedToLoad;
		tbb::atomic<bool> m_QueuedToUnLoad;

		std::vector<ResourceDataPtr> m_Dependencies;

	public:
		typedef boost::signals2::signal<void (ResourceDataPtr)> LoadedSignal;
		// When there is lots of listeners, more signals are spawned so they don't all have to be notified within one frame
		tbb::concurrent_queue<std::shared_ptr<LoadedSignal>> SigLoadedExt;
		std::shared_ptr<LoadedSignal> SigLoaded;
		//LoadedSignal SigReLoaded;
		typedef std::function<void (ResourceDataPtr)> LoadedFn;

		typedef std::function<void (ResourceContainer*)> ReleasedFn;
		ReleasedFn NoReferences;

	public:
		//! Constructor
		ResourceContainer();
		//! Constructor
		ResourceContainer(const std::string& type, const std::string& path, void* ptr);

		//! Dtor
		~ResourceContainer();

		//! Returns the type name (of resource loader to be used for this resource)
		const std::string& GetType() const;
		//! Returns the path property
		const std::string& GetPath() const;

		//! Makes this resource hold a reference to another resource that it depends on (e.g. sprites hold textures)
		void AttachDependency(const ResourceDataPtr& dep);
		const std::vector<ResourceDataPtr>& GetDependencies() const;

		//! Used by a resource loader when it requires access to the GC in order to finish loading the resource
		void setRequiresGC(const bool value);
		bool RequiresGC() const;

		//! Sets the data
		void SetDataPtr(void* ptr);
		//! Returns the resource ptr
		void* GetDataPtr();

		//! Validates / invalidates this resource
		/*!
		* A resource is valid if data is loaded. A resource becomes
		* invalid when it fails to load or it is cleaned up by garbage
		* collection.
		* This method is to be used by a ResourceLoader whenever it validates
		* / invalidates a resource.
		*/
		void setLoaded(bool valid);

		//! Returns true if the resource data is valid
		bool IsLoaded() const;

		//! Notifies the resource of its queue status
		inline bool setQueuedToLoad(const bool is_queued);
		//! Returns true if the resource is currently queued to load
		inline bool IsQueuedToLoad() const;

		//! Notifies the resource of its queue status
		inline bool setQueuedToUnload(const bool is_queued);
		//! Returns true if this resource is currently queued to load
		inline bool IsQueuedToUnload() const;

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

		//! Returns true if the given resource is not used (i.e. only referenced by the manager)
		bool Unused() const { return ReferenceCount() == 1; }

	};

	inline bool ResourceContainer::setQueuedToLoad(const bool is_queued)
	{
		return m_QueuedToLoad.fetch_and_store(is_queued);
	}

	inline bool ResourceContainer::IsQueuedToLoad() const
	{
		return m_QueuedToLoad;
	}

	inline bool ResourceContainer::setQueuedToUnload(const bool is_queued)
	{
		return m_QueuedToUnLoad.fetch_and_store(is_queued);
	}

	inline bool ResourceContainer::IsQueuedToUnload() const
	{
		return m_QueuedToUnLoad;
	}

}

#endif