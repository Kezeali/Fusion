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
*  File Author:
*    Elliot Hayward
*/

#ifndef H_FusionRouter
#define H_FusionRouter

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionMessage.h"

#include <tbb/concurrent_queue.h>
#include <EASTL/hash_map.h>

namespace FusionEngine { namespace Messaging
{

	class Router
	{
	public:
		Router(Address address);
		virtual ~Router();

		Address GetAddress() const { return m_Address; }

		void AddDownstream(Router* downstream);
		void RemoveDownstream(Router* downstream);

		void Process(float timelimit);

		void ReceiveMessage(Message message);

	private:
		virtual void ProcessMessage(Message message) { FSN_ASSERT_FAIL("A message was addressed to router '" + std::string(GetAddress().data(), GetAddress().length()) + "', which has no processor."); }

		Address m_Address;
		Router* m_Upstream;
		eastl::hash_map<Address, Router*> m_Downstream;
		tbb::concurrent_queue<Message> m_OutgoingMessages;
	};

} }

#endif
