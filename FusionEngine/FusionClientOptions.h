/*
  Copyright (c) 2006-2008 Fusion Project Team

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

#ifndef Header_FusionEngine_ClientOptions
#define Header_FusionEngine_ClientOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionSingleton.h"

namespace FusionEngine
{
	//! Max local (split-screen) players per client
	static const unsigned int g_MaxLocalPlayers = 16;

	/*!
	 * \brief
	 * Encapsulates client-side options.
	 *
	 * \remarks Threadsafe
	 */
	class ClientOptions : public Singleton<ClientOptions>
	{
	public:
		//! Constructor
		ClientOptions();
		//! Constructor +file
		ClientOptions(const std::wstring &filename);
		//! Clears controls
		~ClientOptions();

	public:
		//! Wrapper for singleton version
		static ClientOptions& current()
		{
			return getSingleton();
		}

	public:
		typedef std::tr1::unordered_map<std::string, std::string> VarMap;

		//typedef std::tr1::unordered_map<std::string, XmlInputBinding> ControlsList;
		//! Input mappings list.
		//typedef std::vector<XmlInputBinding> ControlsList;

		// Each player has their own var map
		typedef std::vector<VarMap> PlayerVarMapList;

	public:
		//! Number of local players
		unsigned int m_NumLocalPlayers;

		VarMap m_Variables;
		PlayerVarMapList m_PlayerVariables;

		//ControlsList m_Controls;

	public:
		////! Set the controls for defaults
		////! \todo Load default player controls from file (very low priority)
		//void DefaultPlayerControls(ObjectID player);
		////! Sets all the controls to the defaults
		//void DefaultGlobalControls();

		//! Saves to the most recently loaded file.
		bool Save();
		//! Saves the current options to a file
		bool SaveToFile(const std::wstring &filename);
		//! Loads a set of options from a file
		bool LoadFromFile(const std::wstring &filename);


		bool SetOption(const std::string& name, const std::string& value);
		void SetMultipleOptions(const std::tr1::unordered_map<std::string, std::string>& pairs);

		bool SetPlayerOption(int player, const std::string& name, const std::string& value);

		bool GetOption(const std::string &name, std::string *val) const;
		bool GetOption(const std::string &name, int *ret) const;
		std::string GetOption_str(const std::string &name) const;
		bool GetOption_bool(const std::string &name) const;

		bool GetPlayerOption(int player, const std::string& name, std::string *val) const;

	protected:
		mutable CL_Mutex m_Mutex;
		//! Last opened options file
		std::wstring m_LastFile;

		void insertVarMapIntoDOM(ticpp::Element* parent, const VarMap &vars);
		void loadPlayerOptions(const ticpp::Element *const opts_root);
		//void loadKeys(const ticpp::Element &keysroot);

	};

	//static unsigned int NumLocalPlayers()
	//{
	//	return ClientOptions::current().m_NumLocalPlayers;
	//}

	//static bool IsConsoleLogging()
	//{
	//	return false;//ClientOptions::current().m_ConsoleLogging;
	//}

	//static unsigned int Rate()
	//{
	//	return 0;//ClientOptions::current().m_Rate;
	//}

	//static unsigned int ClientPort()
	//{
	//	return 0;//ClientOptions::current().m_LocalPort;
	//}

	//static const std::string& PlayerName(unsigned int p)
	//{
	//	FSN_ASSERT(p < NumLocalPlayers());
	//	return "";//ClientOptions::current().m_PlayerOptions[p].m_Name;
	//}

	//static bool PlayerUseHud(unsigned int p)
	//{
	//	FSN_ASSERT(p < NumLocalPlayers());
	//	return false;//ClientOptions::current().m_PlayerOptions[p].m_HUD;
	//}

}

#endif
