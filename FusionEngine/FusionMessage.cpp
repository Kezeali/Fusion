
#include "FusionMessage.h"

using namespace FusionEngine;

FusionMessage::FusionMessage()
{
}

FusionMessage::FusionMessage(unsigned char type, unsigned int playerInd)
: m_Type(type),
m_PlayerInd(playerInd)
{
}

FusionMessage::FusionMessage(unsigned char type, unsigned int playerInd, unsigned char *message)
: m_Type(type),
m_PlayerInd(playerInd),
m_Message(message)
{
}

void FusionMessage::Write(unsigned char *message)
{
	m_Message = message;
}

const std::string &FusionMessage::Read()
{
	return m_Message;
}
