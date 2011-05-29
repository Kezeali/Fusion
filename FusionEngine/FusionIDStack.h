/*
*  Copyright (c) 2010 Fusion Project Team
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

#ifndef H_FusionEngine_IdStack
#define H_FusionEngine_IdStack

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <limits>

namespace FusionEngine
{

	class IdCollectionException : public Exception
	{
	public:
		IdCollectionException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};

	class NoMoreIdsException : public IdCollectionException
	{
	public:
		NoMoreIdsException(const std::string& description, const std::string& origin, const char* file, long line)
			: IdCollectionException(description, origin, file, line) {}
	};

	template <typename T, class CollectionType = std::deque<T>>
	class IDCollection
	{
	public:
		typedef T IDType;

		//! Initialises m_NextId to zero
		IDCollection()
			: m_FirstId(0),
			m_NextId(0)
		{}
		//! Initialises m_NextId to the given value
		IDCollection(T first_id)
			: m_FirstId(first_id),
			m_NextId(first_id)
		{}
		//! Virtual destructor
		virtual ~IDCollection()
		{}

		//! Returns an ID which is not in use
		virtual T getFreeID() = 0;
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id) = 0;

		//! Resets this object
		void freeAll()
		{
			m_UnusedIds.clear();
			m_NextId = m_FirstId;
		}
		//! Takes all the Ids up to the given one, so the next call to getFreeID() will return the given ID
		void takeAll(T next_id)
		{
			m_UnusedIds.clear();
			m_NextId = next_id;
		}

	protected:
		T m_FirstId;
		// The next ID to use when m_UnusedIDs is empty
		T m_NextId;
		// Lists IDs between 0 and m_NextId that have been freed by Entity removal
		CollectionType m_UnusedIds;
	};

	//! Supplies IDs which aren't assigned, starting with the lowest ID available
	/*!
	* O(log n) (when the ID is in the set, rather than the next id).
	* Constant time when the id is the next id.
	*/
	template <typename T>
	class IDSet : public IDCollection<T, std::set<T>>
	{
	protected:
		typedef std::set<T> IDCollectionType;

	public:
		//! Default
		IDSet()
			: IDCollection()
		{}
		//! Initialises m_NextId to the given value
		IDSet(T first_id)
			: IDCollection(first_id)
		{}
		//! Virtual destructor
		virtual ~IDSet()
		{}
		//! Returns the next ID that will be returned by getFreeID
		T peekNextID() const
		{
			if (m_UnusedIds.empty())
				return m_NextId+1;
			else
				return *m_UnusedIds.begin();
		}
		//! Returns an ID which is not in use
		virtual T getFreeID()
		{
			if (m_UnusedIds.empty())
				return m_NextId++;
			else
			{
				IDCollectionType::iterator lowest = m_UnusedIds.begin();
				ObjectID id = *lowest;
				m_UnusedIds.erase(lowest);
				return id;
			}
		}
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id)
		{
			if (id == m_NextId-1)
				m_NextId = id;
			else if (id < m_NextId-1)
				m_UnusedIds.insert(id); // record unused ID
		}
		//! Removes the given ID from the set
		/*!
		* \return Returns true if the ID was unused (so it could be taken), false otherwise
		*/
		virtual bool takeID(T id)
		{
			if (id == m_NextId)
			{
				++m_NextId;
				return true;
			}
			else if (id < m_NextId)
			{
				IDCollectionType::iterator _where = m_UnusedIds.find(id);
				if (_where != m_UnusedIds.end())
				{
					m_UnusedIds.erase(_where);
					return true;
				}
				else
					return false;
			}
			else // the id being taken is after the current index: jump forward to it
			{
				for (T unusedId = m_NextId; unusedId < id; ++unusedId)
					m_UnusedIds.insert(unusedId);
				m_NextId = id + 1;
				return true;
			}
		}
	};

	//! Supplies IDs which aren't assigned, starting with recently unassigned IDs
	/*!
	* O(1) for all operations (unless compiled for debug, then checks for duplicate calls to freeID())
	*/
	template <typename T>
	class IDStack : public IDCollection<T>
	{
	public:
		//! Default CTOR
		IDStack()
			: IDCollection(),
			m_MaxId(std::numeric_limits<T>::max())
		{}
		//! Sets the first id
		IDStack(T first)
			: IDCollection(first),
			m_MaxId(std::numeric_limits<T>::max())
		{}
		//! Inits like the single param constructor, but also sets the max ID available
		IDStack(T first, T max)
			: IDCollection(first),
			m_MaxId(max)
		{}
		//! Virtual destructor
		virtual ~IDStack()
		{}

		void setMaxID(T max)
		{
			FSN_ASSERT(m_NextId == m_FirstId);
			FSN_ASSERT(m_UnusedIds.empty());
			m_MaxId = max;
		}

		T peekNextID() const
		{
			if (m_UnusedIds.empty())
				return m_NextId+1;
			else
				return m_UnusedIds.back();
		}

		//! Returns an ObjectID which is not in use
		virtual T getFreeID()
		{
			if (m_NextId > m_MaxId)
				FSN_EXCEPT(NoMoreIdsException, "No more IDs are available");

			if (m_UnusedIds.empty())
				return m_NextId++;
			else
			{
				ObjectID id = m_UnusedIds.back();
				m_UnusedIds.pop_back();
				return id;
			}
		}
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id)
		{
			if (id == m_NextId-1)
				--m_NextId;
			else if (id < m_NextId-1)
			{
				FSN_ASSERT_MSG(std::find(m_UnusedIds.begin(), m_UnusedIds.end(), id) != m_UnusedIds.end(),
					"Redundant call to freeID: the given ID is already free");
				m_UnusedIds.push_back(id); // record unused ID
			}
		}

	protected:
		T m_MaxId;
	};

	//! Template specialization of the default IDSet ctor, to make ObjectID generators start at 1
	//template <>
	//IDSet<ObjectID>::IDSet()
	//	: IDCollection(1)
	//{}
	//! Supplies ObjectIDs which aren't assigned
	class ObjectIDSet : public IDSet<ObjectID>
	{
	public:
		//! Initialises the first ID to 1 (Entity IDs start at 1)
		ObjectIDSet()
			: IDSet(1)
		{}
	};

	//! Supplies ObjectIDs which aren't assigned
	class ObjectIDStack : public IDStack<ObjectID>
	{
	public:
		//! Initialises the first ID to 1 (Entity IDs start at 1)
		ObjectIDStack()
			: IDStack(1)
		{}
		//! Sets the max ID
		ObjectIDStack(ObjectID max)
			: IDStack(1, max)
		{}
	};

}

#endif
