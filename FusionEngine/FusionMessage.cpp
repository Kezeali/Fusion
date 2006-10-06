
#include "FusionMessage.h"

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

FusionMessage::FusionMessage(unsigned char channel, unsigned char type, PlayerInd playerInd, unsigned char *message)
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

Packet *GetPacket()
{
	Packet *p = 

const PlayerInd FusionMessage::GetPlayerInd() const
{
	return m_PlayerInd;
}

const unsigned char FusionMessage::GetChannel() const
{
	return m_Channel;
}

const unsigned char FusionMessage::GetType() const
{
	return m_Type;
}
