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

#include "Common.h"

// Class
#include "FusionNetworkSystem.h"

// Fusion
#include "FusionLogger.h"
#include "FusionNetwork.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	DebugPacketHandler::DebugPacketHandler()
	{
		m_Log = Logger::getSingletonPtr()->GetLog("Network");
	}

	void DebugPacketHandler::HandlePacket(IPacket *packet)
	{
		m_Log->AddEntry("Unhandled packet arrived: " + boost::lexical_cast<std::string>(packet->GetType()), LOG_TRIVIAL);
	}

	const std::string s_NetSystemName = "Network";

	NetworkSystem::NetworkSystem()
		: m_PacketDispatcher(NULL),
		m_Network(NULL)
	{
	}

	NetworkSystem::NetworkSystem(Network *network)
		: m_PacketDispatcher(NULL),
		m_Network(network)
	{
	}

	NetworkSystem::~NetworkSystem()
	{
		CleanUp();
	}

	const std::string &NetworkSystem::GetName() const
	{
		return s_NetSystemName;
	}

	bool NetworkSystem::Initialise()
	{
		if (m_PacketDispatcher == NULL)
		{
			m_PacketDispatcher = new PacketDispatcher(m_Network);
			m_PacketDispatcher->SetDefaultPacketHandler(&m_DebugPacketHandler);
		}
		else
			m_PacketDispatcher->SetNetwork(m_Network);

		return true;
	}

	void NetworkSystem::CleanUp()
	{
		if (m_PacketDispatcher != NULL)
		{
			delete m_PacketDispatcher;
		}
	}

	void NetworkSystem::Update(float split)
	{
		m_PacketDispatcher->Run();
	}

	void NetworkSystem::Draw()
	{
	}

	void NetworkSystem::SetNetwork(Network *network)
	{
		m_Network = network;
		if (m_PacketDispatcher != NULL)
		{
			m_PacketDispatcher->SetNetwork(network);
		}
	}

	Network *NetworkSystem::GetNetwork() const
	{
		return m_Network;
	}

	void NetworkSystem::AddPacketHandler(char type, PacketHandler *handler)
	{
		if (m_PacketDispatcher != NULL)
			m_PacketDispatcher->Subscribe(type, handler);
	}

	void NetworkSystem::RequestStepControl()
	{
		m_Network->Send(false, MTID_REQUESTSTEPCONTROL, (unsigned char*)NULL, 0, FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, "");
	}

}