
#include "FusionNetworkUtils.h"

/// RakNet
#include <RakNet/PacketEnumerations.h>

namespace FusionEngine
{

	bool NetUtils::IsFusionPacket(Packet *p)
	{
		cl_assert(p);
		return IsFusionPacket(p->data);
	}

	bool NetUtils::IsFusionPacket(unsigned char *data)
	{
		if (data==0)
			return 255;

		// All fusion packed IDs are greater than / = ID_USER_PACKET_ENUM
		return ( data[0] >= unsigned char(ID_USER_PACKET_ENUM) );
	}

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
			// This cl_assert is kinda useless, blame the RakNet guy (his code :P)
			//  If you wan't to make sure the packet is valid, it shouldn't just happen in
			//  debug releases!
			//cl_assert(length > sizeof(unsigned char) + sizeof(unsigned char) + sizeof(RakNetTime));

			if (length > sizeof(unsigned char) + sizeof(RakNetTime))
				// Get data at [ID_TIMESTAMP + time]
				return (unsigned char) data[sizeof(unsigned char) + sizeof(RakNetTime)];
		}
		else
			return (unsigned char) data[0];

		return 255;
	}

	unsigned char NetUtils::GetPacketChannel(Packet *p)
	{
		return GetPacketChannel(p->data, p->length);
	}

	unsigned char NetUtils::GetPacketChannel(unsigned char *data, unsigned int length)
	{
		if (data==0)
			return 255;

		if ((unsigned char)data[0] == ID_TIMESTAMP)
		{
			// This cl_assert is kinda useless, blame the RakNet guy (his code :P)
			//  If you wan't to make sure the packet is valid, it shouldn't just happen in
			//  debug releases!
			//cl_assert(length > sizeof(unsigned char) + sizeof(unsigned char) + sizeof(RakNetTime));

			if (length > sizeof(unsigned char) + sizeof(unsigned char) + sizeof(RakNetTime))
				// Get data at [ID_TIMESTAMP + time + MTID_]
				return (unsigned char) data[sizeof(unsigned char) + sizeof(unsigned char) + sizeof(RakNetTime)];
		}
		else
			// Get data at [MTID_]
			return (unsigned char) data[sizeof(unsigned char)];

		return 255;
	}

	RakNetTime NetUtils::GetPacketTime(Packet *p)
	{
		cl_assert(p);
		return GetPacketTime(p->data, p->length);
	}

	RakNetTime NetUtils::GetPacketTime(unsigned char *data, unsigned int length)
	{
		if ((unsigned char)data[0] == ID_TIMESTAMP)
		{
			//cl_assert(length >= sizeof(unsigned char) + sizeof(RakNetTime));
			// Make sure there actually is a timestamp here (even if that's all there is)
			if (length >= sizeof(unsigned char) + sizeof(RakNetTime))
			{

				RakNetTime time = 0;

				RakNet::BitStream timeBS(data+1, sizeof(unsigned int), false);
				timeBS.Read(time);

				return time;
			}
		}

		// Nothing found
		return (RakNetTime)0;
	}

	int NetUtils::GetHeaderLength(Packet *p)
	{
		cl_assert(p);
		return GetHeaderLength(p->data);
	}

	int NetUtils::GetHeaderLength(unsigned char *data)
	{
		if ((unsigned char)data[0] == ID_TIMESTAMP)
		{
			// Fusion packets have two IDs (Type and Channel)
			if (NetUtils::IsFusionPacket(data))
				return (int)( sizeof(unsigned char) + sizeof(unsigned char) + sizeof(RakNetTime) );
			else
				return (int)( sizeof(unsigned char) + sizeof(RakNetTime) );
		}
		else
			return (int) sizeof(unsigned char);
	}

}
