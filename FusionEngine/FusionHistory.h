/*
 Copyright (c) 2008 Fusion Project Team

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
*/

#ifndef Header_FusionEngine_History
#define Header_FusionEngine_History

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#define BOOST_CB_DISABLE_DEBUG // Allows overwritten CB iterators to remain valid
#include <boost/circular_buffer.hpp>

namespace FusionEngine
{

	template <class T>
	class Record
	{
	public:
		typedef unsigned long time_type;

		Record()
			: time(0)
		{
		}
		Record(time_type _time, const T& _value)
			: time(_time), value(_value)
		{
		}

		time_type time;
		T value;
	};

	//class MirroredValueManager
	//{
	//public:
	//	typedef typename MirroredValue<T> mirroredvalue_type;
	//	typedef typename mirroredvalue_type::value_ptr value_ptr;

	//	typedef std::tr1::unordered_map<Kt, T> indexer_type;

	//	//! Initialize a new mirrored value container manager
	//	MirroredValueManager()
	//	{
	//	}

	//	//! Copy data from the given containers
	//	MirroredValueManager(const indexer_type& indexer, const Cont2& cont2)
	//		: m_cont1(cont1), m_cont2(cont2)
	//	{
	//	}

	//	void Add(const Kt& key, const T& value)
	//	{
	//		value_ptr valuePtr(new T(value));
	//		mirroredvalue_type indexerMv(valuePtr, this, 1);
	//		mirroredvalue_type containerMv(valuePtr, this 2);

	//		m_indexer.insert(indexer_type::value_type(key, indexerMv));
	//		m_cont2.push_back(containerMv);
	//	}

	//	void Remove(mirroredvalue_type *mirroredValue)
	//	{
	//		if (mirroredValue->m_container == 2)
	//			m_cont1.erase(mirroredValue->m_iterator);
	//		else if (mirroredValue->m_container == 1)
	//			m_cont2.erase(mirroredValue->m_iterator);
	//	}

	//protected:
	//	indexer_type m_indexer;
	//	Cont2 m_cont2;
	//};

	//! Probably better than HistoryBuffer in every way
	template <class T, class Alloc = std::allocator<T>>
	class HistoryList
	{
	public:
		typedef unsigned long time_type;

		typedef std::pair<time_type, T> value_type;

		typedef std::list<value_type, Alloc> container_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;
		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		typedef std::tr1::unordered_map<time_type, iterator> indexer_type;
		typedef typename indexer_type::value_type indexer_value_type;

		typedef size_t size_type;

		typedef typename MirroredValue<T> mirroredvalue_type;
		typedef typename mirroredvalue_type::value_ptr value_ptr;

		template <class T, class Iter>
		class MirroredValue
		{
		public:
			typedef std::tr1::shared_ptr<T> value_ptr;

			MirroredValue()
				: //m_container(0),
			m_manager(NULL)
			{
			}

			MirroredValue(const value_ptr &_value, Iter it, const HistoryList const* manager)
				: m_value(_value),
				m_manager(manager),
				m_iterator(it)
			{
			}

			MirroredValue(const MirroredValue& other)
				: m_value(other.m_value),
				m_manager(other.m_manager),
				m_iterator(other.it)
			{
			}

			~MirroredValue()
			{
				if (m_manager != NULL)
					m_manager->Remove(m_iterator);
			}

			void set_iterator(Iter it)
			{
			}

			//! Indirect member access operator.
			T* operator->()
			{
				return m_value.get();
			}

			T const* operator->() const
			{
				return (const T*)m_value.get();
			}

			T& operator*()
			{
				return *(m_value.get());
			}

			const T& operator*() const
			{
				return *(m_value.get());
			}

			value_ptr m_value;

			MirroredValueManager* m_manager;
			Iter m_iterator;
		};


		indexer_type m_indexer;
		container_type m_data;

		size_type m_size;
		size_type m_capacity;

		HistoryList()
			: m_size(0),
			m_capacity(400)
		{
			//push_back(0, T());
		}

		HistoryList(size_type capacity)
			: m_size(0),
			m_capacity(capacity)
		{
			//push_back(0, T());
		}

		bool empty() const
		{
			return size() == 0;
		}

		size_type size() const
		{
			return m_size;
		}

		void set_capacity(size_type _capacity)
		{
			if (_capacity < m_size)
				m_data.resize(_capacity);
			m_indexer.rehash((size_type)(_capacity * 1.5));

			m_capacity = _capacity;
		}

		T& operator[](time_type _key)
		{
			iterator _where = m_indexer.find(_key);
			if (_where == m_indexer.end())
			{
				add(_key, T());
				return back();
			}

			return (**_where).second;
		}

		iterator begin()
		{
			return m_data.begin();
		}

		iterator end()
		{
			return m_data.end();
		}

		const_iterator begin() const
		{
			return m_data.begin();
		}

		const_iterator end() const
		{
			return m_data.end();
		}

		Alloc get_allocator() const
		{
			return m_data.get_allocator();
		}

		reference front()
		{
			return m_data.front();
		}

		const_reference front() const
		{
			return m_data.front();
		}

		reference back()
		{
			return m_data.back();
		}

		const_reference back() const
		{
			return m_data.back();
		}

		reference oldest()
		{
			return front();
		}

		reference newest()
		{
			return back();
		}

		void _make_value()
		{
			value_ptr valuePtr(new T(value));
			mirroredvalue_type mirroredValue(valuePtr, this);

			indexer_type::iterator _where1 = m_indexer.insert(indexer_type::value_type(key, mirroredValue));
			m_indexer[key].second.set_iterator(_where1);

			m_data.push_back(mirroredValue);
			iterator _where2 = m_data.end();
			m_data.back().set_iterator(_where2);
		}

		//! Pushes a record on to the back of the history list
		/*!
		 * \remarks
		 * Make sure the given record is the newest before pushing it back:
		 * push_back makes no ordering checks so items will remain out of order
		 * if they are added out of order.
		 */
		void push_back(time_type key, const T& value)
		{
			m_data.push_back(value_type(key, value));
			m_indexer.insert( indexer_value_type(key, --end()) );
			++m_size;

			clamp_to_capacity();
		}

		//! Pushes a record on to the front of the history list
		void push_front(time_type key, const T& value)
		{
			m_data.push_front(value_type(key, value));
			m_indexer.insert( indexer_value_type(key, --end()) );
			++m_size;

			clamp_to_capacity();
		}

		void insert(iterator _where, time_type time, const T& value)
		{
			m_data.insert(_where, value_type(time, value));

			clamp_to_capacity();
		}

		void replace(iterator _where, const T& value)
		{
			time_type time = _where->first;
			_where = m_data.erase(_where);
			m_data.insert(_where, value_type(time, value));
		}

		//! Adds a new record, maintaining the list order
		/*!
		 * List order is oldest -> most recent, from begin() to end().
		 *
		 * Adds records as follows:
		 * * If time falls after all current records: the new record will be added
		 * to the end of the list.
		 * * If time falls before all current records: the new record will be added
		 * to the begining of the list.
		 * * If time falls between the begining and end of the list, and no other
		 * records currently in the list have the same time: the new record will be
		 * inserted in the appropriate order.
		 * * If a record with the given time already exists in the list: the new
		 * record will overwrite the old one;
		 */
		void add(time_type time, const T& value)
		{
			iterator _where = find(time);
			if (_where != end())
			{
				replace(_where, value);
			}
			else if (empty() || time < front().first)
			{
				push_front(time, value);
			}
			else if (time > back().first)
			{
				push_back(time, value);
			}
			else
			{
				_where = find_closest(time);
				if (_where->first < time)
					++_where;
				insert(_where, time, value);
			}
		}

		void remove_before(time_type time)
		{
			iterator _where = begin();
			while (_where != end())
			{
				if (_where->first >= time)
					break;

				m_data.erase(_where);
				m_indexer.erase(_where->first);
			}
		}

		void clamp_to_capacity()
		{
			if (m_size > m_capacity)
			{
				iterator rem_it = begin();
				time_type time = rem_it->first;
				m_data.erase(rem_it);
				m_indexer.erase(time);
				--m_size;
			}
		}

		iterator find(time_type _time)
		{
			if (empty())
				return end();

			indexer_type::iterator _where = m_indexer.find(_time);
			if(_where != m_indexer.end())
				return _where->second;
			else
				return end();
		}

		iterator find_closest(time_type _time)
		{
			if (empty())
				return end();

			iterator _where = find(_time);
			if (_where != end())
				return _where; // found exact match

			long front_delta = abs((long)(front().first - _time));
			long back_delta = abs((long)(back().first - _time));
			if (front_delta <= back_delta) // record is probably closer to the beginning - forward search
			{
				for (iterator it = begin(), end = this->end(); it != end; ++it)
				{
					if (it->first == _time)
						return it;
				}
			}
			else // record is probably closer to the end, so do a backward search
			{
				iterator it = end(), begin = this->begin();
				for (--it; it != begin; --it)
				{
					if (it->first == _time)
						return it;
				}
			}
			
			return end();
		}
	};

	//! A simple implementation using boost::circular_buffer
	template<class T>
	class HistoryBuffer
	{
	public:
		//typedef Record<T> record_type;
		//typedef typename record_type::time_type time_type;

		typedef unsigned long time_type;

		typedef std::pair<time_type, T> value_type;

		typedef boost::circular_buffer<value_type> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;

		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		typedef std::tr1::unordered_map<time_type, iterator> timetoindex_type;

		HistoryBuffer()
			//: m_Front(0)
		{
		}

		HistoryBuffer(size_type _size)
			: //m_Front(0),
			m_Data(_size)
		{
		}

		void set_capacity(size_type _size)
		{
			m_Data.set_capacity(_size);
			m_TimeToIndex.rehash((timetoindex_type::size_type)(_size * 1.5));
		}

		reference operator[](size_type _Keyval)
		{
			return m_Data[_Keyval];
		}

		iterator find(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			timetoindex_type::iterator _match = m_TimeToIndex.find(_time);
			if (_match == m_TimeToIndex.end())
				return m_Data.end();

			if (_match->second == m_Data.end())
			{
				// erase outdated time mapping
				m_TimeToIndex.erase(_match);
				return m_Data.end();
			}

			iterator _where = _match->second;
			if (_where->first != _time)
			{
				// erase outdated time mapping
				m_TimeToIndex.erase(_match);
				return m_Data.end();
			}

			return _where;
		}

		iterator find_closest(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			if (m_Data.size() == 1)
				return m_Data.begin();

			iterator _where = find(_time);

			if (_where != end() && _where->first == _time)
				return _where;

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid;
			while (low <= high)
			{
				mid = (low + high) / 2;
				_where = m_Data.begin() + mid;
				if (_where->first > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (_where->first < _time)
					low = mid + 1;
				else
					return _where; // found
			}
			return _where; // not found (return closest)
		}

		//size_type index_of(time_type _time)
		//{
		//	if (m_Data.empty())
		//		return m_Data.size();

		//	size_type index = m_TimeToIndex.find(_time);
		//	if (index >= m_Data.size())
		//		return m_Data.size();

		//	// Outdated record (bug?)
		//	if (m_Data[index].time != _time)
		//		return m_Data.size();

		//	return index;
		//}

		//size_type index_of_closest(time_type _time)
		//{
		//	if (m_Data.empty())
		//		return m_Data.end();

		//	size_type test = m_Data.size();
		//	if (m_Data.size() == 1)
		//		return m_Data.begin();

		//	// Try to find the exact record
		//	size_type index = index_of(_time);
		//	if (index < m_Data.size())
		//		return m_Data[index];

		//	iterator _where = find_closest(_time);
		//	if (_where == end())
		//		return size();

		//	return index_of(_where->time);
		//}

		//! Smartly chooses to set(), push() or insert() to get best performance
		void add(time_type _time, const T& _value)
		{
			iterator _where = find_closest(_time);

			if (_where->first == _time)
				set(_time, _value);
			else if (_where == end())
				push(_time, _value);
			else
				insert(_where, _time, _value);
		}

		//! \remarks Runs in constant time
		void set(time_type _time, const T& _value)
		{
			set(m_TimeToIndex[_time], _time, _value);
		}

		/*void set(size_type _index, time_type _time, const T& _value)
		{
			m_Data[_index] = record_type(_time, _value);
		}*/

		void set(iterator _where, time_type _time, const T& _value)
		{
			(*_where) = record_type(_time, _value);
		}

		//! \remarks Runs in linear time (proportional to size())
		void insert(time_type _time, const T& _value)
		{
			iterator _where = find_closest(_time);
			if (_where != m_Data.begin() && _where->first > _time)
				--_where;
			else if (_where != m_Data.end() && _where->first < _time)
				++_where;

			m_Data.insert(_where, record_type(_time, _value));
			m_TimeToIndex[_time] = _where;
		}

		void insert(iterator _where, time_type _time, const T& _value)
		{
			m_Data.insert(_where, record_type(_time, _value));
			m_TimeToIndex[_time] = _where;
		}

		void push(time_type _time, const T& _value)
		{
			m_Data.push_front(value_type(_time, _value));

			// Update the indexer
			m_TimeToIndex[_time] = m_Data.end() - 1;
			if (m_Data.full())
				m_TimeToIndex.erase(m_Data.back().first);

			// This figures out where the next record will go in the circular buffer
			//m_Front = fe_wrap(m_Front + 1, (size_type)0, m_Data.capacity());
		}

		// Pops off records until 'back' is at the given time
		void pop(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (back().first < _time)
			{
				if (m_Data.size() == 1) break;

				m_TimeToIndex.erase(back().first);

				m_Data.pop_back();
			}


			//fe_clamp(m_Front, (size_type)0, m_Data.size());
		}

		void pop()
		{
			m_Data.pop_back();
			m_TimeToIndex.erase(back().first);

			//fe_clamp(m_Front, (size_type)0, m_Data.size());
		}

		void pop_front(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (front().first > _time)
			{
				if (m_Data.size() == 1) break;

				m_TimeToIndex.erase(front().first);

				m_Data.pop_front();
			}


			//fe_clamp(m_Front, (size_type)0, m_Data.size());
		}

		void erase_before(iterator _back)
		{
			if (m_Data.empty() || _back == m_Data.begin())
				return;

			for (container_type::iterator it = _back - 1, stop = m_Data.begin(); it != stop; --it)
			{
				m_TimeToIndex.erase(it->first);
				it = m_Data.erase(it);
			}

			//fe_clamp(m_Front, (size_type)0, m_Data.size());
		}

		void erase_after(iterator _front)
		{
			if (m_Data.empty() || _front == m_Data.end())
				return;

			m_Data.erase(_front + 1, m_Data.end());
		}

		bool empty()
		{
			return m_Data.empty();
		}

		size_type size() const
		{
			return m_Data.size();
		}

		size_type capacity() const
		{
			return m_Data.capacity();
		}

		void clear()
		{
			m_Data.clear();
			m_TimeToIndex.clear();

		}

		reference back()
		{
			return oldest();
		}

		reference front()
		{
			return newest();
		}

		reference oldest()
		{
			return m_Data.back();
		}

		reference newest()
		{
			return m_Data.front();
		}

		const_reference oldest() const
		{
			return m_Data.back();
		}

		const_reference newest() const
		{
			return m_Data.front();
		}

		iterator begin()
		{
			return m_Data.begin();
		}

		const_iterator begin() const
		{
			return m_Data.begin();
		}

		iterator end()
		{
			return m_Data.end();
		}

		const_iterator end() const
		{
			return m_Data.end();
		}

		container_type m_Data;

		//size_type m_Front;
		timetoindex_type m_TimeToIndex;

	};

	template<class T>
	class History
	{
	public:
		typedef unsigned long time_type;

		typedef std::tr1::unordered_map<time_type, T> container_type;

		typedef typename container_type::size_type size_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;

		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		History()
		{
		}

		History(size_type _size)
		{
			m_Data = container_type(_size);
		}

		void set_capacity(size_type _size)
		{
			m_Data = container_type(_size);
		}

		T& operator[](time_type _Keyval)
		{
			return m_Data[_Keyval];
		}

		iterator find(time_type _time)
		{
			return m_Data.find(_time);
		}

		bool empty()
		{
			return m_Data.empty();
		}

		size_type size() const
		{
			return m_Data.size();
		}

		size_type capacity() const
		{
			return m_Data.capacity();
		}

		void clear()
		{
			m_Data.clear();
		}

		reference back()
		{
			return oldest();
		}

		reference front()
		{
			return newest();
		}

		reference oldest()
		{
			return m_Data.back();
		}

		reference newest()
		{
			return m_Data.front();
		}

		const_reference oldest() const
		{
			return m_Data.back();
		}

		const_reference newest() const
		{
			return m_Data.front();
		}

		iterator begin()
		{
			return m_Data.begin();
		}

		const_iterator begin() const
		{
			return m_Data.begin();
		}

		iterator end()
		{
			return m_Data.end();
		}

		const_iterator end() const
		{
			return m_Data.end();
		}

		container_type m_Data;
	};

	//! Circular buffer - great for high frequency updates
	template<class T>
	class Buffer
	{
	public:
		typedef unsigned long time_type;

		typedef std::pair<time_type, T> record_type;

		typedef boost::circular_buffer<record_type> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;

		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		Buffer()
		{
		}

		Buffer(size_type _size)
		{
			m_Data.set_capacity(_size);
		}

		void set_capacity(size_type _size)
		{
			m_Data.set_capacity(_size);
		}

		record_type& operator[](size_type _Keyval)
		{
			return m_Data[_Keyval];
		}

		iterator find(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type test = m_Data.size();
			if (m_Data.size() == 1)
				return m_Data.begin();

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid = (low + high) / 2;
			iterator midrecord;
			while (low <= high)
			{
				mid = (low + high) / 2;
				midrecord = m_Data.begin() + mid;
				if (midrecord->first > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->first < _time)
					low = mid + 1;
				else
					return midrecord; // found
			}
			return m_Data.end(); // not found
		}

		iterator find_closest(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type test = m_Data.size();
			if (m_Data.size() == 1)
				return m_Data.begin();

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid = (low + high) / 2;
			iterator midrecord;
			while (low <= high)
			{
				mid = (low + high) / 2;
				midrecord = m_Data.begin() + mid;
				if (midrecord->first > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->first < _time)
					low = mid + 1;
				else
					return midrecord; // found
			}
			return midrecord; // not found (return closest)
		}

		size_type index_of(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type test = m_Data.size();
			if (m_Data.size() == 1)
				return m_Data.begin();

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid = (low + high) / 2;
			iterator midrecord;
			while (low <= high)
			{
				mid = (low + high) / 2;
				midrecord = m_Data.begin() + mid;
				if (midrecord->first > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->first < _time)
					low = mid + 1;
				else
					return midrecord; // found
			}
			return m_Data.end(); // not found
		}

		size_type index_of_closest(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type test = m_Data.size();
			if (m_Data.size() == 1)
				return m_Data.begin();

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid = (low + high) / 2;
			iterator midrecord;
			while (low <= high)
			{
				mid = (low + high) / 2;
				midrecord = m_Data.begin() + mid;
				if (midrecord->first > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->first < _time)
					low = mid + 1;
				else
					return midrecord; // found
			}
			return midrecord; // not found (return closest)
		}

		iterator insert(iterator _where, time_type _time, const T& _value)
		{
			if (m_Data.full())
				m_Data.pop_back();
			return m_Data.insert(_where, record_type(time, _value));
		}

		void update(size_type _at, time_type _time, const T& _value)
		{
			_where = m_Data.erase(_where);
			m_Data.insert(_where, record_type(_time, _value));
		}


		void push(time_type _time, const T& _value)
		{
			m_Data.push_front(record_type(_time, _value));
		}

		// Pops off records until 'back' is at the given time
		void pop(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (back().time < _time)
			{
				if (m_Data.size() == 1) break;
				m_Data.pop_back();
			}
		}

		void pop()
		{
			m_Data.pop_back();
		}

		void pop_front(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (front().time > _time)
			{
				if (m_Data.size() == 1) break;
				m_Data.pop_front();
			}
		}

		void erase_before(iterator _back)
		{
			if (m_Data.empty() || _back == m_Data.begin())
				return;

			m_Data.rerase(_back - 1, m_Data.begin());
		}

		void erase_after(iterator _front)
		{
			if (m_Data.empty() || _front == m_Data.end())
				return;

			m_Data.erase(_front + 1, m_Data.end());
		}

		bool empty()
		{
			return m_Data.empty();
		}

		size_type size() const
		{
			return m_Data.size();
		}

		size_type capacity() const
		{
			return m_Data.capacity();
		}

		void clear()
		{
			m_Data.clear();
		}

		record_type& back()
		{
			return oldest();
		}

		record_type& front()
		{
			return newest();
		}

		reference oldest()
		{
			return m_Data.back();
		}

		reference newest()
		{
			return m_Data.front();
		}

		const_reference oldest() const
		{
			return m_Data.back();
		}

		const_reference newest() const
		{
			return m_Data.front();
		}

		iterator begin()
		{
			return m_Data.begin();
		}

		const_iterator begin() const
		{
			return m_Data.begin();
		}

		iterator end()
		{
			return m_Data.end();
		}

		const_iterator end() const
		{
			return m_Data.end();
		}

		container_type m_Data;
	};

}

#endif