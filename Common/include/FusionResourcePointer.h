/*
*  Copyright (c) 2006-2007 Fusion Project Team
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

#ifndef H_FusionEngine_ResourcePointer
#define H_FusionEngine_ResourcePointer

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionResource.h"

#ifdef _DEBUG
#include "FusionConsole.h"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

/*!
 * Define FSN_RESOURCEPOINTER_USE_WEAKPTR to use built-in reference counting for garbage collection.
 * Otherwise std::shared_ptr will be used. <br>
 * Using shared_ptr makes the implementation much simpler on my side :)
 */
//#define FSN_RESOURCEPOINTER_USE_WEAKPTR

namespace FusionEngine
{

	//! Resource container weak pointer
	typedef boost::weak_ptr<ResourceContainer> ResourceWpt;

	//! Makes accessing resource data easier.
	/*!
	 * The basic resource class accesses loaded data via a void*.
	 * To access data by it's actual type a Resource can be
	 * held by an instance of the ResourcePointer class.
	 *
	 * \sa ResourceManager | ResourceContainer
	 */
	template<typename T>
	class ResourcePointer
	{
	protected:
		ResourceDataPtr m_Resource;

	public:
		//! Basic Constructor
		/*!
		 * Creates a null resource pointer
		 */
		ResourcePointer()
		{
		}

		//! Constructor
		ResourcePointer(ResourceDataPtr &resource)
			: m_Resource(resource)
		{
		}

		//! Copy constructor
		ResourcePointer(const ResourcePointer<T>& other)
			: m_Resource(other.m_Resource)
		{
		}


		//! Destructor
		~ResourcePointer()
		{
//#if defined (_DEBUG) && defined(FSN_RESOURCEPOINTER_USE_WEAKPTR)
//			SendToConsole("ResourcePointer<" + std::string(typeid(T).name()) +"> to " + 
//				(IsValid() ? "\'" + m_ResourceBox.lock()->GetPath() + "\'" : "invalid resource") + " deleted");
//#elif defined (_DEBUG)
//			SendToConsole("ResourcePointer<" + std::string(typeid(T).name()) +"> to " + 
//				(IsValid() ? "\'" + m_ResourceBox->GetPath() + "\'" : "invalid resource") + " deleted");
//#endif
		}

	public:

		//! Assignment operator
		ResourcePointer<T>& operator=(const ResourcePointer<T>& r)
		{
			m_Resource = r.m_Resource;

			return *this;
		}

		//! Comparison
		bool operator==(const ResourcePointer<T>& other)
		{
			return m_Resource == other.m_Resource;
		}

		//! Returns the resource data ptr
		T* Get()
		{
			return static_cast<T*>( m_Resource->GetDataPtr() );
		}

		//! Returns the resource data ptr
		T const* Get() const
		{
			return static_cast<const T*>( m_Resource->GetDataPtr() );
		}

		//! Indirect member access operator.
		T* operator->()
		{
			return static_cast<T*>( m_Resource->GetDataPtr() );
		}

		T const* operator->() const
		{
			return static_cast<const T*>( m_Resource->GetDataPtr() );
		}

		void SetTarget(ResourceDataPtr &resource)
		{
			m_Resource = resource;
		}

		void Release()
		{
			m_Resource.reset();
		}

		bool IsLoaded() const
		{
			// Check that this pointer is valid (the resource container exists), before checking that the data is valid
			return m_Resource ? m_Resource->IsLoaded() : false; 
		}

		bool IsNull() const
		{
			return !(m_Resource);
		}

	};

	//! Callback helper
	template <typename T>
	class ResourcePointerSetter
	{
		ResourcePointerSetter(ResourcePointer<T>& ptr)
			: pointer(ptr)
		{}

		void operator() (ResourceDataPtr data)
		{
			pointer.SetTarget(data);
		}

		ResourcePointer<T>& pointer;
	};

	//! Makes a callback helper
	template <typename T>
	static ResourcePointerSetter<T> make_load_callback(ResourcePointer<T>& ptr)
	{
		return ResourcePointerSetter<T>(ptr);
	}

}

#endif
