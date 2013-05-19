/*
*  Copyright (c) 2013 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "Messaging/FusionRouter.h"

#include "FusionConsole.h"

#include <tbb/tick_count.h>

namespace FusionEngine { namespace Messaging
{

	Router::Router(Address address)
		: m_Address(address)
	{
	}

	Router::~Router()
	{
	}

	void Router::AddDownstream(Router* downstream)
	{
		m_Downstream[downstream->GetAddress()] = downstream;
	}

	void Router::RemoveDownstream(Router* downstream)
	{
		m_Downstream.erase(downstream->GetAddress());
	}

	void Router::Process(float timelimit)
	{
		auto startTime = tbb::tick_count::now();
		Message message;
		while ((tbb::tick_count::now() - startTime).seconds() < timelimit && m_OutgoingMessages.try_pop(message))
		{
			if (message.address == m_Address)
			{
				try
				{
					ProcessMessage(message);
				}
				catch (std::exception& ex)
				{
					SendToConsole(ex.what());
				}
			}
			else
			{
				auto entry = m_Downstream.find(m_Address);
				if (entry != m_Downstream.end())
				{
					entry->second->ReceiveMessage(message);
				}
				else if (m_Upstream)
				{
					m_Upstream->ReceiveMessage(message);
				}
			}
		}
	}

	void Router::ReceiveMessage(Message message)
	{
		m_OutgoingMessages.push(message);
	}

} }
