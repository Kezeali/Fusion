
#include "FusionStableHeaders.h"

#include "FusionConsole.h"

#include "FusionScriptManager.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionScriptedSlots.h"
#include "scriptstring.h"

#include <boost/tokenizer.hpp>
#include <functional>

#include <ClanLib/Core/Text/string_help.h>

using namespace std::placeholders;

namespace FusionEngine
{
	Console::Console()
		: m_BufferLength(s_ConsoleDefaultBufferLength),
		m_LastNewlineInBuffer(0),
		m_CharInterval(160),
		m_LengthToNextSignal(m_CharInterval)
	{
		m_Buffer.reserve(m_BufferLength);

		CommandFunctions helpFns;
		helpFns.callback = std::bind(&Console::CC_PrintCommandHelp, this, _1);
		helpFns.autocomplete = [this](int argn, const std::string& argv)->StringVector
		{
			StringVector possibleCommands;
			if (argn == 1)
			{
				auto range = m_CommandHelp.prefix_range(argv);
				for (; range.first != range.second; ++range.first)
					possibleCommands.push_back(range.first->first);
			}
			
			return possibleCommands;
		};

		CommandHelp helpHelp;
		helpHelp.helpText = "Shows a list of recognised commands. Enter 'help <command>' to see help for a specific command. Oh, you just did that.";
		helpHelp.argumentNames.push_back("[command_name]");

		BindCommand("help", helpFns, helpHelp);
	}

	const std::string &Console::GetExceptionMarker() const
	{
		static const std::string strExMkr("**");
		return strExMkr;
	}

	const std::string &Console::GetWarningMarker() const
	{
		static const std::string strWnMkr("##");
		return strWnMkr;
	}

	void Console::SetBufferLength(std::string::size_type length)
	{
		m_BufferLength = length;
		if (m_BufferLength < m_Buffer.length())
			m_Buffer.resize(length);
	}

	std::string::size_type Console::GetBufferLength() const
	{
		return m_BufferLength;
	}

	void Console::SetCharacterInterval(std::string::size_type interval)
	{
		m_CharInterval = interval;
	}

	std::string::size_type Console::GetCharacterInterval() const
	{
		return m_CharInterval;
	}

	void Console::add_raw(const std::string& message)
	{
		if (message.empty())
			return;

		// Write the string to the buffer
		append_buffer(message);

		// Fire new data signal
		OnNewData(message);

		CL_MutexSection lock(&m_BufferMutex);

		// Fire newline signal(s)
		if (!OnNewLine.empty())
		{
			if (m_LastNewlineInBuffer > m_BufferLength)
				m_LastNewlineInBuffer = 0;
			std::string::size_type newlinePos = m_LastNewlineInBuffer, startOfLastNewLine;

			for (;;)
			{
				if (m_LastNewlineInBuffer == 0)
					startOfLastNewLine = 0;
				else
					startOfLastNewLine = newlinePos + 1;
				newlinePos = m_Buffer.find('\n', newlinePos+1);
				if (newlinePos != std::string::npos)
				{
					m_LastNewlineInBuffer = newlinePos;
					if (!(startOfLastNewLine == 0 && newlinePos == 1))
					{
						auto newLine = m_Buffer.substr(startOfLastNewLine, newlinePos-startOfLastNewLine);
						lock.unlock();
						// Signal
						OnNewLine(newLine);
						lock.lock();
					}
				}
				else
					break;
			}
		}

		// Fire char interval signal
		if (m_LengthToNextSignal >= message.length())
		{
			m_LengthToNextSignal = m_CharInterval;
			if (!OnCharacterInterval.empty())
			{
				std::string range;
				if (m_Buffer.length() > m_CharInterval)
					range = m_Buffer.substr(m_Buffer.length() - m_CharInterval);
				else
					range = m_Buffer;
				lock.unlock();
				OnCharacterInterval(range);
			}
		}
		else
			m_LengthToNextSignal -= message.length();
	}

	void Console::append_buffer(const std::string &string)
	{
		CL_MutexSection lock(&m_BufferMutex);
		if (string.length() >= m_BufferLength)
		{
			// The last character in the string should end up as the last
			//  character in the buffer, so figure out where to start from:
			std::string::size_type start = string.length() - m_BufferLength;
			m_Buffer.assign(string, start, m_BufferLength);

			// All new text in the buffer, so we'll have to check for newlines from the start
			m_LastNewlineInBuffer = 0;
		}
		else if (m_Buffer.length() + string.length() > m_BufferLength)
		{
			// Slide the buffer back to fit the string (dropping data off the beginning)
			m_Buffer.assign(m_Buffer.data() + string.length(), m_Buffer.length() - string.length());
			// Put the string on the end of the buffer
			m_Buffer.append(string);

			// The previous data has moved back string.length(), so the update the recorded newline pos.
			if (m_LastNewlineInBuffer >= string.length())
				m_LastNewlineInBuffer -= string.length();
			else if (m_LastNewlineInBuffer != 0)
				m_LastNewlineInBuffer = 0;
		}
		else
		{
			// Current buffer length + string length still won't reach capacity, so
			// the string can be appended straight on the end.
			m_Buffer += string;
		}
	}

	inline std::string nHyphens(std::string::size_type n)
	{
		// Could do this with a table, but I don't care that much
		std::string string;
		for (std::string::size_type i=0; i<n; i++)
			string += "-";
		return string;
	}

	void Console::Add(const std::wstring &message)
	{
		Add(CL_StringHelp::ucs2_to_local8(message));
	}

	void Console::Add(const std::string &message)
	{
		{
			CL_MutexSection lock(&m_HeadingCrapMutex);
			// Close the grouped entries section
			if (!m_LastHeading.empty())
			{
				m_LastHeading = "";
				lock.unlock();
				// Add a closing line
				add_raw("}\n");
			}
		}

		add_raw(message + '\n');
	}

	void Console::Add(const std::string& message, MessageType type)
	{
		std::string headedMessage;
		switch (type)
		{
		case MTNORMAL:
			headedMessage = message;
			break;
		case MTWARNING:
			headedMessage += GetWarningMarker() + " Warning: " + message;
			break;
		case MTERROR:
			headedMessage += GetExceptionMarker() + " Error:   " + message;
			break;
		default:
			headedMessage = message;
		}

		// Add the message to the console data
		Add(headedMessage);
	}

	void Console::Add(const std::string& heading, const std::string& message)
	{
		CL_MutexSection lock(&m_HeadingCrapMutex);
		if (heading != m_LastHeading)
		{
			lock.unlock();
			// Add new heading (will also close the current heading if there is one)
			Add("[" + heading + "]:");
			add_raw("{\n");
		}

		// Add the message
		add_raw(' ' + message + '\n');

		lock.lock();
		m_LastHeading = heading;
	}

	std::string Console::CC_PrintCommandHelp(const StringVector &args)
	{
		// First arg (arg 0) is the command name which invoked this
		//  method, so args.size() == 1 is essentually a command
		//  call without args:
		if (args.size() == 1)
		{
			Add("Recognised Commands (names): ");
			for (CommandHelpMap::iterator it = m_CommandHelp.begin(), end = m_CommandHelp.end();
				it != end; ++it)
			{
				Add("\t " + it->first);
			}
			Add("Enter 'help <name of a command>' to see the instructions for using a specific command (if available.)");
		}
		// More than one arg:
		else
		{
			// arg 1 should be a command name:
			CommandHelpMap::iterator _where = m_CommandHelp.find(args[1]);
			if (_where == m_CommandHelp.end())
			{
				return args[1] + " is not a recognised command.";
			}

			Add("Help for '" + _where->first + "':");
			if (!_where->second.argumentNames.empty())
			{
				Add("Usage:");
				std::string argNames = "";
				for (StringVector::iterator it = _where->second.argumentNames.begin(), end = _where->second.argumentNames.end();
					it != end; ++it)
				{
					argNames += " " + *it;
				}
				Add("\t " + _where->first + argNames);
				Add("\t  Arguments in brackets are optional");
			}
			Add("Description:");
			if (!_where->second.helpText.empty())
				Add(" " + _where->second.helpText);
			else
				Add(" There is no help text associated with this command.");
		}

		return "";
	}

	void Console::BindCommand(const std::string &command, Console::CommandCallback callback)
	{
		m_Commands[command].callback = callback;

		m_CommandHelp[command];
	}

	void Console::BindCommand(const std::string &command, Console::CommandCallback callback, Console::AutocompleteCallback autocomplete)
	{
		CommandFunctions &fns = m_Commands[command];
		fns.callback = callback;
		fns.autocomplete = autocomplete;

		m_CommandHelp[command];
	}

	void Console::BindCommand(const std::string &command, const Console::CommandFunctions &functions, const Console::CommandHelp &help)
	{
		m_Commands[command] = functions;
		m_CommandHelp[command] = help;
	}

	void Console::UnbindCommand(const std::string &command)
	{
		m_Commands.erase(command);
		m_CommandHelp.erase(command);
	}

	void Console::SetCommandHelpText(const std::string &command, const std::string &help_text, const FusionEngine::StringVector &arg_names)
	{
		CommandHelpMap::iterator _where = m_CommandHelp.find(command);
		if (_where != m_CommandHelp.end())
		{
			CommandHelp &helpData = _where->second;
			helpData.helpText = help_text;
			helpData.argumentNames = arg_names;
		}
	}

	void Console::ListPrefixedCommands(const std::string &prefix, StringVector &possibleCommands, StringVector::size_type max_results)
	{
		typedef std::pair<CommandHelpMap::iterator, CommandHelpMap::iterator> HelpIterRange;
		HelpIterRange range = m_CommandHelp.prefix_range(prefix);

		if (max_results != 0)
			possibleCommands.reserve(max_results);
		StringVector::size_type count = 0;
		while (range.first != range.second)
		{
			if (max_results != 0 && ++count > max_results)
				break;
			possibleCommands.push_back(range.first->first);
			++range.first;
		}
	}

	const std::string &Console::ClosestCommand(const std::string &command) const
	{
		static std::string noResult;

		CommandHelpMap::search_results_list results = m_CommandHelp.create_search_results();
		m_CommandHelp.levenshtein_search(command, std::back_inserter(results), 3);
		
		if (!results.empty())
			return (*results.begin())->first;
		else
			return noResult;
	}

	StringVector Console::ListPossibleCompletions(const std::string& command) const
	{
		try
		{
			auto parsed = Tokenise(command);
			if (parsed.second.size() > 1 && parsed.first.autocomplete) // the second clause checks that the autocomplete function is bound
			{
				return parsed.first.autocomplete(parsed.second.size() - 1, parsed.second.back());
			}
		}
		catch (InvalidArgumentException&)
		{}

		return StringVector();
	}

	std::string Console::Autocomplete(const std::string& command, const std::string& completion) const
	{
		try
		{
			StringVector args = Tokenise(command).second;
			if (args.size() > 1)
			{
				args.pop_back();
				args.push_back(completion);
				std::string completedCommand = args.front();
				std::for_each(args.begin() + 1, args.end(), [&completedCommand](const std::string& arg) { completedCommand += " " + arg; });
				return completedCommand;
			}
			else
				return command;
		}
		catch (InvalidArgumentException&)
		{}

		return "";
	}

	inline void addToken(StringVector &args, std::string &quote, bool inQuote, const std::string& token)
	{
		if (inQuote)
			quote += token;
		else
			args.push_back(token);
	}

	std::pair<Console::CommandFunctions, StringVector> Console::Tokenise(const std::string& untrimmed_command) const
	{
		std::string command = fe_trim(untrimmed_command);
		if (command.empty())
			FSN_EXCEPT(InvalidArgumentException, "Huh?");

		// TODO: write tokenizer fn. that works like char_separator but processes escape sequences
		typedef boost::char_separator<char> tokFnType;
		typedef boost::tokenizer<tokFnType> tokenizer;

		// Start tokenizing the command string
		tokFnType tokFn("", " ,\\\"");
		tokenizer tok(command, tokFn);
		tokenizer::iterator it = tok.begin();
		// Make sure there is a first token (i.e. this string contains command identifier)
		if (it != tok.end())
		{
			// Check that the first token is a valid command identifier
			auto _where = m_Commands.find(*it);
			if (_where != m_Commands.end())
			{
				// Tokenize the arguments
				StringVector args; bool inQuote = false; std::string quote;
				for (tokenizer::iterator end = tok.end(); it != end; ++it)
				{
					// Escape chars
					if (*it == "\\")
					{
						if (++it != end)
						{
							//if (*it == "n")
								//addToken(args, quote, inQuote, "\n");
							if (*it == "\"")
								addToken(args, quote, inQuote, "\"");
							else if (*it == "\\")
								addToken(args, quote, inQuote, "\\");
						}
						else
							break;

						continue;
					}
					// Check for quote char
					else if (*it == "\"")
					{
						if (!inQuote)
						{
							inQuote = true;
							quote.clear();
						}
						else
						{
							inQuote = false;
							args.push_back(quote);
							quote.clear();
						}
					}
					// Tokens within a quote are concatenated
					else if (inQuote)
						quote += *it;
					else if (*it != " " && *it != ",")
						args.push_back(*it);
				}
				return std::make_pair(_where->second, args);
			}
			else
			{
				FSN_EXCEPT(UnknownCommandException, *it);
			}
		}

		FSN_EXCEPT(InvalidArgumentException, "Failed to parse the given command");
	}

	void Console::Interpret(const std::string &untrimmed)
	{
		try
		{
			// Tokenise returns a pair containing a reference to a CommandFunctions object and a vector containing the parsed tokens
			auto parsed = Tokenise(untrimmed);
			if (!parsed.second.empty() && parsed.first.callback)
				// Execute the callback and Add it's return value to the console
				Add( parsed.first.callback(parsed.second) );
		}
		catch (UnknownCommandException& ex)
		{
			Add("'" + ex.command + "' is not a recognised command.");
			const std::string &closestCommand = ClosestCommand(ex.command);
			if (!closestCommand.empty())
				Add(" * The closest known command is '" + closestCommand + "'");
			Add("Enter 'help' to for a list of recognised commands.");
		}
		catch (InvalidArgumentException&)
		{
		}
	}

	// TODO: void add(const std::string &string, bool append_newline)
	void Console::Print(const std::string &string)
	{
		CL_MutexSection lock(&m_HeadingCrapMutex);
		// Close the grouped entries section
		if (!m_LastHeading.empty())
		{
			m_LastHeading = "";
			lock.unlock();
			// Add a closing line
			add_raw("}\n");
		}

		add_raw(string);
	}

	// TODO: void add(const std::string &heading, const std::string &string, bool append_newline)
	void Console::Print(const std::string &heading, const std::string &string)
	{
		CL_MutexSection lock(&m_HeadingCrapMutex);
		if (heading != m_LastHeading)
		{
			lock.unlock();
			// Add new heading (will also close the current heading if there is one)
			Add("[" + heading + "]:");
			add_raw("{\n");
		}

		// Add the message
		add_raw(' ' + string);

		lock.lock();
		m_LastHeading = heading;
	}

	void Console::PrintLn(const std::string &message)
	{
		Add(message);
	}

	void Console::PrintLn(const std::string &heading, const std::string& message)
	{
		Add(heading, message);
	}

	void Console::PrintLn(int message)
	{
		Add(CL_StringHelp::int_to_local8(message));
	}

	void Console::PrintLn(double message)
	{
		Add(CL_StringHelp::double_to_local8(message));
	}

	void Console::Clear()
	{
		CL_MutexSection lock(&m_BufferMutex);
		if (!m_Buffer.empty())
		{
			m_Buffer.clear();

			m_LastNewlineInBuffer = 0;
			m_LengthToNextSignal = m_CharInterval;

			lock.unlock();
			// Signal
			OnClear();
		}
	}

	std::string Console::GetHistory() const
	{
		return m_Buffer.substr(m_Buffer.find("\n"));
	}

//#define FSN_DONT_USE_SCRIPTING

#ifndef FSN_DONT_USE_SCRIPTING
	class ScriptedConsoleListenerWrapper : public RefCounted, noncopyable
	{
	public:
		ScriptedConsoleListenerWrapper(asIScriptObject *listener, Console *console);
		~ScriptedConsoleListenerWrapper();

		void Disconnect();

	private:
		void OnNewLine(const std::string &line);
		void OnNewData(const std::string &data);
		void OnClear();

		//ScriptUtils::Calling::Caller m_CallOnNewLine;
		//ScriptUtils::Calling::Caller m_CallOnNewData;
		//ScriptUtils::Calling::Caller m_CallOnClear;

		boost::signals2::connection m_ConsoleOnNewLineConnection;
		boost::signals2::connection m_ConsoleOnNewDataConnection;
		boost::signals2::connection m_ConsoleOnClearConnection;

		asIScriptObject *m_Listener;
	};

	ScriptedConsoleListenerWrapper::ScriptedConsoleListenerWrapper(asIScriptObject *listener, Console *console)
		: m_Listener(listener)
	{
		ScriptUtils::Calling::Caller callNewLine(m_Listener, "void OnNewLine(const string &in)");
		ScriptUtils::Calling::Caller callNewData(m_Listener, "void OnNewData(const string &in)");
		ScriptUtils::Calling::Caller callClear(m_Listener, "void OnClear()");

		if (callNewLine)
			m_ConsoleOnNewLineConnection = console->OnNewLine.connect( std::bind(&ScriptedConsoleListenerWrapper::OnNewLine, this, _1) );

		if (callNewData)
			m_ConsoleOnNewDataConnection = console->OnNewData.connect( std::bind(&ScriptedConsoleListenerWrapper::OnNewData, this, _1) );

		if (callClear)
			m_ConsoleOnClearConnection = console->OnClear.connect( std::bind(&ScriptedConsoleListenerWrapper::OnClear, this) );
	}

	ScriptedConsoleListenerWrapper::~ScriptedConsoleListenerWrapper()
	{
		m_ConsoleOnNewLineConnection.disconnect();
		m_ConsoleOnNewDataConnection.disconnect();
		m_ConsoleOnClearConnection.disconnect();
	}

	void ScriptedConsoleListenerWrapper::Disconnect()
	{
		m_ConsoleOnNewLineConnection.disconnect();
		m_ConsoleOnNewDataConnection.disconnect();
		m_ConsoleOnClearConnection.disconnect();
	}

	void ScriptedConsoleListenerWrapper::OnNewLine(const std::string &line)
	{
		ScriptUtils::Calling::Caller f(m_Listener, "void OnNewLine(const string &in)");
		if (f)
			f(new CScriptString(line));
	}

	void ScriptedConsoleListenerWrapper::OnNewData(const std::string &data)
	{
		ScriptUtils::Calling::Caller f(m_Listener, "void OnNewData(const string &in)");
		if (f)
			f(new CScriptString(data));
	}

	void ScriptedConsoleListenerWrapper::OnClear()
	{
		ScriptUtils::Calling::Caller f(m_Listener, "void OnClear()");
		if (f)
			f();
	}

	ScriptedConsoleListenerWrapper* Scr_ConnectConsoleListener(asIScriptObject *listener, Console *obj)
	{
		ScriptedConsoleListenerWrapper *wrapper = new ScriptedConsoleListenerWrapper(listener, obj);
		listener->Release();
		return wrapper;
	}


	ScriptedSlotWrapper* Scr_ConnectNewLineSlot(const std::string &decl, Console *obj)
	{
		ScriptedSlotWrapper *slot = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), decl);
		if (slot != nullptr)
		{
			boost::signals2::connection c = obj->OnNewLine.connect( std::bind(&ScriptedSlotWrapper::Callback<const std::string &>, slot, _1) );
			slot->HoldConnection(c);
		}
		return slot;
	}

	ScriptedSlotWrapper* Scr_ConnectNewDataSlot(const std::string &decl, Console *obj)
	{
		ScriptedSlotWrapper *slot = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), decl);
		if (slot != nullptr)
		{
			boost::signals2::connection c = obj->OnNewData.connect( std::bind(&ScriptedSlotWrapper::Callback<const std::string &>, slot, _1) );
			slot->HoldConnection(c);
		}
		return slot;
	}

	ScriptedSlotWrapper* Scr_ConnectClearSlot(const std::string &decl, Console *obj)
	{
		ScriptedSlotWrapper *slot = ScriptedSlotWrapper::CreateWrapperFor(asGetActiveContext(), decl);
		if (slot != nullptr)
		{
			boost::signals2::connection c = obj->OnClear.connect( std::bind(&ScriptedSlotWrapper::CallbackNoParam, slot) );
			slot->HoldConnection(c);
		}
		return slot;
	}

	void RegisterScriptedConsoleListener(asIScriptEngine *engine)
	{
		int r;
		engine->RegisterInterface("IConsoleListener");
		//engine->RegisterInterfaceMethod("IConsoleListener", "void OnNewLine(const string&in)");
		//engine->RegisterInterfaceMethod("IConsoleListener", "void OnNewData(const string&in)");
		//engine->RegisterInterfaceMethod("IConsoleListener", "void OnClear()");

		RefCounted::RegisterType<ScriptedConsoleListenerWrapper>(engine, "ConsoleConnection");
		r = engine->RegisterObjectMethod("ConsoleConnection",
			"void disconnect()",
			asMETHOD(ScriptedConsoleListenerWrapper, Disconnect), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"ConsoleConnection@ connectListener(IConsoleListener@)",
			asFUNCTION(Scr_ConnectConsoleListener), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	void RegisterScriptedConsoleCallbacks(asIScriptEngine *engine)
	{
		int r;

		r = engine->RegisterObjectMethod("Console",
			"SignalConnection@ connectToNewLine(const string &in)",
			asFUNCTION(Scr_ConnectNewLineSlot), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"SignalConnection@ connectToNewData(const string &in)",
			asFUNCTION(Scr_ConnectNewDataSlot), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"SignalConnection@ connectToClear(const string &in)",
			asFUNCTION(Scr_ConnectClearSlot), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	void Console_ListPrefixedCommands(const std::string& prefix, StringVector& possible_commands, Console& con)
	{
		con.ListPrefixedCommands(prefix, possible_commands);
	}
#endif

	void Console::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( std::bind(&Console::OnModuleBuild, this, _1) );
	}

	void Console::OnModuleBuild(BuildModuleEvent &ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
			ev.manager->RegisterGlobalObject("Console console", this);
	}

	void Console_ListPossibleCompletions(const std::string& command, StringVector& out, Console* obj)
	{
		out = obj->ListPossibleCompletions(command);
	}

	CScriptString* Console_Autocomplete(const std::string& command, const std::string& completion, Console* obj)
	{
		return new CScriptString( obj->Autocomplete(command, completion) );
	}

	void Console::Register(ScriptManager *manager)
	{
#ifndef FSN_DONT_USE_SCRIPTING
		int r;
		asIScriptEngine* engine = manager->GetEnginePtr();

		RegisterSingletonType<Console>("Console", engine);

		r = engine->RegisterObjectMethod("Console",
			"void print(string &in)",
			asMETHODPR(Console, Print, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Console",
			"void print(string &in, string &in)",
			asMETHODPR(Console, Print, (const std::string&, const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		
		r = engine->RegisterObjectMethod("Console",
			"void println(string &in)",
			asMETHODPR(Console, PrintLn, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Console",
			"void println(string &in, string &in)",
			asMETHODPR(Console, PrintLn, (const std::string&, const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		r = engine->RegisterObjectMethod("Console",
			"void println(int)",
			asMETHODPR(Console, PrintLn, (int), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod(
			"Console",
			"void println(double)",
			asMETHODPR(Console, PrintLn, (double), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void listPrefixedCommands(const string &in, StringArray &out)",
			asFUNCTIONPR(Console_ListPrefixedCommands, (const std::string&, StringVector&, Console&), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void listPrefixedCommands(const string &in, StringArray &out, uint max_results)",
			asMETHODPR(Console, ListPrefixedCommands, (const std::string&, StringVector&, StringVector::size_type), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void listPossibleCompletions(const string &in, StringArray &out) const",
			asFUNCTION(Console_ListPossibleCompletions),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"string@ autocomplete(const string &in, const string &in) const",
			asFUNCTION(Console_Autocomplete),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void interpret(const string &in)",
			asMETHODPR(Console, Interpret, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console", "void clear()", asMETHOD(Console, Clear), asCALL_THISCALL);
		FSN_ASSERT(r >= 0);

		RegisterScriptedConsoleListener(engine);
		RegisterScriptedConsoleCallbacks(engine);
#endif
	}

}