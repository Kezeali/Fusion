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

#include "FusionPrerequisites.h"
#include "FusionException.h"
#include "FusionTypes.h"

#include <deque>
#include <boost/dynamic_bitset.hpp>
#include <limits>
#include <set>

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
		typedef ::FusionEngine::IDCollection<T, std::set<T> > base_type;

	public:
		//! Default
		IDSet()
			: base_type()
		{}
		//! Initialises m_NextId to the given value
		IDSet(T first_id)
			: base_type(first_id)
		{}
		//! Virtual destructor
		virtual ~IDSet()
		{}
		//! Returns the next ID that will be returned by getFreeID
		T peekNextID() const
		{
			if (this->m_UnusedIds.empty())
				return this->m_NextId;
			else
				return *(this->m_UnusedIds.begin());
		}
		//! Returns an ID which is not in use
		virtual T getFreeID()
		{
			if (this->m_UnusedIds.empty())
				return this->m_NextId++;
			else
			{
				auto lowest = this->m_UnusedIds.begin();
				ObjectID id = *lowest;
				this->m_UnusedIds.erase(lowest);

				// It's possible for an ID to get freed when it is more than 1 below m_NextId
				//  then "freed" again with another call to freeID(id) when it is one below m_NextId
				//  (see the logic in that fn) -> this ensures that m_NextId will be incremented
				//  if that was the case for the ID just retreived from the set<> (so that the
				//  next call to this method after the set<> is empty wont return the same ID again)
				if (id == this->m_NextId)
					++(this->m_NextId);

				return id;
			}
		}
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id)
		{
			if (id == (this->m_NextId)-1)
				this->m_NextId = id;
			else if (id < this->m_NextId-1)
				this->m_UnusedIds.insert(id); // record unused ID
		}
		//! Removes the given ID from the set
		/*!
		* \return Returns true if the ID was unused (so it could be taken), false otherwise
		*/
		virtual bool takeID(T id)
		{
			if (id == this->m_NextId)
			{
				++(this->m_NextId);
				return true;
			}
			else if (id < this->m_NextId)
			{
				auto _where = this->m_UnusedIds.find(id);
				if (_where != this->m_UnusedIds.end())
				{
					this->m_UnusedIds.erase(_where);
					return true;
				}
				else
					return false;
			}
			else // the id being taken is after the current index: jump forward to it
			{
				for (T unusedId = this->m_NextId; unusedId < id; ++unusedId)
					this->m_UnusedIds.insert(unusedId);
				this->m_NextId = id + 1;
				return true;
			}
		}

		size_t numUsed() const
		{
			return this->m_NextId - this->m_UnusedIds.size();
		}

		size_t numNotUsed() const
		{
			return std::numeric_limits<T>::max() - numUsed();
		}

		bool hasMore() const
		{
			return numUsed() < std::numeric_limits<T>::max();
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
		typedef IDCollection<T> base_type;

		//! Default CTOR
		IDStack()
			: base_type(),
			m_MaxId(std::numeric_limits<T>::max())
		{}
		//! Sets the first id
		IDStack(T first)
			: base_type(first),
			m_MaxId(std::numeric_limits<T>::max())
		{}
		//! Inits like the single param constructor, but also sets the max ID available
		IDStack(T first, T max)
			: base_type(first),
			m_MaxId(max)
		{}
		//! Virtual destructor
		virtual ~IDStack()
		{}

		void setMaxID(T max)
		{
			FSN_ASSERT(this->m_NextId == this->m_FirstId);
			FSN_ASSERT(this->m_UnusedIds.empty());
			m_MaxId = max;
		}

		T peekNextID() const
		{
			if (this->m_UnusedIds.empty())
				return this->m_NextId+1;
			else
				return this->m_UnusedIds.back();
		}

		//! Returns an ObjectID which is not in use
		virtual T getFreeID()
		{
			if (this->m_NextId > m_MaxId)
				FSN_EXCEPT(NoMoreIdsException, "No more IDs are available");

			if (this->m_UnusedIds.empty())
				return this->m_NextId++;
			else
			{
				ObjectID id = this->m_UnusedIds.back();
				this->m_UnusedIds.pop_back();
				return id;
			}
		}
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id)
		{
			if (id == this->m_NextId-1)
				--(this->m_NextId);
			else if (id < this->m_NextId-1)
			{
				FSN_ASSERT_MSG(std::find(this->m_UnusedIds.begin(), this->m_UnusedIds.end(), id) != this->m_UnusedIds.end(),
					"Redundant call to freeID: the given ID is already free");
				this->m_UnusedIds.push_back(id); // record unused ID
			}
		}

	protected:
		T m_MaxId;
	};

	//! Supplies IDs which aren't assigned, starting with the lowest ID available
	/*!
	* O(n) (when the ID is in the set, rather than the next id).
	* Constant time when the id is the next id.
	*/
	template <typename T>
	class IDBitset : public IDCollection<T, boost::dynamic_bitset<>>
	{
		typedef boost::dynamic_bitset<> Bitset_t;

		typedef IDCollection<T, boost::dynamic_bitset<>> base_type;
	public:
		//! Default
		IDBitset()
			: base_type()
		{}
		//! Initialises m_NextId to the given value
		IDBitset(T first_id)
			: base_type(first_id)
		{}
		//! Virtual destructor
		virtual ~IDBitset()
		{}
		//! Returns the next ID that will be returned by getFreeID
		T peekNextID() const
		{
			if (this->m_UnusedIds.none())
					return this->m_NextId;
			else
				return this->m_UnusedIds.find_first();
		}
		//! Returns an ID which is not in use
		virtual T getFreeID()
		{
			if (this->m_UnusedIds.none())
					return this->m_NextId++;
			else
			{
				ObjectID id = this->m_UnusedIds.find_first();
				this->m_UnusedIds.reset(id);
				//if (this->m_UnusedIds.find_next(id) == Bitset_t::npos)
				//	this->m_UnusedIds.resize(id - 1);

				// It's possible for an ID to get freed when it is more than 1 below m_NextId
				//  then "freed" again with another call to freeID(id) when it is one below m_NextId
				//  (see the logic in that fn) -> this ensures that m_NextId will be incremented
				//  if that was the case for the ID just retreived from the set<> (so that the
				//  next call to this method after the set<> is empty wont return the same ID again)
				if (id == this->m_NextId)
					++this->m_NextId;

				return id;
			}
		}
		//! Allows the given ID which was previously returned by getFreeID to be returned again
		virtual void freeID(T id)
		{
			if (id == this->m_NextId-1)
			{
				this->m_UnusedIds.resize(id - 1);
				this->m_NextId = id;
			}
			else if (id < this->m_NextId-1)
			{
				if (id >= this->m_UnusedIds.size())
					this->m_UnusedIds.resize(id + 1);
				// record unused ID
				this->m_UnusedIds.set(id);
			}
		}
		//! Removes the given ID from the set
		/*!
		* \return Returns true if the ID was unused (so it could be taken), false otherwise
		*/
		virtual bool takeID(T id)
		{
			if (id == this->m_NextId)
			{
				++this->m_NextId;
				return true;
			}
			else if (id < this->m_NextId)
			{
				if (id < this->m_UnusedIds.size())
				{
					auto bitRef = this->m_UnusedIds[id];
					if (bitRef)
					{
						bitRef = false;
						return true;
					}
					else
						return false;
				}
				else
				{
					this->m_UnusedIds.resize(id + 1, false);
					return true;
				}
			}
			else // the id being taken is after the current index: jump forward to it
			{
				this->m_UnusedIds.resize(id, true);
				this->m_NextId = id + 1;
				return true;
			}
		}

		size_t numUsed() const
		{
			return this->m_NextId - this->m_UnusedIds.size();
		}

		size_t numNotUsed() const
		{
			return std::numeric_limits<T>::max() - numUsed();
		}

		bool hasMore() const
		{
			return numUsed() < std::numeric_limits<T>::max();
		}
	};

	//! Supplies ObjectIDs which aren't assigned
	class ObjectIDSet : public IDBitset<ObjectID>
	{
	public:
		//! Initialises the first ID to 1 (Entity IDs start at 1)
		ObjectIDSet()
			: IDBitset(1)
		{}
		virtual ~ObjectIDSet()
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
		virtual ~ObjectIDStack()
		{}
	};

}

#endif
