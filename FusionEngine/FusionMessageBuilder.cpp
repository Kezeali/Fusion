
#include "FusionMessageBuilder.h"

#include "FusionNetworkTypes.h"

/// RakNet
#include "../RakNet/PacketEnumerations.h"

//! \todo for some reason boost's serialization doesn't work with RakNet, so we might
//! as well remove the dependancy on boost altogether.

//#include <boost/archive/text_iarchive.hpp>

using namespace FusionEngine;

FusionMessage *FusionMessageBuilder::BuildMessage(const ShipState &input, PlayerInd playerid)
{	
	/*
	// serialise the input and push it into the stream via boost::archive
	{
		boost::archive::text_oarchive arc(out_stream);

		arc << input;
	}
	*/

	RakNet::BitStream out_stream;

	// PlayerID
	out_stream.Write(input.PID);
	// Pos
	out_stream.Write(input.Position.x);
	out_stream.Write(input.Position.y);
	// Vel
	out_stream.Write(input.Velocity.x);
	out_stream.Write(input.Velocity.y);
	// Rotation / RotVel
	out_stream.Write(input.Rotation);
	out_stream.Write(input.RotationalVelocity);
	// Active weapons
	out_stream.Write(input.current_primary);
	out_stream.Write(input.current_secondary);
	out_stream.Write(input.current_bomb);
	// Available components
	out_stream.Write(input.engines);
	out_stream.Write(input.weapons);

	return (new FusionMessage(CID_GAME, MTID_SHIPFRAME, playerid, out_stream.GetData()));
}

FusionMessage *FusionMessageBuilder::BuildMessage(const FusionEngine::ProjectileState &input, PlayerInd playerid)
{
	RakNet::BitStream out_stream;

	// PlayerID (owner)
	out_stream.Write(input.PID);
	// Unique Identifier
	out_stream.Write(input.OID);
	// Pos
	out_stream.Write(input.Position.x);
	out_stream.Write(input.Position.y);
	// Vel
	out_stream.Write(input.Velocity.x);
	out_stream.Write(input.Velocity.y);
	// Rotation / RotVel
	out_stream.Write(input.Rotation);
	out_stream.Write(input.RotationalVelocity);

	return (new FusionMessage(CID_GAME, MTID_PROJECTILEFRAME, playerid, out_stream.GetData()));
}

FusionMessage *FusionMessageBuilder::BuildMessage(Packet *packet, PlayerInd playerid)
{
	FusionMessage *m;
	unsigned char packetid = _getPacketIdentifier(packet);

	int head_length = _getHeaderLength(packet);
	int data_length = packet->length - head_length;

	unsigned char *data = new unsigned char[data_length];
	memcpy(data+head_length, packet->data, data_length);

	/// System messages
	// New player
	if (packetid == MTID_NEWPLAYER)
	{
		m = new FusionMessage(CID_SYSTEM, MTID_NEWPLAYER, playerid, data);
	}

	/// File transfer messages
	// Ship data
	else if (packetid == MTID_STARTTRANSFER)
	{
		m = new FusionMessage(CID_FILESYS, MTID_STARTTRANSFER, playerid, data);
	}

	/// Gameplay messages
	// Ship data
	else if (packetid == MTID_SHIPFRAME)
	{
		m = new FusionMessage(CID_GAME, MTID_SHIPFRAME, playerid, data);
	}
	// Projectile data
	else if (packetid == MTID_PROJECTILEFRAME)
	{
		m = new FusionMessage(CID_GAME, MTID_PROJECTILEFRAME, playerid, data);
	}

	/// Chat messages
	// To all players chat data
	else if (packetid == MTID_CHALL)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHALL, playerid, data);
	}
	// To team chat data
	else if (packetid == MTID_CHTEAM)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHTEAM, playerid, data);
	}
	// To a specific player chat data
	else if (packetid == MTID_CHONE)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHONE, playerid, data);
	}

	return m;
}

FusionMessage *FusionMessageBuilder::BuildEventMessage(Packet *packet, PlayerInd playerind)
{
	unsigned char type = _getPacketIdentifier(packet);
	return (new FusionMessage(0, type, playerind, packet->data));
}

unsigned char FusionMessageBuilder::_getPacketIdentifier(Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(RakNetTime));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(RakNetTime)];
	}
	else
		return (unsigned char) p->data[0];
}

RakNetTime FusionMessageBuilder::_getPacketTime(Packet *p)
{
	assert(p);

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		// Make sure there actually is a timestamp here
		assert(p->length >= sizeof(unsigned char) + sizeof(RakNetTime));

		RakNetTime time = 0;

		RakNet::BitStream timeBS(p->data+1, sizeof(unsigned int), false);
		timeBS.Read(time);

		return time;
	}
	else
		return (RakNetTime)0;
}

int FusionMessageBuilder::_getHeaderLength(Packet *p)
{
	assert(p);

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		return (int) sizeof(unsigned char) + sizeof(RakNetTime);
	}
	else
		return (int) sizeof(unsigned char);
}
