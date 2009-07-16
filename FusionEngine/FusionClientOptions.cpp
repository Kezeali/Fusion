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

#include "FusionXML.h"

#include <boost/algorithm/string.hpp>

namespace FusionEngine
{

	ClientOptions::ClientOptions(const std::string &type)
		: m_Type(type),
		m_NumLocalPlayers(0)
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

	ClientOptions::ClientOptions(const std::wstring &filename, const std::string &type)
		: m_Type(type),
		m_NumLocalPlayers(0)
	{
		//m_PlayerOptions.resize(g_MaxLocalPlayers);
		m_PlayerVariables.resize(g_MaxLocalPlayers);

		if (!LoadFromFile(filename))
			SaveToFile(filename);
	}

	ClientOptions::~ClientOptions()
	{
	}

	bool ClientOptions::Save()
	{
		if (m_LastFile.empty())
			return false;
		else
			return SaveToFile(m_LastFile);
	}

	bool ClientOptions::SaveToFile(const std::wstring &filename)
	{
		m_Mutex.lock();

		ticpp::Document doc;

		// Decl
		ticpp::Declaration *decl = new ticpp::Declaration( XML_STANDARD, "", "" );
		doc.LinkEndChild( decl ); 

		// Root
		ticpp::Element* root = new ticpp::Element(m_Type);
		doc.LinkEndChild( root );

		insertVarMapIntoDOM(root, m_Variables);

		for (unsigned int i = 0; i <= m_NumLocalPlayers; ++i)
		{
			ticpp::Element* player = new ticpp::Element("playeroptions");
			root->LinkEndChild( player ); 

			std::string playerAttribute;
			if (i == 0)
				playerAttribute = "default";
			else
				playerAttribute = CL_StringHelp::int_to_local8(i);
			player->SetAttribute("player", playerAttribute.c_str());

			insertVarMapIntoDOM(player, m_PlayerVariables[i]);
		}

		//doc.SaveFile(filename);

		// Write file
		try
		{
			SaveString_PhysFS(doc.Value(), filename);
		}
		catch (CL_Exception&)
		{
			//FSN_WEXCEPT(ExCode::IO, L"ClientOptions::SaveToFile", L"'" + filename + L"' could not be saved");
			return false;
		}

		m_LastFile = filename;

		m_Mutex.unlock();
		return true;
	}

	bool ClientOptions::LoadFromFile(const std::wstring &filename)
	{
		CL_MutexSection mutexSection(&m_Mutex);
		try
		{
			// Load the document
			ticpp::Document doc(OpenXml_PhysFS(filename));

			ticpp::Element* pElem = doc.FirstChildElement();

			if (pElem->Value() != m_Type)
				FSN_WEXCEPT(ExCode::FileType, L"ClientOptions::LoadFromFile", filename + L" is not a " + fe_widen(m_Type) + L" file");

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
					loadPlayerOptions(child.Get());
				}
			}
		}
		catch (ticpp::Exception &ex)
		{
			//SendToConsole("Failed to load input plugin: " + std::string(ex.what()));

			FSN_EXCEPT(ExCode::IO, "ClientOptions::LoadFromFile", ex.what());
		}

		m_LastFile = filename;

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
		VarMap::const_iterator itVar = m_Variables.find(name);
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
		VarMap::const_iterator itVar = m_Variables.find(name);
		if (itVar == m_Variables.end())
			return false;

		*ret = CL_StringHelp::local8_to_int(itVar->second);
		return true;
	}

	std::string ClientOptions::GetOption_str(const std::string &name) const
	{
		CL_MutexSection mutex_lock(&m_Mutex);

		VarMap::const_iterator _where = m_Variables.find(name);
		if (_where == m_Variables.end())
			return "";
		return _where->second;
	}

	bool ClientOptions::GetOption_bool(const std::string &name) const
	{
		m_Mutex.lock();
		VarMap::const_iterator _where = m_Variables.find(name);
		std::string value = "";
		if (_where != m_Variables.end())
			value = _where->second;
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

	void ClientOptions::insertVarMapIntoDOM(ticpp::Element* parent, const VarMap &vars)
	{
		for (VarMap::const_iterator it = vars.begin(), end = vars.end(); it != end; ++it)
		{
			ticpp::Element* var = new ticpp::Element("var");
			var->SetAttribute("name", it->first.c_str());
			var->SetAttribute("value", it->second.c_str());
			parent->LinkEndChild( var );
		}
	}

	void ClientOptions::loadPlayerOptions(const ticpp::Element *const opts_root)
	{
		if (!fe_nocase_strcmp(opts_root->Value().c_str(), "playeroptions"))
			return;

		// Get the player number for this group (note that 'default' -> zero)
		std::string player = opts_root->GetAttribute("player");
		unsigned int playerNum = 0;
		if (!fe_nocase_strcmp(player.c_str(), "default"))
		{
			if (!fe_issimplenumeric(player))
				return;

			playerNum = CL_StringHelp::local8_to_int(player);
		}

		if (playerNum > m_NumLocalPlayers || playerNum > m_PlayerVariables.size())
			return;

		VarMap playerVars = m_PlayerVariables[playerNum];

		ticpp::Iterator< ticpp::Element > child;
		for ( child = child.begin( opts_root ); child != child.end(); child++ )
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

}
