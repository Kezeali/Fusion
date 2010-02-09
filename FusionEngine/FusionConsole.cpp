
#include "Common.h"

#include "FusionConsole.h"

#include "FusionScriptingEngine.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionScriptedSlots.h"
#include "scriptstring.h"

#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>

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
		helpFns.callback = boost::bind(&Console::CC_PrintCommandHelp, this, _1);

		CommandHelp helpHelp;
		helpHelp.helpText = "Shows a list of recognised commands.\nEnter 'help <command>' to see help for a specific command. For example, what you just did.";
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

		// Fire newline signal(s)
		if (!OnNewLine.empty())
		{
			if (m_LastNewlineInBuffer > m_BufferLength)
				m_LastNewlineInBuffer = 0;
			std::string::size_type newlinePos = m_LastNewlineInBuffer, startOfLastNewLine;
			while (true)
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
						// Signal
						OnNewLine( m_Buffer.substr(startOfLastNewLine, newlinePos-startOfLastNewLine) );
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
				if (m_Buffer.length() > m_CharInterval)
					OnCharacterInterval( m_Buffer.substr(m_Buffer.length() - m_CharInterval) );
				else
					OnCharacterInterval(m_Buffer);
			}
		}
		else
			m_LengthToNextSignal -= message.length();
	}

	void Console::append_buffer(const std::string &string)
	{
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

	inline std::string nHyphens(std::wstring::size_type n)
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
		// Close the grouped entries section
		if (!m_LastHeading.empty())
		{
			m_LastHeading = "";
			// Add a closing line
			add_raw("}\n");
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
		if (heading != m_LastHeading)
		{
			// Add new heading (will also close the current heading if there is one)
			Add("[" + heading + "]:");
			add_raw("{\n");
		}

		// Add the message
		add_raw(' ' + message + '\n');

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

	inline void addToken(StringVector &args, std::string &quote, bool inQuote, const std::string& token)
	{
		if (inQuote)
			quote += token;
		else
			args.push_back(token);
	}

	void Console::Interpret(const std::string &untrimmed)
	{
		std::string command = fe_trim(untrimmed);
		if (command.empty())
			return;

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
			CommandCallbackMap::iterator _where = m_Commands.find(*it);
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
				// Execute the callback and Add it's return value to the console
				Add( _where->second.callback(args) );
			}
			else
			{
				Add(*it + " is not a recognised command.\nEnter 'help' to for a list of recognised commands.");
				const std::string &closestCommand = ClosestCommand(*it);
				if (!closestCommand.empty())
					Add("Did you mean " + closestCommand + "?");
			}
		}
	}

	// TODO: void add(const std::string &string, bool append_newline)
	void Console::Print(const std::string &string)
	{
		// Close the grouped entries section
		if (!m_LastHeading.empty())
		{
			m_LastHeading = "";
			// Add a closing line
			add_raw("}\n");
		}

		add_raw(string);
	}

	// TODO: void add(const std::string &heading, const std::string &string, bool append_newline)
	void Console::Print(const std::string &heading, const std::string &string)
	{
		if (heading != m_LastHeading)
		{
			// Add new heading (will also close the current heading if there is one)
			Add("[" + heading + "]:");
			add_raw("{\n");
		}

		// Add the message
		add_raw(' ' + string);

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
		if (!m_Buffer.empty())
		{
			m_Buffer.clear();

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

		bsig2::connection m_ConsoleOnNewLineConnection;
		bsig2::connection m_ConsoleOnNewDataConnection;
		bsig2::connection m_ConsoleOnClearConnection;

		asIScriptObject *m_Listener;
	};

	ScriptedConsoleListenerWrapper::ScriptedConsoleListenerWrapper(asIScriptObject *listener, Console *console)
		//: m_CallOnNewLine(listener, "void OnNewLine(const string &in)"),
		//m_CallOnNewData(listener, "void OnNewData(const string &in)"),
		//m_CallOnClear(listener, "void OnClear()"),
		: m_Listener(listener)
	{
		//if (m_CallOnNewLine.ok())
		//	m_ConsoleOnNewLineConnection = console->OnNewLine.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnNewLine, this, _1) );

		//if (m_CallOnNewData.ok())
		//	m_ConsoleOnNewDataConnection = console->OnNewData.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnNewData, this, _1) );

		//if (m_CallOnClear.ok())
		//	m_ConsoleOnClearConnection = console->OnClear.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnClear, this) );

		//m_CallOnNewLine.release();
		//m_CallOnNewData.release();
		//m_CallOnClear.release();

		ScriptUtils::Calling::Caller callNewLine(m_Listener, "void OnNewLine(const string &in)");
		ScriptUtils::Calling::Caller callNewData(m_Listener, "void OnNewData(const string &in)");
		ScriptUtils::Calling::Caller callClear(m_Listener, "void OnClear()");

		if (callNewLine.ok())
			m_ConsoleOnNewLineConnection = console->OnNewLine.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnNewLine, this, _1) );

		if (callNewData.ok())
			m_ConsoleOnNewDataConnection = console->OnNewData.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnNewData, this, _1) );

		if (callClear.ok())
			m_ConsoleOnClearConnection = console->OnClear.connect( boost::bind(&ScriptedConsoleListenerWrapper::OnClear, this) );
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
		if (f.ok())
			f(new CScriptString(line));
	}

	void ScriptedConsoleListenerWrapper::OnNewData(const std::string &data)
	{
		ScriptUtils::Calling::Caller f(m_Listener, "void OnNewData(const string &in)");
		if (f.ok())
			f(new CScriptString(data));
	}

	void ScriptedConsoleListenerWrapper::OnClear()
	{
		ScriptUtils::Calling::Caller f(m_Listener, "void OnClear()");
		if (f.ok())
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
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			asIScriptModule *module = ctxGetModule(context);
			ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(module, decl);

			boost::signals2::connection c = obj->OnNewLine.connect( boost::bind(&ScriptedSlotWrapper::Callback<const std::string &>, slot, _1) );
			slot->HoldConnection(c);

			return slot;
		}

		return NULL;
	}

	ScriptedSlotWrapper* Scr_ConnectNewDataSlot(const std::string &decl, Console *obj)
	{
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			asIScriptModule *module = ctxGetModule(context);
			ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(module, decl);

			bsig2::connection c = obj->OnNewData.connect( boost::bind(&ScriptedSlotWrapper::Callback<const std::string &>, slot, _1) );
			slot->HoldConnection(c);

			return slot;
		}

		return NULL;
	}

	ScriptedSlotWrapper* Scr_ConnectClearSlot(const std::string &decl, Console *obj)
	{
		asIScriptContext *context = asGetActiveContext();
		if (context != NULL)
		{
			asIScriptModule *module = ctxGetModule(context);
			ScriptedSlotWrapper *slot = new ScriptedSlotWrapper(module, decl);

			bsig2::connection c = obj->OnClear.connect( boost::bind(&ScriptedSlotWrapper::Callback, slot) );
			slot->HoldConnection(c);

			return slot;
		}

		return NULL;
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
			"CallbackConnection@ connectTo_NewLine(const string &in)",
			asFUNCTION(Scr_ConnectNewLineSlot), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"CallbackConnection@ connectTo_NewData(const string &in)",
			asFUNCTION(Scr_ConnectNewLineSlot), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"CallbackConnection@ connectTo_Clear(const string &in)",
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
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&Console::OnModuleBuild, this, _1) );
	}

	void Console::OnModuleBuild(BuildModuleEvent &ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
			ev.manager->RegisterGlobalObject("Console console", this);
	}

	void Console::Register(ScriptingEngine *manager)
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
			"void listPrefixedCommands(string &in, StringArray &out)",
			asFUNCTIONPR(Console_ListPrefixedCommands, (const std::string&, StringVector&, Console&), void),
			asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void listPrefixedCommands(string &in, StringArray &out, uint max_results)",
			asMETHODPR(Console, ListPrefixedCommands, (const std::string&, StringVector&, StringVector::size_type), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console",
			"void interpret(string &in)",
			asMETHODPR(Console, Interpret, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Console", "void clear()", asMETHOD(Console, Clear), asCALL_THISCALL);
		FSN_ASSERT(r >= 0);

		RegisterScriptedConsoleListener(engine);
		RegisterScriptedConsoleCallbacks(engine);
#endif
	}

}