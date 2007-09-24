/*
  Copyright (c) 2007 Fusion Project Team

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
#include "FusionNetwork.h"

namespace FusionEngine
{

	Network::Network()
	{
	}

	Network::~Network()
	{
	}

	bool Network::SendToChannel(bool timestamped, char type, char subtype, char *data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		// Add the given subType to the beginning of the data string
		char* subTypedData = new char[length+1];
		subTypedData[0] = subtype;
		memcpy(subTypedData+1, data, length);

		bool success = Send(timestamped, type, subTypedData, length+1, priority, reliability, channel, destination);
		delete[] subTypedData;
		return success;
	}

	bool Network::SendRaw(char *data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		return Send(false, 0, data, length, priority, reliability, channel, destination);
	}

	void Network::PushBackPacket(IPacket *packet, bool toHead)
	{
		//! \todo Implement a default Network#PushBackPacket() (other than just deleting the packet!)
		DeallocatePacket( packet );
	}

	void Network::DeallocatePacket(IPacket *packet)
	{
		delete packet;
	}

	int Network::GetPing(const NetHandle& handle)
	{
		return 0;
	}

	//void Network::SendRaw(char *data, int len)
	//{
	//	send(data, len, MEDIUM_PRIORITY, RELIABLE, (char)CID_NONE);
	//}

	//void Network::SendShipState(const ShipState& state)
	//{
	//	unsigned char* buf;
	//	int length;

	//	length = state.Serialize(buf);

	//	length = addHeader(buf, length, true, MTID_SHIPFRAME, CID_GAME);

	//	send(buf, length, LOW_PRIORITY, UNRELIABLE_SEQUENCED, CID_GAME);
	//}

	//Packet *Network::PopNextMessage(char channel)
	//{
	//	return m_Queue->PopMessage(channel);
	//}

	//Event *Network::PopNextEvent() const
	//{
	//	Event *e = m_Events.front();
	//	m_Events.pop_front();
	//	return e;
	//}

	//void Network::ClearEvents()
	//{
	//	// Each Message
	//	EventQueue::iterator it = m_Events.begin();
	//	for (; it != m_Events.end(); ++it)
	//	{
	//		delete (*it);
	//	}

	//	m_Events.clear();
	//}

	//void FusionNetworkGeneric::_notifyNetEvent(unsigned char messageId)
	//{
	//m_Mutex->enter();

	////add event

	//m_Mutex->notify();
	//m_Mutex->leave();
	//}

	//EventQueue &FusionNetworkGeneric::GetEvents()
	//{
	//m_Mutex->enter();

	////return events

	//m_Mutex->notify();
	//m_Mutex->leave();
	//}

}
