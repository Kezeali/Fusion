
#include "FusionConsole.h"

#include "FusionScriptingEngine.h"
#include "FusionScriptTypeRegistrationUtils.h"

#include <boost/tokenizer.hpp>

namespace FusionEngine
{
	Console::Console()
		: m_BufferLength(s_ConsoleDefaultBufferLength),
		m_LastNewlineInBuffer(0),
		m_CharInterval(160),
		m_LengthToNextSignal(m_CharInterval)
	{
		m_Buffer.reserve(m_BufferLength);
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
		Add(CL_StringHelp::ucs2_to_utf8(message));
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
			// headedMessage = message;
			break;
		case MTWARNING:
			headedMessage += GetWarningMarker() + " Warning:   " + message;
			break;
		case MTERROR:
			headedMessage += GetExceptionMarker() + " Exception: " + message;
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

	void Console::BindCommand(const std::string &command, Console::CommandCallback callback)
	{
		m_Commands[command] = callback;
	}

	static inline void addToken(StringVector &args, std::string &quote, bool inQuote, const std::string& token)
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
				Add( _where->second(args) );
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

	void Console::RegisterScriptElements(ScriptingEngine *manager)
	{
#ifndef FSN_DONT_USE_SCRIPTING
		int r;
		asIScriptEngine* engine = manager->GetEnginePtr();

		RegisterTypeNoHandle<Console>("Console", engine);

		r = engine->RegisterObjectMethod("Console",
			"void print(string &in)",
			asMETHODPR(Console, Print, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		r = engine->RegisterObjectMethod("Console",
			"void print(string &heading, string &in)",
			asMETHODPR(Console, Print, (const std::string&, const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		
		r = engine->RegisterObjectMethod("Console",
			"void println(string &in)",
			asMETHODPR(Console, PrintLn, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		r = engine->RegisterObjectMethod("Console",
			"void println(string &heading, string &in)",
			asMETHODPR(Console, PrintLn, (const std::string&, const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		r = engine->RegisterObjectMethod("Console",
			"void println(int)",
			asMETHODPR(Console, PrintLn, (int), void),
			asCALL_THISCALL); FSN_ASSERT(r);
		r = engine->RegisterObjectMethod(
			"Console",
			"void println(double)",
			asMETHODPR(Console, PrintLn, (double), void),
			asCALL_THISCALL); FSN_ASSERT(r);

		r = engine->RegisterObjectMethod("Console",
			"void interpret(string &in)",
			asMETHODPR(Console, Interpret, (const std::string&), void),
			asCALL_THISCALL); FSN_ASSERT(r);

		r = engine->RegisterObjectMethod("Console", "void clear()", asMETHOD(Console, Clear), asCALL_THISCALL);
		FSN_ASSERT(r);

		manager->RegisterGlobalObject("Console console", this);
#endif
	}

}