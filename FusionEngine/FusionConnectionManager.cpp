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

#include "FusionConnectionManager.h"

namespace FusionEngine
{

	const RakClientInterface* ConnectionManager::GetClient(const std::string& host, unsigned short hostPort, unsigned short localPort)
	{
		std::string tag = CL_String::format("%1:%2", host, hostPort);

		// Try to find an existing connection
		ClientConnectionsList::iterator i = m_ClientConnections.find(tag);
		if (i != m_ClientConnections.end())
			return (*i);

		// Create a new connection
		RakClientInterface* peer = RakNetworkFactory::GetRakClientInterface();
		peer->Connect(host.c_str(), hostPort, localPort, 0, 0);

		// Required for timestamps (it should be on by default anyway)
		peer->StartOccasionalPing();

		m_ClientConnections.insert(
			ClientConnectionsList::value_type(tag, peer)
			);

	}

}