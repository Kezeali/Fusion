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

// Inherited
#include "FusionState.h"
#include "FusionPacketHandler.h"

// Fusion
#include "FusionPacketDispatcher.h"
#include "FusionLog.h"


namespace FusionEngine
{

	class DebugPacketHandler : public PacketHandler
	{
	public:
		DebugPacketHandler();

		void HandlePacket(IPacket *packet);

	protected:
		LogPtr m_Log;
	};

	class NetworkSystem : public System
	{
	public:
		NetworkSystem();
		NetworkSystem(Network *network);
		virtual ~NetworkSystem();

		virtual const std::string &GetName() const;

		virtual bool Initialise();
		virtual void CleanUp();

		virtual void Update(float split);
		virtual void Draw();

		void SetNetwork(Network *network);
		Network *GetNetwork() const;

		void AddPacketHandler(char type, PacketHandler *handler);
		void RemovePacketHandler(char type, PacketHandler *handler);

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

		DebugPacketHandler m_DebugPacketHandler;
	};

}

#endif
