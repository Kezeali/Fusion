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

#ifdef _DEBUG
#include "FusionConsole.h"
#endif

namespace FusionEngine
{

	//! The ResourceToken system is a simple garbage collection system.
	/*!
	 * The ResourceManager manages a collection owning pointers
	 * (Resource objects) and, via reference counting, is notified when
	 * a resource has no external references (ResourceToken) remaining.
	 * If a resource has no related external references, the memory
	 * related to it will be freed next time the ResourceManager does
	 * garbage collection
	 * (e.g. between levels, /after/ loading the next level's data
	 * (to minimise unnecessary re-loading))
	 *
	 * \todo Rename as ResourceToken
	 *
	 * \todo Make this copyable (fix copy-constructor / assignment constructor)
	 *
	 * \sa ResourceManager | Resource
	 */
	template<typename T = void*>
	class ResourcePointer
	{
	protected:
		Resource* m_Resource;

		CL_Slot m_ResourceDestructionSlot;

		unsigned long m_Hash;

		bool m_Valid;

		Resource::ResourcePtrTicket m_Ticket;

	public:
		//! Basic Constructor
		/*!
		 * Creates an invalid resource pointer
		 */
		ResourcePointer()
			: m_Resource(0),
			m_Ticket(0),
			m_Valid(false)
		{
		}

		//! Constructor
		ResourcePointer(Resource* resource)
			: m_Resource(resource),
			m_Valid(true)
		{
			m_Ticket = resource->AddRef();
			m_ResourceDestructionSlot = resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		//! Copy constructor
		template <typename Y>
		explicit ResourcePointer(ResourcePointer<Y>& other)
			: m_Resource(other.m_Resource),
			m_Valid(true)
		{
			m_Ticket = m_Resource->AddRef();
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		template <typename Y>
		ResourcePointer(ResourcePointer<Y> const& other)
			: m_Resource(other.m_Resource), 
			m_Ticket(other.m_Ticket),
			m_Valid(true)
		{
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		ResourcePointer(ResourcePointer const& other)
			: m_Resource(other.m_Resource), 
			m_Ticket(other.m_Ticket),
			m_Valid(true)
		{
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}


		//! Destructor
		~ResourcePointer()
		{
#ifdef _DEBUG
			SendToConsole("ResourcePointer deleted");
#endif
			if (m_Resource != 0)
				m_Resource->DropRef(this->GetTicket());
		}

	public:
		template<class Y>
		ResourcePointer & operator=(ResourcePointer<Y> const & r)
		{
			m_Resource = r.m_Resource;
			m_Ticket = r.m_Ticket;
			m_Valid = r.m_Valid;
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
			return *this;
		}
		ResourcePointer & operator=(ResourcePointer const & r)
		{
			m_Resource = r.m_Resource;
			m_Ticket = r.m_Ticket;
			m_Valid = r.m_Valid;
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
			return *this;
		}

		//! Returns the resource data ptr
		template<typename T>
		T* GetDataPtr()
		{
			if (m_Resource != 0)
				return (T*)m_Resource->GetDataPtr();
			else
				return 0;
		}

		//! Returns the resource data ptr
		template<typename T>
		T const* GetDataPtr() const
		{
			if (m_Resource != 0)
				return (T*)m_Resource->GetDataPtr();
			else
				return 0;
		}

		//! Indirect member access operator.
		T* operator->()
		{ return (T*)m_Resource->GetDataPtr(); }

		T const* operator->() const
		{ return (const T*)m_Resource->GetDataPtr(); }

		void SetTarget(Resource* resource)
		{
			if (m_Resource != NULL)
			{
				m_Resource->OnDestruction.disconnect(m_ResourceDestructionSlot);
				m_Resource->DropRef(m_Ticket);
			}

			m_Resource = resource;
			m_Ticket = m_Resource->AddRef();
			m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		}

		bool IsValid() const
		{
			// Checks that the resource exists at all before checking that it is valid
			return m_Resource == 0 ? false : m_Resource->IsValid();
		}

		Resource::ResourcePtrTicket GetTicket() const
		{
			return m_Ticket;
		}

		void onResourceDestruction()
		{
			m_Valid = false;
			m_Resource = 0;
		}
	};

}

#endif