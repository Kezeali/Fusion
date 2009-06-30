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
#ifndef Header_FusionEngine_NetworkSystem
#define Header_FusionEngine_NetworkSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionSystem.h"

#include "FusionNetwork.h"
#include "FusionPacketDispatcher.h"

namespace FusionEngine
{

	class NetworkSystem : public System
	{
	public:
		NetworkSystem();
		virtual ~NetworkSystem();

		virtual const std::string &GetName() const;

		virtual bool Initialise();
		virtual void CleanUp();

		virtual bool Update(unsigned int split);
		virtual void Draw();

		virtual Network *GetNetwork() const;

		//! Allows a peer to take control of the update rate.
		/*
		* This allows step-through debugging without the game going out of sync.<br>
		* If multiple clients request step control, the server waits for all
		* clients to reach the current step before sending out the continue message
		* at each step.
		*/
		virtual void RequestStepControl();

	protected:
		PacketDispatcher *m_PacketDispatcher;
		Network *m_Network;
	};

	const std::string s_NetSystemName = "Network";

	NetworkSystem::NetworkSystem()
		: m_PacketDispatcher(NULL),
		m_Network(NULL)
	{
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
		}
		else
			m_PacketDispatcher->SetNetwork(m_Network);
	}

	void NetworkSystem::CleanUp()
	{
		if (m_PacketDispatcher != NULL)
		{
			delete m_PacketDispatcher;
		}
	}

	bool NetworkSystem::Update(unsigned int split)
	{
		m_PacketDispatcher->Run();
	}

	Network *NetworkSystem::GetNetwork() const
	{
		return m_Network;
	}

	void NetworkSystem::RequestStepControl()
	{
		m_Network->Send(false, MTID_REQUESTSTEPCONTROL, 
	}

}

#endif
