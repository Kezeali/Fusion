
#include "FusionNetworkUtils.h"

/// RakNet
#include "../RakNet/PacketEnumerations.h"

using namespace FusionEngine;

unsigned char NetUtils::GetPacketIdentifier(Packet *p)
{
	return GetPacketIdentifier(p->data, p->length);
}

unsigned char NetUtils::GetPacketIdentifier(unsigned char *data, unsigned int length)
{
	if (data==0)
		return 255;

	if ((unsigned char)data[0] == ID_TIMESTAMP)
	{
		assert(length > sizeof(unsigned char) + sizeof(RakNetTime));
		return (unsigned char) data[sizeof(unsigned char) + sizeof(RakNetTime)];
	}
	else
		return (unsigned char) data[0];
}

RakNetTime NetUtils::GetPacketTime(Packet *p)
{
	assert(p);

	return GetPacketTime(p->data, p->length);
}

RakNetTime NetUtils::GetPacketTime(unsigned char *data, unsigned int length)
{
	if ((unsigned char)data[0] == ID_TIMESTAMP)
	{
		// Make sure there actually is a timestamp here
		assert(length >= sizeof(unsigned char) + sizeof(RakNetTime));

		RakNetTime time = 0;

		RakNet::BitStream timeBS(data+1, sizeof(unsigned int), false);
		timeBS.Read(time);

		return time;
	}
	else
		return (RakNetTime)0;
}

int NetUtils::GetHeaderLength(Packet *p)
{
	assert(p);

	return GetHeaderLength(p->data);
}

int NetUtils::GetHeaderLength(unsigned char *data)
{
	if ((unsigned char)data[0] == ID_TIMESTAMP)
	{
		return (int) sizeof(unsigned char) + sizeof(RakNetTime);
	}
	else
		return (int) sizeof(unsigned char);
}
