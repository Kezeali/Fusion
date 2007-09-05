/*
  Copyright (c) 2006-2007 Fusion Project Team

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

namespace FusionEngine
{

	//! Maintains a pointer and manages access to data for a single resource.
	/*!
	 * The resource manager maintains a list of Resources, which can hold any
	 * datatype which needs to be loaded from the filesystem.
	 * If a Resource object is deleted, it will delete the data it points to and notify
	 * all related ResourcePointer objects that they have been invalidated.
	 *
	 * \sa ResourceManager | ResourcePointer | ResourceLoader
	 */
	class Resource
	{
	protected:
		std::string m_Type;
		ResourceTag m_Tag;
		std::string m_Path;
		void* m_Data;
		bool m_Valid;
		int m_RefCount;

	public:
		CL_Signal_v0 OnDestruction;
		CL_Signal_v0 OnInvalidation;

	public:
		//! Constructor
		Resource()
			: m_Type(""),
			m_Tag(""),
			m_Path(""),
			m_Data(0),
			m_RefCount(0)
		{
			_setValid(false);
		}

		//! Constructor
		template<typename T>
		Resource(const char* type, const ResourceTag &tag, std::string path, T* ptr)
			: m_Type(type),
			m_Tag(tag),
			m_Path(path),
			m_Data(ptr),
			m_RefCount(0)
		{
			if (ptr != 0)
				_setValid(true);
		}

		~Resource()
		{
			// Fire Signals
			OnInvalidation();
			OnDestruction();

			if (m_Data != 0)
				delete m_Data;
		}

	public:
		//! Returns the type name (of resource loader to be used for this resource)
		const char* GetType() const
		{
			return m_Type;
		}
		//! Returns the resource tag which should point to the Rsc
		ResourceTag GetTag() const
		{
			return m_Tag;
		}
		//! Returns the text description of this resource (used by the ResourceLoader)
		const std::string &GetText() const
		{
			return m_Path;
		}
		//! Returns the path property
		const std::string& GetPath() const
		{
			return m_Path;
		}

		//! Specifically for StringLoader
		/*!
		 * Allows StringLoader to save memory by making the Data property point directly
		 * to the Text property (yes, this is very dumb... but I can't think of a better way '_')
		 */
		std::string *_getTextPtr()
		{
			return &m_Text;
		}

		//! Sets the data (with given type)
		template<typename T>
		void SetDataPtr(T* ptr)
		{
			m_Data = ptr;
		}
		//! Sets the data
		void SetDataPtr(void* ptr)
		{
			m_Data = ptr;
		}

		//! Operator version of SetDataPtr()
		void operator=(void* ptr)
		{
			m_Data = ptr;
		}

		//! Returns the resource ptr (static cast)
		template<typename T>
		T* GetDataPtr()
		{
			return static_cast<T*>(m_Data);
		}

		//! Returns the resource ptr
		void* GetDataPtr()
		{
			return m_Data;
		}

		//! Returns the resource object
		template<typename T>
		T &GetData() const
		{
			return *(static_cast<T*>(m_Data));
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
			m_Valid = valid;
			if (!valid)
				OnInvalidation();
		}

		//! Returns true if the resource ptr is valid
		bool IsValid() const
		{
			return m_Valid;
		}

		//! Increments ref count
		void AddRef()
		{
			m_RefCount++;
		}

		//! Decrements ref count
		void DropRef()
		{
			m_RefCount--;
		}

		//! Returns true if this resource is referenced
		bool IsReferenced() const
		{
			return m_RefCount > 0;
		}
	};

}

#endif