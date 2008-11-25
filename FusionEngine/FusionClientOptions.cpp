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

#include "FusionClientOptions.h"

#include "FusionResourceManager.h"

#include "FusionXMLLoader.h"

namespace FusionEngine
{

	ClientOptions::ClientOptions()
		: m_NumLocalPlayers(0),
		m_Rate(100),
		m_LocalPort(1337)
	{
		//// Set the global controls to some valid values.
		//DefaultGlobalControls();

		//PlayerInputs.resize(g_MaxLocalPlayers);
		//// Do the same for all the players
		//for (ObjectID i=0; i<g_MaxLocalPlayers; i++)
		//{
		//	DefaultPlayerControls(i);
		//}

		// Make sure there's enough room for all the player options objects
		m_PlayerOptions.resize(g_MaxLocalPlayers);
		m_PlayerVariables.resize(g_MaxLocalPlayers);
	}

	ClientOptions::ClientOptions(const std::string &filename)
		: m_NumLocalPlayers(0),
		m_Rate(100),
		m_LocalPort(1337)
	{
		m_PlayerOptions.resize(g_MaxLocalPlayers);
		m_PlayerVariables.resize(g_MaxLocalPlayers);

		if (!LoadFromFile(filename))
			SaveToFile(filename);
	}

	ClientOptions::~ClientOptions()
	{
		m_PlayerOptions.clear();
		m_Controls.clear();
	}

	bool ClientOptions::Save()
	{
		if (m_LastFile.empty())
			return false;
		else
			return SaveToFile(m_LastFile);
	}

	bool ClientOptions::SaveToFile(const std::string &filename)
	{
		ResourcePointer<TiXmlDocument> docResource = ResourceManager::getSingleton().OpenOrCreateResource<TiXmlDocument>(filename);
		ticpp::Document doc(docResource.GetDataPtr());

		// Decl
		ticpp::Declaration decl( XML_STANDARD, "", "" );
		doc.LinkEndChild( &decl ); 

		// Root
		ticpp::Element root("clientoptions");
		doc.LinkEndChild( &root );

		insertVarMapIntoDOM(root, m_Variables);

		for (int i = 0; i <= m_NumLocalPlayers; ++i)
		{
			ticpp::Element player("playeroptions");
			root.LinkEndChild( &player ); 

			std::string playerAttribute;
			if (i == 0)
				playerAttribute = "default";
			else
				playerAttribute = CL_String::from_int(i);
			player.SetAttribute("player", playerAttribute.c_str());

			insertVarMapIntoDOM(player, m_PlayerVariables[i]);
		}

		//doc.SaveFile(filename);
		doc.SaveFile();

		return true;
	}

	bool ClientOptions::LoadFromFile(const std::string &filename)
	{
		ResourceManager *rm = ResourceManager::getSingletonPtr();
		if (rm == NULL)
			return false;

		try
		{
			ResourcePointer<TiXmlDocument> docResource = rm->GetResource<TiXmlDocument>(filename);
			if (!docResource.IsValid())
				throw FileNotFoundException("ClientOptions::LoadFromFile", "Couldn't open resource '" + filename + "'", __FILE__, __LINE__);

			ticpp::Document doc(docResource.GetDataPtr());
			ticpp::Element* pElem = doc.FirstChildElement();

			if (pElem->Value() != "clientoptions")
				throw FileTypeException("InputPluginLoader::LoadInputs", filename + " is not a client options file", __FILE__, __LINE__);

			ticpp::Iterator< ticpp::Element > child;
			for ( child = child.begin( pElem ); child != child.end(); child++ )
			{
				if (child->Value() == "var")
				{
					std::string name = child->GetAttribute("name");
					if (name.empty()) continue;
					std::string value = child->GetAttribute("value");
					m_Variables[name] = value;

					// Set legacy (hard-coded) options
					fe_tolower(value);
					if (name == "console_logging")
						m_ConsoleLogging = (value == "1" || value == "t" || value == "true");
					else if (name == "num_local_players")
						child->GetAttribute("value", &m_NumLocalPlayers);
					else if (name == "localport")
						child->GetAttribute("value", &m_LocalPort);
				}
				else if (child->Value() == "playeroptions")
				{
					loadPlayerOptions(*child.Get());
				}
				else if (child->Value() == "keys")
				{
					loadKeys(*child);
				}
			}
		}
		catch (ticpp::Exception &ex)
		{
			//SendToConsole("Failed to load input plugin: " + std::string(ex.what()));

			throw FileSystemException("ClientOptions::LoadFromFile", ex.what(), __FILE__, __LINE__);
		}

		return true;
	}

	bool ClientOptions::GetOption(const std::string &name, std::string *ret)
	{
		VarMap::iterator itVar = m_Variables.find(name);
		if (itVar == m_Variables.end())
			return false;

		ret->assign(itVar->second);
		return true;
	}

	// Could cache these conversions, but options are retrieved so infrequently
	//  that that would be a waste of effort
	bool ClientOptions::GetOption(const std::string &name, int *ret)
	{
		VarMap::iterator itVar = m_Variables.find(name);
		if (itVar == m_Variables.end())
			return false;

		*ret = CL_String::to_int(itVar->second);
		return true;
	}

	std::string ClientOptions::GetOption_str(const std::string &name)
	{
		return m_Variables[name];
	}

	bool ClientOptions::GetOption_bool(const std::string &name)
	{
		std::string value = m_Variables[name];
		fe_tolower(value);
		return (value == "1" || value == "t" || value == "true");
	}

	void ClientOptions::insertVarMapIntoDOM(ticpp::Element &parent, const VarMap &vars)
	{
		for (VarMap::const_iterator it = vars.begin(), end = vars.end(); it != end; ++it)
		{
			ticpp::Element var("var");
			var.SetAttribute("name", it->first.c_str());
			var.SetAttribute("value", it->second.c_str());
			parent.LinkEndChild( &var );
		}
	}

	void ClientOptions::loadPlayerOptions(const ticpp::Element &opts_root)
	{
		if (!CL_String::compare_nocase(opts_root.Value(), "playeroptions"))
			return;

		std::string player = opts_root.GetAttribute("player");
		unsigned int playerNum = 0;
		if (!CL_String::compare_nocase(player, "default"))
		{
			if (!fe_issimplenumeric(player))
				return;

			playerNum = CL_String::to_int(player);
		}

		if (playerNum > m_NumLocalPlayers || playerNum > m_PlayerVariables.size())
			return;

		VarMap playerVars = m_PlayerVariables[playerNum];

		ticpp::Iterator< ticpp::Element > child;
		for ( child = child.begin( &opts_root ); child != child.end(); child++ )
		{
			if (CL_String::compare_nocase(child->Value(), "var"))
			{
				std::string name = child->GetAttribute("name");
				if (name.empty()) continue;

				std::string value = child->GetAttribute("value");
				playerVars[name] = value;
			}
		}
	}

	void ClientOptions::loadKeys(const ticpp::Element &keysroot)
	{
		if (keysroot.Value() != "keys")
			return;

		std::string player = keysroot.GetAttribute("player");
		if (player == "default")
			player = "";

		ticpp::Iterator< ticpp::Element > child;
		ticpp::Iterator< ticpp::Element > end;
		for (child = child.begin( &keysroot ), end = child.end(); child != end; child++)
		{
			if (child->Value() != "bind")
				continue;

			std::string key = child->GetAttribute("key");
			std::string input = child->GetAttribute("input");

			m_Controls.push_back( InputBinding(input, key, player) );
		}
	}

	//void ClientOptions::DefaultPlayerControls(ObjectID player)
	//{
	//	switch (player)
	//	{
	//		// Player 1
	//	case 0:
	//		PlayerInputs[0].thrust = CL_KEY_UP;
	//		PlayerInputs[0].reverse = CL_KEY_DOWN;
	//		PlayerInputs[0].left = CL_KEY_LEFT;
	//		PlayerInputs[0].right = CL_KEY_RIGHT;
	//		PlayerInputs[0].primary = CL_KEY_DIVIDE;
	//		PlayerInputs[0].secondary = CL_KEY_PERIOD;
	//		PlayerInputs[0].bomb = ',';
	//		break;
	//		// Player 2
	//	case 1:
	//		PlayerInputs[1].thrust = CL_KEY_W;
	//		PlayerInputs[1].reverse = CL_KEY_S;
	//		PlayerInputs[1].left = CL_KEY_A;
	//		PlayerInputs[1].right = CL_KEY_D;
	//		PlayerInputs[1].primary = CL_KEY_Q;
	//		PlayerInputs[1].secondary = CL_KEY_E;
	//		PlayerInputs[1].bomb = CL_KEY_R;
	//		break;
	//		// Player 3
	//	case 2:
	//		PlayerInputs[2].thrust = CL_KEY_U;
	//		PlayerInputs[2].reverse = CL_KEY_J;
	//		PlayerInputs[2].left = CL_KEY_H;
	//		PlayerInputs[2].right = CL_KEY_K;
	//		PlayerInputs[2].primary = CL_KEY_Y;
	//		PlayerInputs[2].secondary = CL_KEY_I;
	//		PlayerInputs[2].bomb = CL_KEY_B;
	//		break;
	//		// Player 4
	//	case 3:
	//		PlayerInputs[3].thrust = CL_KEY_NUMPAD8;
	//		PlayerInputs[3].reverse = CL_KEY_NUMPAD5;
	//		PlayerInputs[3].left = CL_KEY_NUMPAD4;
	//		PlayerInputs[3].right = CL_KEY_NUMPAD6;
	//		PlayerInputs[3].primary = CL_KEY_NUMPAD7;
	//		PlayerInputs[3].secondary = CL_KEY_NUMPAD9;
	//		PlayerInputs[3].bomb = CL_KEY_NUMPAD0;
	//		break;
	//		// Greater than 4 players (just in case)
	//	default:
	//		PlayerInputs[player].thrust = CL_KEY_W;
	//		PlayerInputs[player].reverse = CL_KEY_S;
	//		PlayerInputs[player].left = CL_KEY_A;
	//		PlayerInputs[player].right = CL_KEY_D;
	//		PlayerInputs[player].primary = CL_KEY_Q;
	//		PlayerInputs[player].secondary = CL_KEY_E;
	//		PlayerInputs[player].bomb = CL_KEY_R;
	//		break;
	//	}

	//}

	//void ClientOptions::DefaultGlobalControls()
	//{
	//	GlobalInputs.menu = CL_KEY_ESCAPE;
	//	GlobalInputs.console = '`';
	//}

}
