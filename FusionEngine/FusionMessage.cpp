
#include "FusionMessage.h"

using namespace FusionEngine;

FusionMessage::FusionMessage()
{
}

FusionMessage::FusionMessage(int type)
: m_Type(type)
{
}

FusionMessage::FusionMessage(int type, const std::string &message)
: m_Type(type),
m_Message(message)
{
}

void FusionMessage::Write(const std::string &message)
{
	m_Message = message;
}

const std::string &FusionMessage::Read()
{
	return m_Message;
}
