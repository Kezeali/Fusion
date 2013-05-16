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

#ifndef H_FusionMessage
#define H_FusionMessage

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionAddress.h"

#include <boost/any.hpp>

namespace FusionEngine { namespace Messaging
{

	struct Message
	{
		Address address;
		char messageType;
		boost::any data;

		Message()
			: messageType(0)
		{}
		Message(Address address, char messageType, boost::any data)
			: address(address),
			messageType(messageType),
			data(data)
		{}
		Message(const Message& other)
			: address(other.address),
			messageType(other.messageType),
			data(other.data)
		{
		}
	};

} }

#endif
