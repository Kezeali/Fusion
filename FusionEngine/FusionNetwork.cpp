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
		: m_SmoothedPing(0.f),
		m_SPTightness(0.1f)
	{
	}

	Network::~Network()
	{
	}

	bool Network::SendRaw(char *data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		return Send(false, 0, data, length, priority, reliability, channel, destination);
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

	bool Network::Send(bool timestamped, char type, unsigned char *data, unsigned int length, NetPriority priority, NetReliability reliability, char channel, const NetHandle& destination)
	{
		return Send(timestamped, type, (char*)data, length, priority, reliability, channel, destination);
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

	int Network::GetLastPing(const NetHandle& handle)
	{
		return 0;
	}

	int Network::GetAveragePing(const NetHandle& handle)
	{
		return 0;
	}

	int Network::GetLowestPing(const NetHandle& handle)
	{
		return 0;
	}

	float Network::GetSmoothedPing(const NetHandle& handle)
	{
		m_SmoothedPing = m_SmoothedPing + (GetLastPing(handle) - m_SmoothedPing) * m_SPTightness;
		return m_SmoothedPing;
	}

	void Network::SetSmoothingTightness(float tightness)
	{
		m_SPTightness = tightness;
	}

}
