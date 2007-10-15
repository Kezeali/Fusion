/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_Console
#define Header_FusionEngine_Console

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

/// Fusion
#include "FusionException.h"

namespace FusionEngine
{
	static const size_t g_ConsoleDefaultMaxData = 1000;

	//! Not Used
	struct ConsoleLine
	{
		std::string message;
		CL_Color colour;
	};

	//! Provides console data access to all FusionEngine objects
	/*!
	 * This class does not actually do anything with the data it contains,
	 * but instead uses callbacks to allow other classes to deal with console
	 * data. On that note, this is also not a script executor; the console
	 * GUI does that.
	 *
	 * \todo Store ConsoleLines as circular 
	 * linked list (or something else that doesn't require data manipulation
	 * to remove items from the beginning - could it be done with a vector and
	 * stored positions?)
	 */
	class Console : public Singleton<Console>
	{
	public:
		//! Basic constructor
		Console();

	public:
		//! Lines in the console
		typedef std::deque<std::string> ConsoleLines;

		//! Message header types
		enum MessageType {
			MTNORMAL = 0,
			MTERROR,
			MTWARNING
		};

	public:
		//! Returns the exception marker
		const std::string& GetExceptionMarker() const;
		//! Returns the warning marker
		const std::string& GetWarningMarker() const;

		//! Adds the given message to the console history
		void Add(const std::string &message);

		//! Adds the given message, after prepending it with a heading of the specified type, to the console history
		void Add(const std::string &message, MessageType type);

		//! Adds the given Error to the console history
		void Add(const Exception *error);

		//! Removes all data from the console
		void Clear();

		//! Returns all the lines that have been added to the console
		const ConsoleLines& GetHistory() const;

		//! Triggers when new data is added to the console
		CL_Signal_v1<const std::string &> OnNewLine;

		//! Triggers when the console is cleared
		CL_Signal_v0 OnClear;

	protected:
		//! Lists all the data which has been added to the console.
		ConsoleLines m_Data;

		size_t m_MaxData;

	};
	
	//! Static method to safely add a message to the singleton object
	static void SendToConsole(const std::string &message)
	{
		Console* c = Console::getSingletonPtr();
		if (c != NULL)
		{
			c->Add(message);
		}

		// If the console hasn't been created, just send the message to standard output
		else
		{
			std::cout << message << std::endl;
		}
	}

	//! Static method to safely add a message to the singleton object
	/*!
	* \sa Console#Add(const std::string, MessageType)
	*/
	static void SendToConsole(const std::string &message, Console::MessageType type)
	{
		Console* c = Console::getSingletonPtr();
		if (c != NULL)
		{
			c->Add(message, type);
		}

		// If the console hasn't been created, just send the message to standard output
		else
		{
			std::cout << message << std::endl;
		}
	}

	//! Static method to safely add a message to the singleton object
	static void SendToConsole(const Exception &ex)
	{
		Console* c = Console::getSingletonPtr();
		if (c != NULL)
		{
			c->Add(ex.ToString(), Console::MTERROR);
		}

		// If the console hasn't been created, just send the message to standard output
		else
		{
			std::cout << ex.ToString() << std::endl;
		}
	}

}

#endif
