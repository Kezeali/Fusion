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

#include "FusionBoostSignals2.h"

//#include <functional>

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
	public:
		typedef std::set<ResourceDataPtr> DependenciesSet;

	protected:
		std::string m_Type;
		//ResourceTag m_Tag;
		std::wstring m_Path;

		void *m_Data;
		// Data that isn't unloaded until the resource is released
		void *m_QuickLoadData;

		bool m_Loaded;
		bool m_HasQuickLoadData;

		DependenciesSet m_Dependencies;

		volatile long m_RefCount;

		CL_Mutex m_Mutex;

		bool m_ToLoad;
		bool m_ToUnload;

	public:
		bsig2::signal<void ()> SigDelete;
		bsig2::signal<void ()> SigLoad;
		bsig2::signal<void ()> SigUnload;

		typedef boost::signals2::signal<void (ResourceDataPtr)> LoadedSignal;
		LoadedSignal SigLoaded;

		typedef std::tr1::function<void (ResourceDataPtr)> LoadedFn;

		typedef std::tr1::function<void (ResourceContainer*)> ReleasedFn;
		ReleasedFn NoReferences;

	public:
		//! Constructor
		ResourceContainer()
			: m_Type(""),
			//m_Tag(L""),
			m_Path(L""),
			m_Data(NULL),
			m_QuickLoadData(NULL),
			m_HasQuickLoadData(false),
			m_RefCount(0),
			m_ToLoad(false),
			m_ToUnload(false)
		{
			_setValid(false);
		}

		//! Constructor
		ResourceContainer(const char* type, const std::wstring& path, void* ptr)
			: m_Type(type),
			//m_Tag(tag),
			m_Path(path),
			m_Data(ptr),
			m_QuickLoadData(NULL),
			m_HasQuickLoadData(false),
			m_RefCount(0),
			m_ToLoad(false),
			m_ToUnload(false)
		{
			if (ptr != 0)
				_setValid(true);
			else
				_setValid(false);
		}

		//! Constructor
		ResourceContainer(const std::string& type, const std::wstring& path, void* ptr)
			: m_Type(type),
			//m_Tag(tag),
			m_Path(path),
			m_Data(ptr),
			m_QuickLoadData(NULL),
			m_HasQuickLoadData(false),
			m_RefCount(0),
			m_ToLoad(false),
			m_ToUnload(false)
		{
			if (ptr != 0)
				_setValid(true);
			else
				_setValid(false);
		}

		//! Constructor
		ResourceContainer(const std::string& type, const std::string& path, void* ptr)
			: m_Type(type),
			//m_Tag(fe_widen(tag)),
			m_Path(fe_widen(path)),
			m_Data(ptr),
			m_QuickLoadData(NULL),
			m_HasQuickLoadData(false),
			m_RefCount(0),
			m_ToLoad(false),
			m_ToUnload(false)
		{
			if (ptr != 0)
				_setValid(true);
			else
				_setValid(false);
		}

		~ResourceContainer()
		{
#ifdef _DEBUG
			if (m_Loaded || m_Data != NULL)
			{
				SendToConsole(L"Resource '" + m_Path + L"' may not have been properly dellocated before deletion - Resource Data leaked.");
			}
			if (m_HasQuickLoadData || m_QuickLoadData != NULL)
			{
				SendToConsole(L"Resource '" + m_Path + L"' may not have been properly dellocated before deletion - QuickLoad Data leaked.");
			}
#endif
		}

	public:
		//! Returns the type name (of resource loader to be used for this resource)
		const std::string& GetType() const
		{
			return m_Type;
		}
		//! Returns the resource tag which should point to the Rsc
		//const ResourceTag& GetTag() const
		//{
		//	return m_Tag;
		//}
		//! Returns the path property
		const std::wstring& GetPath() const
		{
			return m_Path;
		}

		//! Specifically for StringLoader
		/*!
		 * Allows StringLoader to save memory by making the Data property point directly
		 * to the Path property (yes, this is very dumb... but I can't think of a better way o_o)
		 */
		std::wstring *_getTextPtr()
		{
			return &m_Path;
		}

		//! Sets the data
		void SetDataPtr(void* ptr)
		{
			m_Data = ptr;
		}

		////! Returns the resource ptr (cast)
		//template<typename T>
		//T* GetDataPtr()
		//{
		//	return dynamic_cast<T*>(m_Data);
		//}

		//! Returns the resource ptr
		void* GetDataPtr()
		{
			return m_Data;
		}

		//! Returns the resource object
		template<typename T>
		T &GetData() const
		{
			FSN_ASSERT(IsLoaded());
			return *(dynamic_cast<T*>(m_Data));
		}

		//! Validates / invalidates this resource
		/*!
		 * A resource is valid if the pointer is valid. A resource becomes
		 * invalid when it fails to load or it is cleaned up by garbage
		 * collection.
		 * This method is to be used by a ResourceLoader whenever it validates
		 * / invalidates a resource.
		 */
		void _setValid(bool valid)
		{
			m_Loaded = valid;
			//if (valid)
			//	SigLoad();
			//else
			//	SigUnload();
		}

		//! Returns true if the resource data is valid
		bool IsLoaded() const
		{
			return m_Loaded;
		}

		//! Sets the data
		void SetQuickLoadDataPtr(void* ptr)
		{
			m_QuickLoadData = ptr;
		}

		//! Returns the resource ptr
		void* GetQuickLoadDataPtr()
		{
			return m_QuickLoadData;
		}

		//! Validates / invalidates this resource
		/*!
		 * A resource is valid if the pointer is valid. A resource becomes
		 * invalid when it fails to load or it is cleaned up by garbage
		 * collection.
		 * This method is to be used by a ResourceLoader whenever it validates
		 * / invalidates a resource.
		 */
		void _setHasQuickLoadData(bool has_data)
		{
			m_HasQuickLoadData = has_data;
		}

		//! Returns true if the resource data is valid
		bool HasQuickLoadData() const
		{
			return m_HasQuickLoadData;
		}

		void DependsOn(ResourceContainer *resource)
		{
			m_Dependencies.insert(ResourceDataPtr(resource));
		}

		void _addDependant(ResourceContainer *dependant)
		{
			dependant->DependsOn(this);
		}

		void _setQueuedToLoad(bool is_queued)
		{
			m_ToLoad = is_queued;
		}

		bool IsQueuedToLoad() const
		{
			return m_ToLoad;
		}

		void _setQueuedToUnoad(bool is_queued)
		{
			m_ToUnload = is_queued;
		}

		bool IsQueuedToUnload() const
		{
			return m_ToUnload;
		}


		void AddReference()
		{
			InterlockedIncrement(&m_RefCount);
		}

		void RemoveReference()
		{
			long refCount = InterlockedDecrement(&m_RefCount);
			if (refCount == 1)
			{
				if (NoReferences) NoReferences(this);
			}
#ifdef _DEBUG
			else if (refCount == 0)
				SendToConsole(L"Resource ref-count reached zero without being deleted. Resource Name: " + m_Path);
#endif
		}

		long ReferenceCount() const
		{
			return m_RefCount;
		}

		//! Retures true if the given resource is not used (i.e. only referenced by the manager)
		bool Unused() const
		{
			return ReferenceCount() == 1;
		}

		//! Makes this resource immutable
		/*!
		* Returns false if the resource is invalid (not loaded)
		* and thus cannot be locked.
		*/
		//bool Lock()
		//{
		//	// Only lock if this resource is currently loaded
		//	if (IsValid())
		//	{
		//		m_Mutex.lock();
		//		return true;
		//	}
		//	else
		//		return false;
		//}

		//! Makes this resource mutable
		//void Unlock()
		//{
		//	m_Mutex.unlock();
		//}

		//! Increments ref count
		//void AddRef()
		//{
		//	m_RefCount++;
		//}

//		//! Decrements ref count
//		void DropRef()
//		{
//			m_RefCount--;
//		}
//
//		//! Increments ref count
//		ResourcePtrTicket AddRef()
//		{
//#ifdef FSN_RESOURCE_USE_TICKETS
//			m_RefTickets.push_back(m_NextTicket);
//			return m_NextTicket++;
//#else
//			return m_RefCount++;
//#endif
//		}
//
//		//! Decrements ref count
//		void DropRef(ResourcePtrTicket ticket)
//		{
//#ifdef FSN_RESOURCE_USE_TICKETS
//			for (TicketList::iterator it = m_RefTickets.begin(); it != m_RefTickets.end(); ++it)
//			{
//				if (*it == ticket)
//				{
//					m_RefTickets.erase(it);
//					break;
//				}
//			}
//
//#else
//			m_RefCount--;
//#endif
//		}
//
//		//! Returns true if this resource is referenced
//		bool IsReferenced() const
//		{
//#ifdef FSN_RESOURCE_USE_TICKETS
//			return !m_RefTickets.empty();
//
//#else
//			return m_RefCount > 0;
//#endif
//		}
	};

}

#endif