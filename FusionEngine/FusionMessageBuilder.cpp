
#include "FusionMessageBuilder.h"

#include <boost/archive/text_iarchive.hpp>

using namespace FusionEngine;

FusionMessage *FusionMessageBuilder::BuildMessage(const ShipState &input, PlayerInd playerid)
{
	RakNet::BitStream out_stream;

	// serialise the input and push it into the stream via boost::archive
	{
		boost::archive::text_oarchive arc(out_stream);

		arc << input;
	}

	return (new FusionMessage(CID_GAME, MTID_SHIPFRAME, playerid, out_stream.GetData()));
}

FusionMessage *FusionMessageBuilder::BuildMessage(const FusionEngine::ProjectileState &input, PlayerInd playerid)
{
	RakNet::BitStream out_stream;

	// serialise the input and push it into the stream via boost::archive
	{
		boost::archive::text_oarchive arc(out_stream);

		arc << input;
	}

	return (new FusionMessage(CID_GAME, MTID_PROJECTILEFRAME, playerid, out_stream.GetData()));
}

FusionMessage *FusionMessageBuilder::BuildMessage(const Packet *packet, PlayerInd playerid)
{
	FusionMessage *m;
	unsigned char packetid = _getPacketIdentifier(packet);

	unsigned char *data = packet->data;

	// System messages
	if ((packetid & CID_SYSTEM) > 0)
	{
		// New player
		if ((packetid & MTID_NEWPLAYER) > 0)
		{
			m = new FusionMessage(CID_SYSTEM, MTID_NEWPLAYER, playerid, data);
		}
	}
	// File transfer messages
	else if ((packetid & CID_FILETRANSFER) > 0)
	{
		// Ship data
		if ((packetid & MTID_STARTTRANSFER) > 0)
		{
			m = new FusionMessage(CID_FILETRANSFER, MTID_STARTTRANSFER, playerid, data);
		}
	}
	// Gameplay messages
	else if ((packetid & CID_GAME) > 0)
	{
		// Ship data
		if ((packetid & MTID_SHIPFRAME) > 0)
		{
			m = new FusionMessage(CID_GAME, MTID_SHIPFRAME, playerid, data);
		}
		// Projectile data
		else if ((packetid & MTID_PROJECTILEFRAME) > 0)
		{
			m = new FusionMessage(CID_GAME, MTID_PROJECTILEFRAME, playerid, data);
		}
	}
	// Chat messages
	else if ((packetid & CID_GAME) > 0)
	{
		// To all players chat data
		if ((packetid & MTID_CHALL) > 0)
		{
			m = new FusionMessage(CID_GAME, MTID_CHALL, playerid, data);
		}
		// To team chat data
		else if ((packetid & MTID_CHTEAM) > 0)
		{
			m = new FusionMessage(CID_GAME, MTID_CHTEAM, playerid, data);
		}
		// To a specific player chat data
		else if ((packetid & MTID_CHONE) > 0)
		{
			m = new FusionMessage(CID_GAME, MTID_CHONE, playerid, data);
		}
	}

	return m;
}

FusionMessage *FusionMessageBuilder::BuildEventMessage(const Packet *packet, PlayerInd playerind)
{
	unsigned char type = _getPacketIdentifier(packet);
	FusionMessage* m = new FusionMessage(0, type, playerind, packet->data);
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
