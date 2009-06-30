/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_LinkedNode
#define Header_FusionEngine_LinkedNode

#if _MSC_VER > 1000
#	pragma once
#endif

namespace FusionEngine
{

 /*!
	* \brief
	* Template class for creating doubly linked-list node classes.
	*/
	//template<typename T>
	class LinkedNode
	{
		//! The next node in the list
		LinkedNode* m_Next;
		//! The previous node in the list
		LinkedNode* m_Previous;

	public:
		//! Constructor
		LinkedNode( void )
		{
			m_Previous = NULL;
			m_Next = NULL;
		}

		//! Destructor
		/*!
		 * Removes this node from the list
		 */
		virtual ~LinkedNode( void )
		{
			this->remove();
		}

	public:
		//! Removes this node from the list
		void remove( void )
		{
			if (m_Next != NULL)
			{
				m_Next->m_Previous = m_Previous;
				m_Next = NULL;
			}
			if (m_Previous != NULL)
			{
				m_Previous->m_Next = m_Next;
				m_Previous = NULL;
			}
		}
		void removeAllAfter( void )
		{
			if (m_Next != NULL)
				m_Next->removeAllAfter();

			this->remove();
		}
		void removeAllBefore( void )
		{
			if (m_Previous != NULL)
				m_Previous->removeAllBefore();

			this->remove();
		}
		void removeAll( void )
		{
			removeAllBefore();
			removeAllAfter();
		}
		//! Returns the next node in the list
		LinkedNode* getNext( void ) const
		{
			return m_Next;
		}
		//! Returns the previous node in the list
		LinkedNode* getPrevious( void ) const
		{
			return m_Previous;
		}
		//! Adds a new node to the list after the current node
		void insert( LinkedNode* node )
		{
			// Init the node
			node->m_Previous = this;
			node->m_Next = m_Next; // It's OK if this->m_Next is NULL here

			// Insert the node
			if (m_Next != NULL)
				m_Next->m_Previous = node;
			m_Next = node;
		}
		//! Adds a node to the list before the current node
		void insertBefore( LinkedNode* node )
		{
			node->m_Next = this;
			node->m_Previous = this->m_Previous;

			if (m_Previous != NULL)
				m_Previous->m_Next = node;
			this->m_Previous = node;
		}
		//! Adds a new node to the end of the list
		void push_back( LinkedNode* node )
		{
			if (m_Next = NULL)
				insert(node);
			else
				m_Next->push_back(node);
		}
		//! Adds a new node to the begining of the list
		void push_front( LinkedNode* node )
		{
			if (m_Next = NULL)
				insert(node);
			else
				m_Next->push_back(node);
		}

	};

}

#endif