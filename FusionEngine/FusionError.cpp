
#include "FusionError.h"

namespace FusionEngine
{

	Error::Error()
		: m_Type(TRIVIAL),
		m_Critical(false)
	{
	}

	Error::Error(Error::ErrorType type, const std::string &message, bool critical)
		: m_Type(type),
		m_Message(message),
		m_Critical(critical)
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

	bool Error::IsCritical() const
	{
		return m_Critical;
	}

}