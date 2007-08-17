
#include "FusionException.h"

#include "FusionResourceManager.h"

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

	std::string Exception::grabReliableString(const std::string &resource, int args)
	{
		// Try to get the string requested
		ResourceManager* resMan = ResourceManager::getSingletonPtr();
		if (resMan != 0)
		{
			ResourcePointer<std::string> resource = GetResource<std::string>(resource);
			if (resource.IsValid())
			{
				return (*resource.GetDataPtr());
			}
		}

		// Make a string to fulfil the argument requirement
		std::string arguments_string = " - ";
		for (int a = 0; a < args; a++)
		{
			char number[10];
			std::string num_string = "%";

			memset(number, 0, 10);
			snprintf(number, 10, "%d", a);

			arguments_string += "%" + number + " ";
		}

		// Fall back on a generic message
		std::string::size_type lastSlash = resource.find_last_of("/");
		if (lastSlash != std::string::npos)
			return resource.substr(lastSlash+1) + arguments_string;

		else if (!resource.empty())
			return resource + arguments_string;

		else
			return "I think an error occored, but I have no idea what O_o" + arguments_string;
	}

}