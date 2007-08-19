/*
  Copyright (c) 2007 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward

*/

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

	Exception::Exception(const std::string &origin, bool critical)
		: m_Origin(origin),
		m_Critical(critical)
	{
	}

	Exception::Exception(const std::string& origin, const std::string &message, bool critical)
		: m_Origin(origin),
		m_Message(message),
		m_Critical(critical)
	{
	}

	Exception::ExceptionType Exception::GetType() const
	{
		return m_Type;
	}

	std::string Exception::GetName() const
	{
		static std::string strName("FusionEngine::Exception");
		return strName;
	}

	void Exception::SetOrigin(const std::string &origin)
	{
		m_Origin = origin;
	}

	const std::string& Exception::GetOrigin() const
	{
		return m_Origin;
	}

	std::string Exception::GetDescription() const
	{
		return m_Message;
	}

	bool Exception::IsCritical() const
	{
		return m_Critical;
	}

	std::string Exception::ToString() const
	{
		std::string str = GetName() + " in " + GetOrigin();
		std::string description = GetDescription();

		if (description.empty())
			return str;
		else
			return str + ": " + GetDescription();
	}

}