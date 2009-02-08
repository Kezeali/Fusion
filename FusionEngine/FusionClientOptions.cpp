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

#include "Common.h"

#include "FusionClientOptions.h"

#include "FusionVirtualFileSource_PhysFS.h"

namespace FusionEngine
{

	ClientOptions::ClientOptions()
		: m_NumLocalPlayers(0)
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
		//m_PlayerOptions.resize(g_MaxLocalPlayers);
		m_PlayerVariables.resize(g_MaxLocalPlayers);
	}

	ClientOptions::ClientOptions(const std::string &filename)
		: m_NumLocalPlayers(0)
	{
		//m_PlayerOptions.resize(g_MaxLocalPlayers);
		m_PlayerVariables.resize(g_MaxLocalPlayers);

		if (!LoadFromFile(filename))
			SaveToFile(filename);
	}

	ClientOptions::~ClientOptions()
	{
		m_Mutex.lock();
		//m_PlayerOptions.clear();
		m_Controls.clear();
		m_Mutex.unlock();
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
		m_Mutex.lock();

		//ResourcePointer<TiXmlDocument> docResource = ResourceManager::getSingleton().OpenOrCreateResource<TiXmlDocument>(filename);
		ticpp::Document doc;

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
				playerAttribute = CL_StringHelp::int_to_local8(i);
			player.SetAttribute("player", playerAttribute.c_str());

			insertVarMapIntoDOM(player, m_PlayerVariables[i]);
		}

		//doc.SaveFile(filename);

		// Write file
		try
		{
			// Initialize a vdir
			CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
			CL_IODevice in = vdir.open_file(fe_widen(filename), CL_File::create_always, CL_File::access_write);

			in.write(doc.Value().c_str(), doc.Value().length());
		}
		catch (CL_Exception&)
		{
			//FSN_WEXCEPT(ExCode::IO, L"ClientOptions::SaveToFile", L"'" + filename + L"' could not be saved");
			return false;
		}

		m_Mutex.unlock();
		return true;
	}

	bool ClientOptions::LoadFromFile(const std::string &filename)
	{
		CL_MutexSection mutexSection(&m_Mutex);
		try
		{
			ticpp::Document doc;

			// Read file
			try
			{
				// Initialize a vdir
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				CL_IODevice in = vdir.open_file(fe_widen(filename), CL_File::open_existing, CL_File::access_read);

				char filedata[2084];
				in.read(&filedata, in.get_size());

				doc.Parse(std::string(filedata), true, TIXML_ENCODING_UTF8);
			}
			catch (CL_Exception&)
			{
				//FSN_WEXCEPT(ExCode::IO, L"ClientOptions::SaveToFile", L"'" + filename + L"' could not be saved");
				return false;
			}

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
				}
				else if (child->Value() == "playeroptions")
				{
					loadPlayerOptions(*child.Get());
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

	bool ClientOptions::SetOption(const std::string &name, const std::string &value)
	{
		m_Mutex.lock();
		m_Variables[name] = value;
		m_Mutex.unlock();
		return true;
	}

	void ClientOptions::SetMultipleOptions(const std::tr1::unordered_map<std::string, std::string> &pairs)
	{
		m_Mutex.lock();
		m_Variables.insert(pairs.begin(), pairs.end());
		m_Mutex.unlock();
	}

	bool ClientOptions::SetPlayerOption(int player, const std::string &name, const std::string &value)
	{
		CL_MutexSection mutex_lock(&m_Mutex);
		if (player >= m_NumLocalPlayers)
			return false;
		m_PlayerVariables[player][name] = value;
		return true;
	}

	bool ClientOptions::GetOption(const std::string &name, std::string *ret) const
	{
		CL_MutexSection mutex_lock(&m_Mutex);
		VarMap::iterator itVar = m_Variables.find(name);
		if (itVar == m_Variables.end())
			return false;

		ret->assign(itVar->second);
		return true;
	}

	// Could cache these conversions, but options are retrieved so infrequently
	//  that that would be a waste of effort
	bool ClientOptions::GetOption(const std::string &name, int *ret) const
	{
		CL_MutexSection mutex_lock(&m_Mutex);
		VarMap::iterator itVar = m_Variables.find(name);
		if (itVar == m_Variables.end())
			return false;

		*ret = CL_StringHelp::local8_to_int(itVar->second);
		return true;
	}

	std::string ClientOptions::GetOption_str(const std::string &name) const
	{
		CL_MutexSection mutex_lock(&m_Mutex);
		return m_Variables[name];
	}

	bool ClientOptions::GetOption_bool(const std::string &name) const
	{
		m_Mutex.lock();
		std::string value = m_Variables[name];
		m_Mutex.unlock();
		fe_tolower(value);
		return (value == "1" || value == "t" || value == "true");
	}

	bool ClientOptions::GetPlayerOption(int player, const std::string &name, std::string *value) const
	{
		CL_MutexSection mutex_lock(&m_Mutex);
		if (player >= m_NumLocalPlayers)
			return false;

		const VarMap &playerVars = m_PlayerVariables[player];

		VarMap::const_iterator _where = playerVars.find(name);
		if (_where == playerVars.end())
			return false;

		value->assign(_where->second);
		return true;
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
		if (!CL_StringHelp::compare(opts_root.Value(), "playeroptions", true))
			return;

		std::string player = opts_root.GetAttribute("player");
		unsigned int playerNum = 0;
		if (!CL_StringHelp::compare(player, "default", true))
		{
			if (!fe_issimplenumeric(player))
				return;

			playerNum = CL_StringHelp::local8_to_int(player);
		}

		if (playerNum > m_NumLocalPlayers || playerNum > m_PlayerVariables.size())
			return;

		VarMap playerVars = m_PlayerVariables[playerNum];

		ticpp::Iterator< ticpp::Element > child;
		for ( child = child.begin( &opts_root ); child != child.end(); child++ )
		{
			if (CL_StringHelp::compare(child->Value(), "var", true))
			{
				std::string name = child->GetAttribute("name");
				if (name.empty()) continue;

				std::string value = child->GetAttribute("value");
				playerVars[name] = value;
			}
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
