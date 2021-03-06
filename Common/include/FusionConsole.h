/*
*  Copyright (c) 2006-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionEngine_Console
#define H_FusionEngine_Console

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

// Inherited
#include "FusionSingleton.h"

#include "FusionScriptModule.h"
#include "FusionException.h"

#include <boost/signals2/signal.hpp>
#include <functional>
#include <unordered_map>

#include <tbb/atomic.h>

namespace FusionEngine
{
	
	//! Default maximum console buffer length
	static const size_t s_ConsoleDefaultBufferLength = 3200;


	//! Thrown by Console#Tokenise()
	class UnknownCommandException : public InvalidArgumentException
	{
	public:
		//! CTOR
		/*!
		* The descripton param is used to pass the name of the command
		*/
		UnknownCommandException(const std::string& description, const std::string& origin, const char* file, long line)
			: InvalidArgumentException(description, origin, file, line),
			command(description)
		{
		}
		//! DTOR
		virtual ~UnknownCommandException() throw() {}
		//! The command that couldn't be found
		std::string command;
	};

	namespace detail
	{
		class CommandHelpImpl;
	}

	//! Provides console data access to all FusionEngine objects
	/*!
	 * This class does not actually do anything with the data it contains,
	 * but instead uses callbacks to allow other classes to deal with console
	 * data.
	 */
	class Console : public Singleton<Console>, boost::noncopyable
	{
	public:
		//! Lines in the console
		//typedef std::list<std::string> ConsoleLines;

		typedef std::function<std::string (const StringVector&)> CommandCallback;
		typedef std::function<StringVector (int, const std::string&)> AutocompleteCallback;
		struct CommandFunctions
		{
			CommandCallback callback;
			AutocompleteCallback autocomplete;
		};
		typedef std::unordered_map<std::string, CommandFunctions> CommandCallbackMap;

		struct CommandHelp
		{
			std::string helpText;
			// Short descriptions of each argument - these may be printed in the
			//  command entry area of a console UI to indicate the expected inputs
			StringVector argumentNames;
		};

		//! Message header types
		enum MessageType {
			MTNORMAL = 0,
			MTERROR,
			MTWARNING
		};

		//! Basic constructor
		Console();

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

		//! Binds the given callback to the given command
		void BindCommand(const std::string &command, CommandCallback callback);
		//! Binds the given command with an autocomplete fn.
		void BindCommand(const std::string &command, CommandCallback callback, AutocompleteCallback autocomplete);
		//! Binds a command given the passed definition
		void BindCommand(const std::string &command, const CommandFunctions &functions, const CommandHelp &help);

		//! Removes binding identified by the given command
		void UnbindCommand(const std::string &command);

		//! Sets the help text for the given command
		void SetCommandHelpText(const std::string &command, const std::string &help_text, const StringVector &arg_names);

		//! Gets the help text for the given command
		CommandHelp GetCommandHelp(const std::string &command) const;

		//! Returns commands which begin with the given string
		void ListPrefixedCommands(const std::string &prefix, StringVector &possibleCommands, StringVector::size_type max_results = 0);

		//! Finds the command name closest to the given one
		/*
		* If the autocomplete results aren't satisfactory, this can
		* be used to provide command suggestions to the user.
		*/
		const std::string &ClosestCommand(const std::string &command) const;

		//! Returns possible completions for the last token in the given command (nothing if there is only the command argument itself)
		StringVector ListPossibleCompletions(const std::string& command) const;
		//! Replaces the last token in the given command with the given value
		std::string Autocomplete(const std::string& command, const std::string& completion) const;

		//! Returns the tokens that make up the given command
		std::pair<CommandFunctions, StringVector> Tokenise(const std::string& command) const;

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

		static void Register(ScriptManager* manager);

		void SetModule(ModulePtr module);

		void OnModuleBuild(BuildModuleEvent &ev);

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
		clan::Mutex m_HeadingCrapMutex;
		std::string m_LastHeading;

		std::string::size_type m_LastNewlineInBuffer;

		std::string::size_type m_LengthToNextSignal;
		std::string::size_type m_CharInterval;

		clan::Mutex m_BufferMutex;

		std::string::size_type m_BufferLength;
		std::string m_Buffer;

		CommandCallbackMap m_Commands;
		typedef detail::CommandHelpImpl CommandHelpImpl_t;
		std::shared_ptr<CommandHelpImpl_t> m_CommandHelp;

		boost::signals2::connection m_ModuleConnection;

		//size_t m_MaxData;
		//! Lists all the data which has been added to the console.
		//ConsoleLines m_Data;

	};

	//! Safely sends a message to the Console
	void SendToConsole(const std::string &message);

	//! Sends a sectioned message to the Console
	void SendToConsole(const std::string& heading, const std::string& message);

	//! Safely sends a message to the Console (wide-char)
	void SendToConsole(const std::wstring &message);

	//! Sends a sectioned message to the Console (wide-char)
	void SendToConsole(const std::wstring& heading, const std::wstring &message);

	//! Static method to safely add a message to the Console singleton
	/*!
	* \sa Console#Add(const std::string, MessageType)
	*/
	void SendToConsole(const std::string &message, Console::MessageType type);

	//! Static method to safely add a message to the Console singleton
	/*!
	* \sa Console#Add(const std::string, MessageType)
	*/
	void SendToConsole(const std::wstring &message, Console::MessageType type);

	//! Static method to safely add a message to the singleton object
	void SendToConsole(const Exception &ex);

}

#endif
