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

#include "PrecompiledHeaders.h"

#include "FusionResource.h"

#ifdef _DEBUG
#include "FusionConsole.h"
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
		m_Data(nullptr),
		m_QuickLoadData(nullptr),
		m_RequiresGC(false),
		m_HasQuickLoadData(false)
	{
		m_RefCount = 0;
		m_QueuedToLoad = false;
		m_QueuedToUnLoad = false;
		setLoaded(false);
	}

	ResourceContainer::ResourceContainer(const std::string& type, const std::string& path, void* ptr)
		: m_Type(type),
		m_Path(path),
		m_Data(ptr),
		m_QuickLoadData(nullptr),
		m_RequiresGC(false),
		m_HasQuickLoadData(false)
	{
		m_RefCount = 0;
		m_QueuedToLoad = false;
		m_QueuedToUnLoad = false;
		setLoaded(ptr != nullptr);
	}

	ResourceContainer::~ResourceContainer()
	{
#ifdef _DEBUG
		if (m_Loaded || m_Data != nullptr)
		{
			SendToConsole("Resource '" + m_Path + "' may not have been properly dellocated before deletion - Resource Data leaked.");
		}
		if (m_HasQuickLoadData || m_QuickLoadData != nullptr)
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

	void ResourceContainer::AttachDependency(ResourceDataPtr& dep)
	{
		m_Dependencies.push_back(dep);
	}

	const std::vector<ResourceDataPtr>& ResourceContainer::GetDependencies() const
	{
		return m_Dependencies;
	}

	void ResourceContainer::setRequiresGC(const bool value)
	{
		m_RequiresGC = value;
	}

	bool ResourceContainer::RequiresGC() const
	{
		return m_RequiresGC;
	}

	void ResourceContainer::SetDataPtr(void* ptr)
	{
		m_Data = ptr;
	}

	void* ResourceContainer::GetDataPtr()
	{
		return m_Data;
	}

	void ResourceContainer::setLoaded(bool valid)
	{
		m_Loaded = valid;
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

	void ResourceContainer::setHasQuickLoadData(bool has_data)
	{
		m_HasQuickLoadData = has_data;
	}

	bool ResourceContainer::HasQuickLoadData() const
	{
		return m_HasQuickLoadData;
	}

	void ResourceContainer::AddReference()
	{
		++m_RefCount;
	}

	void ResourceContainer::RemoveReference()
	{
		auto refCount = --m_RefCount;
		if (refCount == 1)
		{
			if (NoReferences) NoReferences(this);
		}
		else if (refCount == 0)
		{
#ifdef _DEBUG
			if (m_Data != nullptr)
				SendToConsole("Resource ref-count reached zero without being unloaded. Path: " + m_Path);
#endif
			delete this;
		}
	}

	long ResourceContainer::ReferenceCount() const
	{
		return m_RefCount;
	}

}
