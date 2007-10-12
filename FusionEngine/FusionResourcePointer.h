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

#ifndef Header_FusionEngine_ResourcePointer
#define Header_FusionEngine_ResourcePointer

#if _MSC_VER > 1000
#	pragma once
#endif

#include "Common.h"

namespace FusionEngine
{

	//! The ResourcePointer system is a simple garbage collection system.
	/*!
	 * The ResourceManager manages a collection owning pointers
	 * (Resource objects) and, via reference counting, is notified when
	 * a resource has no external references (ResourcePointers) remaining.
	 * If a resource has no related external references, the memory
	 * related to it will be freed next time the ResourceManager does
	 * garbage collection
	 * (e.g. between levels, /after/ loading the next level's data
	 * (to minimise unnecessary re-loading))
	 *
	 * \sa ResourceManager | Resource
	 */
	template<typename T>
	class ResourcePointer
	{
	protected:
		Resource* m_Resource;

		CL_Slot m_ResourceDestructionSlot;

		static unsigned long ms_NextHash;
		unsigned long m_Hash;

	public:
		//! Basic Constructor
		/*!
		 * Creates an invalid resource pointer
		 */
		ResourcePointer()
			: m_Resource(0)
		{
			m_Hash = ms_NextHash++;
		}

		//! Constructor
		ResourcePointer(Resource* resource)
			: m_Resource(resource)
		{
			m_Hash = ms_NextHash++;
			resource->AddRef(this);
			m_ResourceDestructionSlot = resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		//! Copy constructor
		ResourcePointer(ResourcePointer& other)
			: m_Resource(other.m_Resource)
		{
			m_Hash = ms_NextHash++;
			m_Resource->AddRef(this);
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		//! Destructor
		~ResourcePointer()
		{
			if (m_Resource != 0)
				m_Resource->DropRef(this);
		}

	public:
		//! Returns the resource data ptr
		template<typename T>
		T* GetDataPtr()
		{
			if (m_Resource != 0)
				return m_Resource->GetDataPtr<T>();
			else
				return 0;
		}

		//! Returns the resource data ptr
		template<typename T>
		T const* GetDataPtr() const
		{
			if (m_Resource != 0)
				return m_Resource->GetDataPtr<T>();
			else
				return 0;
		}

		bool IsValid() const
		{
			// Checks that the resource exists at all before checking that it is valid
			return m_Resource == 0 ? false : m_Resource->IsValid();
		}

		unsigned long GetHash() const
		{
			return m_Hash;
		}

		void onResourceDestruction()
		{
			m_Resource = 0;
		}
	};

}

#endif