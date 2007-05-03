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

#include "FusionConnectionStage.h"

namespace FusionEngine
{

	ConnectionStage::ConnectionStage(RakPeerInterface* peer, const char* address, unsigned short port, unsigned short localPort, unsigned short maxConnections, int threadSleep)
		: m_Connection(peer),
		m_Address(address),
		m_RemotePort(port),
		m_LocalPort(localPort),
		m_MaxConnections(maxConnections),
		m_ThreadSleep(threadSleep)
	{
	}

	ConnectionStage::~ConnectionStage()
	{
	}

	bool ConnectionStage::Initialise()
	{
		m_Stage = CS_STARTUP;
		// Can't continue if the peer given isn't a valid ptr
		return m_Connection != NULL;
	}

	float ConnectionStage::Update(unsigned int split)
	{
		switch (m_Stage)
		{
		case CS_STARTUP:
			SocketDescriptor socketDescriptor(m_LocalPort, 0);
			if (
				!m_Connection->Startup(m_MaxConnections, m_ThreadSleep, &socketDescriptor, 1 ) &&
				!m_Connection->Connect(m_Address, m_RemotePort, 0, 0)
				)
			{
				throw LoadingException(
					CL_String::format("Couldn't start the network connection. Please ensure that the address %1:%2 is valid.", m_Address, m_RemotePort)
					);
				break;
			}
			
			break;

		case CS_CONNECTING:
			if (m_Connection->IsActive())
			{
				m_Stage = CS_DONE;
				m_Done = true;
				break; // Optimising, or just stupid?
			}
			else
			{
				m_RunningTime += split;
				if (m_RunningTime > m_Timeout)
				{
					m_Failed = true;
					throw LoadingException("The connection timed out.");
				}
			}
			break;

		default:
			break;
		}

		return 1.0f/(float)m_Stage;
	}

	void ConnectionStage::CleanUp()
	{
	}

}
