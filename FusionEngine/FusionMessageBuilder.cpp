
#include "FusionMessageBuilder.h"

/// Fusion
#include "FusionNetworkTypes.h"
#include "FusionNetworkUtils.h"


using namespace FusionEngine;

FusionMessage *MessageBuilder::BuildMessage(const ShipState &input)
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

	FusionMessage *m = new FusionMessage(
		CID_GAME,
		MTID_SHIPFRAME,
		playerid,
		out_stream.GetData(),
		(unsigned int)out_stream.GetNumberOfBytesUsed());
	return m;
}

FusionMessage *MessageBuilder::BuildMessage(const FusionEngine::ProjectileState &input, ObjectID playerid)
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

	FusionMessage *m = new FusionMessage(
		CID_GAME,
		MTID_PROJECTILEFRAME,
		playerid,
		out_stream.GetData(),
		(unsigned int)out_stream.GetNumberOfBytesUsed());
	return m;
}

FusionMessage *MessageBuilder::BuildMessage(const ShipInput &input, ObjectID playerid)
{
	RakNet::BitStream out_stream;

	// PlayerID (owner)
	out_stream.Write(input.pid);
	// Movement inputs
	out_stream.Write(input.thrust);
	out_stream.Write(input.reverse);
	out_stream.Write(input.left);
	out_stream.Write(input.right);
	// Triggers
	out_stream.Write(input.primary);
	out_stream.Write(input.secondary);
	out_stream.Write(input.bomb);

	FusionMessage *m = new FusionMessage(
		CID_GAME,
		MTID_PROJECTILEFRAME,
		playerid,
		out_stream.GetData(),
		(unsigned int)out_stream.GetNumberOfBytesUsed()
		);
	return m;
}

FusionMessage *MessageBuilder::BuildMessage(Packet *packet, ObjectID playerid)
{
	FusionMessage *m;
	unsigned char packetid = NetUtils::GetPacketIdentifier(packet);

	int head_length = NetUtils::GetHeaderLength(packet);
	int data_length = packet->length - head_length;

	unsigned char *data = new unsigned char[data_length];
	memcpy(data, packet->data+head_length, data_length);

	/// System messages
	// New player
	if (packetid == MTID_NEWPLAYER)
	{
		m = new FusionMessage(CID_SYSTEM, MTID_NEWPLAYER, playerid, data, packet->length);
	}

	/// File transfer messages
	// Ship data
	else if (packetid == MTID_STARTTRANSFER)
	{
		m = new FusionMessage(CID_FILESYS, MTID_STARTTRANSFER, playerid, data, packet->length);
	}

	/// Gameplay messages
	// Ship data
	else if (packetid == MTID_SHIPFRAME)
	{
		m = new FusionMessage(CID_GAME, MTID_SHIPFRAME, playerid, data, packet->length);
	}
	// Projectile data
	else if (packetid == MTID_PROJECTILEFRAME)
	{
		m = new FusionMessage(CID_GAME, MTID_PROJECTILEFRAME, playerid, data, packet->length);
	}

	/// Chat messages
	// To all players chat data
	else if (packetid == MTID_CHALL)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHALL, playerid, data, packet->length);
	}
	// To team chat data
	else if (packetid == MTID_CHTEAM)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHTEAM, playerid, data, packet->length);
	}
	// To a specific player chat data
	else if (packetid == MTID_CHONE)
	{
		m = new FusionMessage(CID_CHAT, MTID_CHONE, playerid, data, packet->length);
	}

	return m;
}

FusionMessage *MessageBuilder::BuildEventMessage(Packet *packet, ObjectID ObjectID)
{
	unsigned char type = NetUtils::GetPacketIdentifier(packet);

	FusionMessage *m = new FusionMessage(0, type, ObjectID, packet->data, packet->length);
	return m;
}
