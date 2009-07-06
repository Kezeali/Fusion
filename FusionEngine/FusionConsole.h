/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#include "FusionBoostSignals2.h"

// Inherited
#include "FusionSingleton.h"

// Fusion
//#include "FusionCircularStringBuffer.h"

// External
#include <containers/structured_map.hpp>

namespace FusionEngine
{
	//! Default maximum console buffer length
	static const size_t s_ConsoleDefaultBufferLength = 3200;

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
	 * data.
	 */
	class Console : public Singleton<Console>
	{
	public:
		//! Lines in the console
		//typedef std::list<std::string> ConsoleLines;

		typedef boost::function<std::string (const StringVector&)> CommandCallback;
		typedef boost::function<std::string (int, const std::string&)> AutocompleteCallback;
		struct CommandFunctions
		{
			CommandCallback callback;
			AutocompleteCallback autocomplete;
		};
		typedef std::tr1::unordered_map<std::string, CommandFunctions> CommandCallbackMap;

		struct CommandHelp
		{
			std::string helpText;
			// Short descriptions of each argument - these may be printed in the
			//  command entry area of a console UI to indicate the expected inputs
			StringVector argumentNames;
		};
		// Used for auto-complete (hence useage of map rather than unordered_map)
		//  and getting / listing help strings
		typedef containers::structured_map<std::string, CommandHelp> CommandHelpMap;

		//! Message header types
		enum MessageType {
			MTNORMAL = 0,
			MTERROR,
			MTWARNING
		};

	public:
		//! Basic constructor
		Console();

	public:
		//! Returns the exception marker
		const std::string& GetExceptionMarker() const;
		//! Returns the warning marker
		const std::string& GetWarningMarker() const;

		void SetBufferLength(std::string::size_type length);
		std::string::size_type GetBufferLength() const;

		void SetCharacterInterval(std::string::size_type interval);
		std::string::size_type GetCharacterInterval() const;

		//! Adds the given message to the console history (Wide-Char)
		void Add(const std::wstring &message);
		//! Adds the given message to the console history
		void Add(const std::string &message);

		//! Adds the given message, after prepending it with a heading of the specified type, to the console history
		void Add(const std::string &message, MessageType type);

		//! Adds the given message under the given heading
		/*!
		 * If the last message added was under the same heading, this message
		 * will be added directly below it. Otherwise a new heading will be added.
		 */
		void Add(const std::string& heading, const std::string &message);

		//! This member is bound as a console command.
		/*
		* Prints the names of all the bound console commands. If a
		* specific command is given as an arg, the help string for
		* that command is printed.
		*/
		std::string CC_PrintCommandHelp(const StringVector &args);

		//! Binds the given callback to the given command
		void BindCommand(const std::string &command, CommandCallback callback);
		//! Binds the given command with an autocomplete fn.
		void BindCommand(const std::string &command, CommandCallback callback, AutocompleteCallback autocomplete);
		//! Binds a command given the passed definition
		void BindCommand(const std::string &command, const CommandFunctions &functions, const CommandHelp &help);

		//! Sets the help text for the given command
		void SetCommandHelpText(const std::string &command, const std::string &help_text, const StringVector &arg_names);

		//! Returns commands which begin with the given string
		void ListPrefixedCommands(const std::string &prefix, StringVector &possibleCommands, StringVector::size_type max_results = 0);

		//! Finds the command name closest to the given one
		/*
		* If the autocomplete results aren't satisfactory, this can
		* be used to provide command suggestions to the user.
		*/
		const std::string &ClosestCommand(const std::string &command) const;

		//! Runs the given command.
		/*!
		 * The callback indexed by the given command name will be called
		 * and the result (the return value of the callback) will be
		 * printed to the console.
		 */
		void Interpret(const std::string &command);

		//! Adds the given Error to the console history
		//void Add(const Exception *error);

		//! Adds the given string to the console without appending a newline
		void Print(const std::string &string);

		//! Adds the given string to the console under the given heading without appending a newline
		void Print(const std::string &heading, const std::string &string);

		//! Adds the given message to the console history
		void PrintLn(const std::string &message);

		//! Adds the given message to the console under the given heading
		void PrintLn(const std::string &heading, const std::string &message);

		//! Adds the given int to the console history
		void PrintLn(int message);

		//! Adds the given double to the console history
		void PrintLn(double message);

		//! Removes all data from the console
		void Clear();

		//! Returns all the lines that have been added to the console
		std::string GetHistory() const;

		void RegisterScriptElements(ScriptingEngine* manager);

		//! Triggers when new data is added to the console
		boost::signals2::signal<void (const std::string&)> OnNewLine;

		//! Triggers whenever a certain number of characters have been written to the console
		/*!
		 * Set the trigger count with SetCharacterInterval()
		 */
		boost::signals2::signal<void (const std::string&)> OnCharacterInterval;

		//! Fired at the Console's discression
		boost::signals2::signal<void (const std::string&)> OnNewData;

		//! Triggers when the console is cleared
		boost::signals2::signal<void ()> OnClear;

	protected:
		void add_raw(const std::string& string);

		void append_buffer(const std::string &string);
	protected:
		std::string m_LastHeading;

		std::string::size_type m_LastNewlineInBuffer;

		std::string::size_type m_LengthToNextSignal;
		std::string::size_type m_CharInterval;

		std::string::size_type m_BufferLength;
		std::string m_Buffer;

		CommandCallbackMap m_Commands;
		CommandHelpMap m_CommandHelp;

		//size_t m_MaxData;
		//! Lists all the data which has been added to the console.
		//ConsoleLines m_Data;

	};

	//! Safely sends a message to the Console
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

	//! Sends a sectioned message to the Console
	static void SendToConsole(const std::string& heading, const std::string& message)
	{
		Console* c = Console::getSingletonPtr();
		if (c != NULL)
		{
			c->Add(heading, message);
		}
		// If the console hasn't been created, just send the message to standard output
		else
		{
			std::cout << heading << ": " << message << std::endl;
		}
	}

	//! Safely sends a message to the Console (wide-char)
	static void SendToConsole(const std::wstring &message)
	{
		SendToConsole(fe_narrow(message));
	}

	//! Sends a sectioned message to the Console (wide-char)
	static void SendToConsole(const std::wstring& heading, const std::wstring &message)
	{
		SendToConsole(fe_narrow(heading), fe_narrow(message));
	}

	//! Static method to safely add a message to the Console singleton
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

	//! Static method to safely add a message to the Console singleton
	/*!
	* \sa Console#Add(const std::string, MessageType)
	*/
	static void SendToConsole(const std::wstring &message, Console::MessageType type)
	{
		SendToConsole(fe_narrow(message), type);
	}

	//! Static method to safely add a message to the singleton object
	static void SendToConsole(const Exception &ex)
	{
		Console* c = Console::getSingletonPtr();
		if (c != NULL)
		{
			std::string message = ex.ToString();
			c->Add(message, Console::MTERROR);
		}

		// If the console hasn't been created, just send the message to standard output
		else
		{
			std::cout << ex.ToString() << std::endl;
		}
	}

}

#endif
