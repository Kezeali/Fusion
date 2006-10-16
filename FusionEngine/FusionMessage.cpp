
#include "FusionMessage.h"

/// Fusion
#include "FusionNetworkUtils.h"

/// RakNet
#include "../RakNet/PacketEnumerations.h"

using namespace FusionEngine;

FusionMessage::FusionMessage()
{
}

FusionMessage::FusionMessage(unsigned char channel, unsigned char type, PlayerInd playerInd)
: m_Channel(channel),
m_Type(type),
m_PlayerInd(playerInd)
{
	// Get a timestamp
	m_Timestamp = RakNet::GetTime();
}

FusionMessage::FusionMessage(unsigned char channel, unsigned char type, PlayerInd playerInd, unsigned char *message, unsigned int length)
: m_Channel(channel),
m_Type(type),
m_PlayerInd(playerInd),
m_Message(message)
{
	// Get a timestamp
	m_Timestamp = RakNet::GetTime();
}

FusionMessage::~FusionMessage()
{
	delete m_Message;
}

void FusionMessage::Write(unsigned char *message)
{
	m_Message = message;
}

const unsigned char *FusionMessage::Read() const
{
	return m_Message;
}

RakNet::BitStream *FusionMessage::GetBitStream()
{
	RakNet::BitStream *bs = new RakNet::BitStream(m_Message, m_Length, true);
	return bs;
}

RakNet::BitStream *FusionMessage::GetTimedBitStream()
{
	RakNet::BitStream *bs = new RakNet::BitStream;
	bs->Write((unsigned char)ID_TIMESTAMP);

	// If this is an in-message (or has a timestamp for some other reason),
	//  this will get its timestamp, otherwise we will add a timestamp.
	RakNetTime time = NetUtils::GetPacketTime(m_Message, m_Length);

	if (time)
		bs->Write(time);
	else
		bs->Write(RakNet::GetTime());

	bs->Write(m_Message);

	return bs;
}

void FusionMessage::SetLength(unsigned int length)
{
	m_Length = length;
}

unsigned int FusionMessage::GetLength() const
{
	return m_Length;
}

const PlayerInd FusionMessage::GetPlayerInd() const
{
	return m_PlayerInd;
}

const unsigned char FusionMessage::GetType() const
{
	return m_Type;
}

const unsigned char FusionMessage::GetChannel() const
{
	return m_Channel;
}
