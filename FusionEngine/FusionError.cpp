
#include "FusionError.h"

using namespace FusionEngine;

Error::Error()
: m_Type(NONE)
{
}

Error::Error(Error::ErrorType type, const std::string &message)
: m_Type(type),
m_Message(message)
{
}

Error::ErrorType Error::GetType() const
{
	return m_Type;
}

const std::string &Error::GetError() const
{
	return m_Message;
}
