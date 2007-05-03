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

namespace FusionEngine
{

	ClientOptions::ClientOptions()
		: mNumPlayers(0)
	{
		// Set the global controls to some valid values.
		DefaultGlobalControls();

		mPlayerInputs.resize(g_MaxPlayers);
		// Do the same for all the players
		for (ObjectID i=0; i<g_MaxPlayers; i++)
		{
			DefaultPlayerControls(i);
		}

		// Make sure there's enough room for all the player options objects
		mPlayerOptions.resize(g_MaxPlayers);
	}

	ClientOptions::ClientOptions(const std::string &filename)
	{
		ClientOptions();

		if (!LoadFromFile(filename))
			SaveToFile(filename);
	}

	void ClientOptions::DefaultPlayerControls(ObjectID player)
	{
		switch (player)
		{
			// Player 1
		case 0:
			mPlayerInputs[0].thrust = CL_KEY_UP;
			mPlayerInputs[0].reverse = CL_KEY_DOWN;
			mPlayerInputs[0].left = CL_KEY_LEFT;
			mPlayerInputs[0].right = CL_KEY_RIGHT;
			mPlayerInputs[0].primary = CL_KEY_DIVIDE;
			mPlayerInputs[0].secondary = CL_KEY_DECIMAL;
			mPlayerInputs[0].bomb = ',';
			break;
			// Player 2
		case 1:
			mPlayerInputs[1].thrust = CL_KEY_W;
			mPlayerInputs[1].reverse = CL_KEY_S;
			mPlayerInputs[1].left = CL_KEY_A;
			mPlayerInputs[1].right = CL_KEY_D;
			mPlayerInputs[1].primary = CL_KEY_Q;
			mPlayerInputs[1].secondary = CL_KEY_E;
			mPlayerInputs[1].bomb = CL_KEY_R;
			break;
			// Player 3
		case 2:
			mPlayerInputs[2].thrust = CL_KEY_U;
			mPlayerInputs[2].reverse = CL_KEY_J;
			mPlayerInputs[2].left = CL_KEY_H;
			mPlayerInputs[2].right = CL_KEY_K;
			mPlayerInputs[2].primary = CL_KEY_Y;
			mPlayerInputs[2].secondary = CL_KEY_I;
			mPlayerInputs[2].bomb = CL_KEY_B;
			break;
			// Player 4
		case 3:
			mPlayerInputs[3].thrust = CL_KEY_NUMPAD8;
			mPlayerInputs[3].reverse = CL_KEY_NUMPAD5;
			mPlayerInputs[3].left = CL_KEY_NUMPAD4;
			mPlayerInputs[3].right = CL_KEY_NUMPAD6;
			mPlayerInputs[3].primary = CL_KEY_NUMPAD7;
			mPlayerInputs[3].secondary = CL_KEY_NUMPAD9;
			mPlayerInputs[3].bomb = CL_KEY_NUMPAD0;
			break;
			// Greater than 4 players (just in case)
		default:
			mPlayerInputs[player].thrust = CL_KEY_W;
			mPlayerInputs[player].reverse = CL_KEY_S;
			mPlayerInputs[player].left = CL_KEY_A;
			mPlayerInputs[player].right = CL_KEY_D;
			mPlayerInputs[player].primary = CL_KEY_Q;
			mPlayerInputs[player].secondary = CL_KEY_E;
			mPlayerInputs[player].bomb = CL_KEY_R;
			break;
		}

	}

	void ClientOptions::DefaultGlobalControls()
	{
		mGlobalInputs.menu = CL_KEY_ESCAPE;
		mGlobalInputs.console = '`';
	}

	bool ClientOptions::Save()
	{
		if (m_LastFile.empty())
			return false;

		return true;
	}

	bool ClientOptions::SaveToFile(const std::string &filename)
	{
		TiXMLDocument doc;

		// Decl
		TiXmlDeclaration* decl = new TiXmlDeclaration( XML_STANDARD, "", "" );  
		doc.LinkEndChild( decl ); 

		// Root
		TiXmlElement * root = new TiXmlElement("ClientOptions");  
		doc.LinkEndChild( root );  

		// block: player options
		{
			TiXmlElement* players = new TiXmlElement( "PlayerOptions" );  
			root->LinkEndChild( msgs ); 

			TiXmlElement* opt;

			PlayerOptionsList::iterator it;

			for (it=mPlayerOptions.begin(); it != mPlayerOptions.end(); ++it)
			{
				//??? should mPlayerOptions be a map? or should we iteratre by index (c-style) here?
				const std::string& key = (*it).first;
				const std::string& value = (*it).second;

				opt = new TiXmlElement();  
				opt->LinkEndChild( new TiXmlText(value.c_str()));  

				players->LinkEndChild( opt );
			}
		}

		// block: windows
		{
			TiXmlElement* windowsNode = new TiXmlElement( "Windows" );  
			root->LinkEndChild( windowsNode );  

			list<WindowSettings>::iterator iter;

			for (iter=m_windows.begin(); iter != m_windows.end(); iter++)
			{
				const WindowSettings& w=*iter;

				TiXmlElement* window;
				window = new TiXmlElement( "Window" );  
				windowsNode->LinkEndChild( window );  
				window->SetAttribute("name", w.name.c_str());
				window->SetAttribute("x", w.x);
				window->SetAttribute("y", w.y);
				window->SetAttribute("w", w.w);
				window->SetAttribute("h", w.h);
			}
		}

		// block: connection
		{
			TiXmlElement * cxn = new TiXmlElement( "Connection" );  
			root->LinkEndChild( cxn );  
			cxn->SetAttribute("ip", m_connection.ip.c_str());
			cxn->SetDoubleAttribute("timeout", m_connection.timeout); 
		}

		doc.SaveFile(pFilename);  

		return true;
	}

	bool ClientOptions::LoadFromFile(const std::string &filename)
	{
		return true;
	}

}
