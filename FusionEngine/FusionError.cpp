
#include "FusionError.h"

using namespace FusionEngine;

Error::Error()
: m_Type(NONE)
{
}

Error::Error(QuitEvent::ErrorType type, const std::string &message)
: m_Type(type),
m_Message(message)
{
}

Error::ErrorType Error::GetType() const
{
	return m_Type;
}

const std::string &Error::GetMessage()
{
	return m_Message;
}
