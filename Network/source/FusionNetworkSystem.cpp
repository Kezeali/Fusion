/*
  Copyright (c) 2009-2010 Fusion Project Team

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

#include "PrecompiledHeaders.h"

#include "FusionNetworkSystem.h"

#include <boost/lexical_cast.hpp>

#include "FusionLogger.h"
#include "FusionNetworkManager.h"
#include "FusionRakNetwork.h"
#include "FusionPacketDispatcher.h"
#include "FusionPlayerRegistry.h"

namespace FusionEngine
{

	DebugPacketHandler::DebugPacketHandler()
	{
		m_Log = Logger::getSingleton().OpenLog("Network");
	}

	void DebugPacketHandler::HandlePacket(RakNet::Packet *packet)
	{
		if (packet->length > 0)
			m_Log->AddEntry("Unhandled packet arrived: " + boost::lexical_cast<std::string>(packet->data[0]), LOG_TRIVIAL);
	}

	const std::string s_NetSystemName = "Network";

	NetworkSystem::NetworkSystem()
		: m_PacketDispatcher(nullptr),
		m_Network(nullptr),
		m_NetworkManager(nullptr)
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
		if (m_Network == nullptr)
			m_Network = new RakNetwork();

		if (m_PacketDispatcher == nullptr)
		{
			m_PacketDispatcher = new PacketDispatcher();
			m_PacketDispatcher->SetDefaultPacketHandler(&m_DebugPacketHandler);
		}

		if (m_NetworkManager == nullptr)
			m_NetworkManager = new NetworkManager(m_Network, m_PacketDispatcher);

		return true;
	}

	template <class T>
	void checkedDelete(T*& ptr)
	{
		if (ptr != nullptr)
		{
			delete ptr;
			ptr = nullptr;
		}
	}

	void NetworkSystem::CleanUp()
	{
		checkedDelete(m_NetworkManager);
		checkedDelete(m_PacketDispatcher);
		checkedDelete(m_Network);
	}

	void NetworkSystem::Update(float split)
	{
		m_NetworkManager->DispatchPackets();
	}

	void NetworkSystem::Draw()
	{
	}

	bool NetworkSystem::IsConnected() const
	{
		return m_Network->IsConnected();
	}

}
