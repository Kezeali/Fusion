/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionResource.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <boost/thread/locks.hpp>
#endif


namespace FusionEngine
{

	void intrusive_ptr_add_ref(ResourceContainer *ptr)
	{
		ptr->AddReference();
	}

	void intrusive_ptr_release(ResourceContainer *ptr)
	{
		ptr->RemoveReference();
	}

	ResourceContainer::ResourceContainer()
		: m_Type(""),
		m_Path(""),
		m_Data(NULL),
		m_QuickLoadData(NULL),
		m_HasQuickLoadData(false),
		m_RefCount(0),
		m_ToLoad(false),
		m_ToUnload(false)
	{
		_setValid(false);
	}

	ResourceContainer::ResourceContainer(const char* type, const std::string& path, void* ptr)
		: m_Type(type),
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

	ResourceContainer::ResourceContainer(const std::string& type, const std::wstring& path, void* ptr)
		: m_Type(type),
		m_Path(fe_narrow(path)),
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

	ResourceContainer::ResourceContainer(const std::string& type, const std::string& path, void* ptr)
		: m_Type(type),
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

	ResourceContainer::~ResourceContainer()
	{
#ifdef _DEBUG
		if (m_Loaded || m_Data != NULL)
		{
			SendToConsole("Resource '" + m_Path + "' may not have been properly dellocated before deletion - Resource Data leaked.");
		}
		if (m_HasQuickLoadData || m_QuickLoadData != NULL)
		{
			SendToConsole("Resource '" + m_Path + "' may not have been properly dellocated before deletion - QuickLoad Data leaked.");
		}
#endif
	}

	const std::string& ResourceContainer::GetType() const
	{
		return m_Type;
	}

	const std::string& ResourceContainer::GetPath() const
	{
		return m_Path;
	}

	std::string *ResourceContainer::_getTextPtr()
	{
		return &m_Path;
	}

	void ResourceContainer::SetDataPtr(void* ptr)
	{
		m_Data = ptr;
	}

	void* ResourceContainer::GetDataPtr()
	{
		return m_Data;
	}

	void ResourceContainer::_setValid(bool valid)
	{
		m_Loaded = valid;
		//if (valid)
		//	SigLoad();
		//else
		//	SigUnload();
	}

	bool ResourceContainer::IsLoaded() const
	{
		return m_Loaded;
	}

	void ResourceContainer::SetQuickLoadDataPtr(void* ptr)
	{
		m_QuickLoadData = ptr;
	}

	void* ResourceContainer::GetQuickLoadDataPtr()
	{
		return m_QuickLoadData;
	}

	void ResourceContainer::_setHasQuickLoadData(bool has_data)
	{
		m_HasQuickLoadData = has_data;
	}

	bool ResourceContainer::HasQuickLoadData() const
	{
		return m_HasQuickLoadData;
	}

	void ResourceContainer::_setQueuedToLoad(bool is_queued)
	{
		m_ToLoad = is_queued;
	}

	bool ResourceContainer::IsQueuedToLoad() const
	{
		return m_ToLoad;
	}

	void ResourceContainer::_setQueuedToUnoad(bool is_queued)
	{
		m_ToUnload = is_queued;
	}

	bool ResourceContainer::IsQueuedToUnload() const
	{
		return m_ToUnload;
	}

	void ResourceContainer::AddReference()
	{
#ifdef _WIN32
		InterlockedIncrement(&m_RefCount);
#else
		boost::lock_guard<boost::mutex> lock(m_Mutex);
		++m_RefCount;
#endif
	}

	void ResourceContainer::RemoveReference()
	{
#ifdef _WIN32
		long refCount = InterlockedDecrement(&m_RefCount);
#else
		boost::lock_guard<boost::mutex> lock(m_Mutex);
		long refCount = --m_RefCount;
#endif
		if (refCount == 1)
		{
			if (NoReferences) NoReferences(this);
		}
#ifdef _DEBUG
		else if (refCount == 0)
			SendToConsole("Resource ref-count reached zero without being deleted. Path: " + m_Path);
#endif
	}

	long ResourceContainer::ReferenceCount() const
	{
		return m_RefCount;
	}

	bool ResourceContainer::Unused() const
	{
		return ReferenceCount() == 1;
	}

}
