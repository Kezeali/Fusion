
#include "FusionException.h"

namespace FusionEngine
{

	Exception::Exception()
		: m_Type(TRIVIAL),
		m_Critical(false)
	{
	}

	Exception::Exception(Exception::ExceptionType type, const std::string &message, bool critical)
		: m_Type(type),
		m_Message(message),
		m_Critical(critical)
	{
	}

	Exception::ExceptionType Exception::GetType() const
	{
		return m_Type;
	}

	const std::string &Exception::GetError() const
	{
		return m_Message;
	}

	bool Exception::IsCritical() const
	{
		return m_Critical;
	}

}