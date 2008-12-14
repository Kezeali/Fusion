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
	static const unsigned int g_MaxLocalPlayers = 4;

	/*!
	 * \brief Input binding from XML
	 *
	 * \remarks
	 * It should be fine to define this here - it is only used by InputManager
	 * which #includes this file.
	 */
	class XmlInputBinding
	{
	public:
		std::string m_Player;
		std::string m_Input; // The 'agency' this control provides :P
		std::string m_Key; // The key on the keyboard / button on the controler

	public:
		XmlInputBinding(std::string input, std::string key, std::string player)
			: m_Input(input),
			m_Key(key),
			m_Player(player)
		{
		}
	};

	//! Settings specfic to each player
	class PlayerOptions
	{
	public:
		//! Constructor.
		PlayerOptions()
			: m_HUD(true)
		{}

		//! Display the HUD for this player
		bool m_HUD;

		std::string m_Name;
	};

	/*!
	 * \brief
	 * Encapsulates client-side options.
	 *
	 * \todo Make member variable identifiers consistant over all classes by prepending them with
	 * 'm_', even public members
	 */
	class ClientOptions : public Singleton<ClientOptions>
	{
	public:
		//! Constructor
		ClientOptions();
		//! Constructor +file
		ClientOptions(const std::string &filename);
		//! Clears controls
		~ClientOptions();

	public:
		//! Wrapper for singleton version
		static ClientOptions& current()
		{
			return getSingleton();
		}

	public:
		//! [depreciated] General Player options list
		typedef std::vector<PlayerOptions> PlayerOptionsList;
		//! Player Input mappings list.
		typedef std::vector<XmlInputBinding> ControlsList;

		typedef std::map<std::string, std::string> VarMap;

		// Each player has their own var map
		typedef std::vector<VarMap> PlayerVarMapList;

	public:
		//! [depreciated] True if console history should be logged.
		bool m_ConsoleLogging;

		//! [depreciated] General player options (other than inputs)
		PlayerOptionsList m_PlayerOptions;

		//! [depreciated] Maximum messages per second
		unsigned int m_Rate;

		//! [depreciated] The local port (to connect from)
		unsigned short m_LocalPort;

		//! Number of local players
		unsigned int m_NumLocalPlayers;

		//! Controls
		ControlsList m_Controls;

		VarMap m_Variables;
		PlayerVarMapList m_PlayerVariables;

		//! Player input mappings
		//PlayerInputsList PlayerInputs;
		//! Global input mappings
		//GlobalInputMap GlobalInputs;

	public:
		////! Set the controls for defaults
		////! \todo Load default player controls from file (very low priority)
		//void DefaultPlayerControls(ObjectID player);
		////! Sets all the controls to the defaults
		//void DefaultGlobalControls();

		//! Saves to the most recently loaded file.
		bool Save();
		//! Saves the current options to a file
		bool SaveToFile(const std::string &filename);
		//! Loads a set of options from a file
		bool LoadFromFile(const std::string &filename);


		bool GetOption(const std::string &name, std::string *val);
		bool GetOption(const std::string &name, int *ret);
		std::string GetOption_str(const std::string &name);
		bool GetOption_bool(const std::string &name);

		void insertVarMapIntoDOM(ticpp::Element &parent, const VarMap &vars);
		void loadPlayerOptions(const ticpp::Element &opts_root);
		void loadKeys(const ticpp::Element &keysroot);

	protected:
		//! Last opened options file
		std::string m_LastFile;

	};

	static unsigned int NumLocalPlayers()
	{
		return ClientOptions::current().m_NumLocalPlayers;
	}

	static bool IsConsoleLogging()
	{
		return ClientOptions::current().m_ConsoleLogging;
	}

	static unsigned int Rate()
	{
		return ClientOptions::current().m_Rate;
	}

	static unsigned int ClientPort()
	{
		return ClientOptions::current().m_LocalPort;
	}

	static const std::string& PlayerName(unsigned int p)
	{
		cl_assert(p < NumLocalPlayers());
		return ClientOptions::current().m_PlayerOptions[p].m_Name;
	}

	static bool PlayerUseHud(unsigned int p)
	{
		cl_assert(p < NumLocalPlayers());
		return ClientOptions::current().m_PlayerOptions[p].m_HUD;
	}

}

#endif
