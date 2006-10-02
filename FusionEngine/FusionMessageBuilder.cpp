
#include "FusionMessageBuilder.h"

#include <boost/archive/text_iarchive.hpp>

using namespace FusionEngine;

FusionMessage *FusionMessageBuilder::BuildMessage(const ShipState &input)
{
	unsigned char *data[];

	RakNet::BitStream out_stream;

	{
		boost::archive::text_oarchive arc(out_stream);

		arc << input;
	}
}

FusionMessage *FusionMessageBuilder::BuildMessage(const FusionEngine::ProjectileState &input)
{
}

FusionMessage *FusionMessageBuilder::BuildMessage(const Packet &packet)
{
	unsigned char packetId = _getPacketIdentifier(p);
	switch (packetId)
	{
	case MTID_SHIPFRAME:
		break;
	}
}

unsigned char FusionMessageBuilder::_getPacketIdentifier(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p->data[0];
}
/*
unsigned char FusionMessageBuilder::_getPacketChannel(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p->data[0];
}
*/
