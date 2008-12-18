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

	template <class T>
	class Node
	{
	public:
		typedef typename std::tr1::shared_ptr<Node<T>> nodeptr;

		Node()
		{
		}
		Node(const T& _value)
			: value(_value)
		{
		}
		~Node()
		{
			remove();
		}

		bool in_list()
		{
			return m_prev && m_next;
		}

		//! removes this node from the list
		void remove()
		{
			if (in_list())
			{
				m_next->m_prev = m_prev;
				m_prev->m_next = m_next;

				m_prev.reset();
				m_next.reset();
			}
		}

		//! Put other in this node's place
		void replace(nodeptr other)
		{
			if (!in_list())
				return;

			m_prev->m_next = other;
			m_next->m_prev = other;

			// remove this node from the list
			m_prev.reset();
			m_next.reset();
		}

		//! Insert other before this node
		void insert(nodeptr other)
		{
			insert_before(other);
		}

		//! Insert other before this node
		void insert_before(nodeptr other)
		{
			if (!in_list())
				return;

			// remove other from its current posision
			if (other->in_list())
			{
				other->m_next->m_prev = other->m_prev;
				other->m_prev->m_next = other->m_next;
			}

			other->m_prev = m_prev;
			other->m_next = nodeptr(this);

			m_prev->m_next = other;
			m_prev = other;
		}

		//! Insert other after this node
		void insert_after(nodeptr other)
		{
			if (!in_list())
				return;

			// remove other from its current posision
			if (other->in_list())
			{
				other->m_next->m_prev = other->m_prev;
				other->m_prev->m_next = other->m_next;
			}

			other->m_next = m_next;
			other->m_prev = nodeptr(this);

			m_next->m_prev = other;
			m_next = other;
		}

	private:
		nodeptr m_prev;
		nodeptr m_next;

	public:
		T value;
	};

	//! Probably better than HistoryBuffer in every way
	template <class T, class Alloc = std::allocator<T>>
	class HistroyList
	{
	public:
		typedef unsigned long time_type;

		typedef time_type key_type;
		typedef T value_type;

		typedef std::pair<key_type, value_type> pair_type;
		typedef Node<pair_type> node;
		typedef typename node::nodeptr nodeptr;

		typedef std::tr1::unordered_map<time_type, nodeptr> timemap_type;

		typedef T& reference;
		typedef const T& const_reference;

		/*!
       \struct const_traits
       \brief Defines the data types for a const iterator.
     */
		template <class Traits>
		struct const_traits {
			// Basic types
			typedef typename Traits::value_type value_type;
			typedef typename Traits::const_pointer pointer;
			typedef typename Traits::const_reference reference;
			typedef typename Traits::size_type size_type;
			typedef typename Traits::difference_type difference_type;

			// Non-const traits
			typedef nonconst_traits<Traits> nonconst_self;
		};

		/*!
		   \struct nonconst_traits
		   \brief Defines the data types for a non-const iterator.
		 */
		template <class Traits>
		struct nonconst_traits {
			// Basic types
			typedef typename Traits::value_type value_type;
			typedef typename Traits::pointer pointer;
			typedef typename Traits::reference reference;
			typedef typename Traits::size_type size_type;
			typedef typename Traits::difference_type difference_type;

			// Non-const traits
			typedef nonconst_traits<Traits> nonconst_self;
		};

		template <class Container, class Traits>
		class Iterator
			: public std::iterator<
			std::bidirectional_iterator_tag,
			typename Traits::value_type,
			typename Traits::difference_type,
			typename Traits::pointer,
			typename Traits::reference>
		{
			// Helper types
			typedef std::iterator<
				std::bidirectional_iterator_tag,
				typename Traits::value_type,
				typename Traits::difference_type,
				typename Traits::pointer,
				typename Traits::reference> base_iterator;

			typedef Iterator<Container, typename Traits::nonconst_self> nonconst_self;

			// Basic types
			typedef typename base_iterator::value_type value_type;
			typedef typename base_iterator::pointer pointer;
			typedef typename base_iterator::reference reference;
			typedef typename Traits::size_type size_type;
			typedef typename base_iterator::difference_type difference_type;

			typedef typename std::tr1::weak_ptr<Node<elem_type>> nodeweakptr;

			typedef nodeweakptr pointer;

			Iterator(container_type *b, nodeweakptr start_pos)
				: m_buf(b), m_targetnode(p)
			{
			}

			reference operator*()
			{
				return (*m_pointer);
			}
			pointer operator->()
			{
				return &(operator*());
			}

		private:
			container_type *m_container;
			nodeweakptr m_pointer;
		};

		typedef Iterator<T> iterator;
		typedef Iterator<T, Alloc> const_iterator;

		Alloc m_allocator;

		nodeptr m_back;
		nodeptr m_front;

		timemap_type m_indexer;

		HistoryList()
		{
			m_back = m_front = nodeptr(m_allocator.allocate(1));
		}

		bool empty()
		{
			return m_indexer.size() == 0;
		}

		reference operator[](time_type _Keyval)
		{
			nodeptr what = m_indexer[_Keyval];
			if (!what->in_list()) // a new index was added
				m_back->insert_after(what);

			return what->value;
		}

		typedef
			circular_buffer_iterator<self_type>
			iterator;
		iterator begin()
		{
			return iterator(this, 0);
		}

		iterator end()
		{
			return iterator(this, size());
		}

		iterator find(time_type _time)
		{
			if (empty())
				return end();

			timemap_type::iterator _where = m_indexer.find(_time);
			if (_where != m_indexer.end())
				return iterator(*_where);

			iterator _where = m_Data.begin() + index;
			if (_where->time != _time)
			{
				return m_Data.end();
			}

			return _where;
		}
	};

	//! A simple implementation using boost::circular_buffer
	template<class T>
	class HistoryBuffer
	{
	public:
		typedef Record<T> record_type;
		typedef typename record_type::time_type time_type;

		typedef boost::circular_buffer<record_type> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;

		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		typedef std::tr1::unordered_map<time_type, size_type> timetoindex_type;

		HistoryBuffer()
			: m_Front(0)
		{
		}

		HistoryBuffer(size_type _size)
			: m_Front(0),
			m_Data(_size)
		{
		}

		void set_capacity(size_type _size)
		{
			m_Data.set_capacity(_size);
			m_TimeToIndex.rehash((timetoindex_type::size_type)(_size * 1.5));
		}

		record_type& operator[](size_type _Keyval)
		{
			return m_Data[_Keyval];
		}

		iterator find(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type index = m_TimeToIndex[_time];
			if (index >= m_Data.size())
				return m_Data.end();

			iterator _where = m_Data.begin() + index;
			if (_where->time != _time)
			{
				// erase outdated time mapping
				m_TimeToIndex.erase(_where);
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

			if (_where->time == _time)
				return _where;

			size_type low = 0;
			size_type high = m_Data.size() - 1;
			size_type mid;
			while (low <= high)
			{
				mid = (low + high) / 2;
				_where = m_Data.begin() + mid;
				if (_where->time > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (_where->time < _time)
					low = mid + 1;
				else
					return _where; // found
			}
			return _where; // not found (return closest)
		}

		size_type index_of(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.size();

			size_type index = m_TimeToIndex.at(_time);
			if (index >= m_Data.size())
				return m_Data.size();

			// Outdated record (bug?)
			if (m_Data[index].time != _time)
				return m_Data.size();

			return index;
		}

		size_type index_of_closest(time_type _time)
		{
			if (m_Data.empty())
				return m_Data.end();

			size_type test = m_Data.size();
			if (m_Data.size() == 1)
				return m_Data.begin();

			// Try to find the exact record
			size_type index = index_of(_time);
			if (index < m_Data.size())
				return m_Data[index];

			iterator _where = find_closest(_time);
			if (_where == end())
				return size();

			return index_of(_where->time);
		}

		//! Smartly chooses to set(), push() or insert() to get best performance
		void add(time_type _time, const T& _value)
		{
			iterator _where = find_closest(_time);

			if (_where->time == _time)

		}

		//! \remarks Runs in constant time
		void set(time_type _time, const T& _value)
		{
			set(m_TimeToIndex[_time], _time, _value);
		}

		void set(size_type _index, time_type _time, const T& _value)
		{
			m_Data[_index] = record_type(_time, _value);
		}

		//! \remarks Runs in linear time (proportional to size())
		void insert(time_type _time, const T& _value)
		{
			iterator _where = find_closest(_time);
			if (_where != m_Data.begin() && _where->time > _time)
				--_where;
			else if (_where != m_Data.end() && _where->time < _time)

				m_Data.insert(_where, record_type(_time, _value));
		}

		void insert(iterator _where, time_type _time, const T& _value)
		{
			m_Data.insert(_where, record_type(_time, _value));
		}

		void push(time_type _time, const T& _value)
		{
			// Update the indexer
			m_TimeToIndex[_time] = m_Front;
			if (m_Data.full)
				m_TimeToIndex.erase(m_Data.back().first);

			m_Data.push_front(record_type(_time, _value));

			// This figures out where the next record will go in the circular buffer
			m_Front = fe_wrap(m_Front + 1, (size_type)0, m_Data.capacity());
		}

		// Pops off records until 'back' is at the given time
		void pop(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (back().time < _time)
			{
				if (m_Data.size() == 1) break;

				m_TimeToIndex.erase(back().time);

				m_Data.pop_back();
			}


			fe_clamp(m_Front, (size_type)0, m_Data.size());

		}

		void pop()
		{
			m_Data.pop_back();


			fe_clamp(m_Front, (size_type)0, m_Data.size());

		}

		void pop_front(time_type _time)
		{
			if (m_Data.empty())
				return;

			while (front().time > _time)
			{
				if (m_Data.size() == 1) break;

				m_TimeToIndex.erase(front().time);

				m_Data.pop_front();
			}


			fe_clamp(m_Front, (size_type)0, m_Data.size());

		}

		void erase_before(iterator _back)
		{
			if (m_Data.empty() || _back == m_Data.begin())
				return;

			m_Data.rerase(_back - 1, m_Data.begin());


			fe_clamp(m_Front, (size_type)0, m_Data.size());

		}

		void erase_after(iterator _front)
		{
			if (m_Data.empty() || _front == m_Data.end())
				return;

			m_Data.erase(_front + 1, m_Data.end());


			fe_clamp(m_Front, (size_type)0, m_Data.size());

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

		size_type m_Front;
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
				if (midrecord->time > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->time < _time)
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
				if (midrecord->time > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->time < _time)
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
				if (midrecord->time > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->time < _time)
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
				if (midrecord->time > _time)
				{
					if (mid == 0) break;
					high = mid - 1;
				}
				else if (midrecord->time < _time)
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