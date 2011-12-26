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

#include "FusionStableHeaders.h"

#include "FusionException.h"

#include "FusionStringFormatting.h"

namespace FusionEngine
{

	Exception::Exception()
		: m_Line(0)
	{
	}

	Exception::Exception(const std::string &message)
		: m_Message(message),
		m_Line(-1)
	{
	}

	Exception::Exception(const std::string& message, const std::string &origin)
		: m_Origin(origin),
		m_Message(message),
		m_Line(-1)
	{
	}

	Exception::Exception(const std::string& message, const std::string &origin, const char* file, long line)
		: m_Origin(origin),
		m_Message(message),
		m_File(file),
		m_Line(line)
	{
	}

	Exception::~Exception()
	{
	}

	const std::string& Exception::GetName() const
	{
		static const std::string strName(typeid(*this).name());
		return strName;
	}

	const std::string& Exception::GetOrigin() const
	{
		return m_Origin;
	}

	const std::string& Exception::GetDescription() const
	{
		return m_Message;
	}

	std::string Exception::ToString() const
	{
		std::string str = GetName();
		
		if (!GetOrigin().empty())
			str += " in " + GetOrigin();

		if (!m_File.empty() && m_Line >= 0)
			str += makestring() << "(" << m_File << ":" << m_Line << ")";

		if (!GetDescription().empty())
			str += ": " + GetDescription();

		return str;
	}

	const char *Exception::what() const
	{
		return GetDescription().c_str();
	}

}