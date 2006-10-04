
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
}

FusionMessage::FusionMessage(unsigned char channel, unsigned char type, PlayerInd playerInd, unsigned char *message)
: m_Channel(channel),
m_Type(type),
m_PlayerInd(playerInd),
m_Message(message)
{
}

FusionMessage::~FusionMessage()
{
	delete m_Message;
}

void FusionMessage::Write(unsigned char *message)
{
	m_Message = message;
}

const std::string &FusionMessage::Read()
{
	return m_Message;
}

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
