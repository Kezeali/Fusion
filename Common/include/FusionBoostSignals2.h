/*
  Copyright (c) 2009 Fusion Project Team

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

/*!
 * \file FusionBoostSignals2.h
 * Simple header for files using Boost.Signals2 - includes
 * boost headers and defines a namespace abbr. for
 * boost#signals2 (bsig2 = boost#signals2).
 */

#ifndef H_FusionEngine_BoostSignals2
#define H_FusionEngine_BoostSignals2

#if _MSC_VER > 1000
# pragma once
#endif

#include <boost/signals2/signal.hpp>
#include <boost/bind.hpp>

namespace FusionEngine
{

	namespace bsig2 = boost::signals2;

	//class ConnectionContainer
	//{
	//public:
	//	typedef std::vector<boost::signals2::connection> ConnectionArray;
	//	//typedef std::shared_ptr<boost::signals2::scoped_connection> ScopedConnectionPtr;
	//	//typedef std::vector<ScopedConnectionPtr> ConnectionArray;
	//	typedef ConnectionArray::size_type size_type;
	//public:
	//	virtual ~ConnectionContainer();

	//	void push_back(const boost::signals2::connection &connection);
	//	void erase(const boost::signals2::connection &connection);

	//	void clear();

	//	void clean();

	//	void reserve(size_type size);

	//protected:
	//	void disconnect(const boost::signals2::connection &connection);

	//	ConnectionArray m_Connections;
	//};

	//ConnectionContainer::~ConnectionContainer()
	//{
	//	std::for_each(m_Connections.begin(), m_Connections.end(), boost::bind(&ConnectionContainer::disconnect, this, _1));
	//}

	//void ConnectionContainer::push_back(const boost::signals2::connection &connection)
	//{
	//	m_Connections.push_back( connection );
	//	//m_Connections.push_back( ScopedConnectionPtr(new boost::signals2::scoped_connection( connection )) );
	//}

	//void ConnectionContainer::erase(const boost::signals2::connection &connection)
	//{
	//}

	//void ConnectionContainer::clear()
	//{
	//	std::for_each(m_Connections.begin(), m_Connections.end(), boost::bind(&ConnectionContainer::disconnect, this, _1));
	//	m_Connections.clear();
	//}

	//void ConnectionContainer::clean()
	//{
	//	for (ConnectionArray::iterator it = m_Connections.begin(), end = m_Connections.end(); it != end; ++it)
	//	{
	//		if (!it->connected())
	//			it = m_Connections.erase(it);
	//	}
	//}

	//void ConnectionContainer::reserve(ConnectionContainer::size_type size)
	//{
	//	m_Connections.reserve(size);
	//}

	//void ConnectionContainer::disconnect(const boost::signals2::connection &connection)
	//{
	//	connection.disconnect();
	//}

}

#endif
