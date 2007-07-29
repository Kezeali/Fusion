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
		: NumPlayers(0)
	{
		// Set the global controls to some valid values.
		DefaultGlobalControls();

		PlayerInputs.resize(g_MaxLocalPlayers);
		// Do the same for all the players
		for (ObjectID i=0; i<g_MaxLocalPlayers; i++)
		{
			DefaultPlayerControls(i);
		}

		// Make sure there's enough room for all the player options objects
		PlayerOptions.resize(g_MaxLocalPlayers);
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
			PlayerInputs[0].thrust = CL_KEY_UP;
			PlayerInputs[0].reverse = CL_KEY_DOWN;
			PlayerInputs[0].left = CL_KEY_LEFT;
			PlayerInputs[0].right = CL_KEY_RIGHT;
			PlayerInputs[0].primary = CL_KEY_DIVIDE;
			PlayerInputs[0].secondary = CL_KEY_PERIOD;
			PlayerInputs[0].bomb = ',';
			break;
			// Player 2
		case 1:
			PlayerInputs[1].thrust = CL_KEY_W;
			PlayerInputs[1].reverse = CL_KEY_S;
			PlayerInputs[1].left = CL_KEY_A;
			PlayerInputs[1].right = CL_KEY_D;
			PlayerInputs[1].primary = CL_KEY_Q;
			PlayerInputs[1].secondary = CL_KEY_E;
			PlayerInputs[1].bomb = CL_KEY_R;
			break;
			// Player 3
		case 2:
			PlayerInputs[2].thrust = CL_KEY_U;
			PlayerInputs[2].reverse = CL_KEY_J;
			PlayerInputs[2].left = CL_KEY_H;
			PlayerInputs[2].right = CL_KEY_K;
			PlayerInputs[2].primary = CL_KEY_Y;
			PlayerInputs[2].secondary = CL_KEY_I;
			PlayerInputs[2].bomb = CL_KEY_B;
			break;
			// Player 4
		case 3:
			PlayerInputs[3].thrust = CL_KEY_NUMPAD8;
			PlayerInputs[3].reverse = CL_KEY_NUMPAD5;
			PlayerInputs[3].left = CL_KEY_NUMPAD4;
			PlayerInputs[3].right = CL_KEY_NUMPAD6;
			PlayerInputs[3].primary = CL_KEY_NUMPAD7;
			PlayerInputs[3].secondary = CL_KEY_NUMPAD9;
			PlayerInputs[3].bomb = CL_KEY_NUMPAD0;
			break;
			// Greater than 4 players (just in case)
		default:
			PlayerInputs[player].thrust = CL_KEY_W;
			PlayerInputs[player].reverse = CL_KEY_S;
			PlayerInputs[player].left = CL_KEY_A;
			PlayerInputs[player].right = CL_KEY_D;
			PlayerInputs[player].primary = CL_KEY_Q;
			PlayerInputs[player].secondary = CL_KEY_E;
			PlayerInputs[player].bomb = CL_KEY_R;
			break;
		}

	}

	void ClientOptions::DefaultGlobalControls()
	{
		GlobalInputs.menu = CL_KEY_ESCAPE;
		GlobalInputs.console = '`';
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
		//TiXmlDocument doc;

		//// Decl
		//TiXmlDeclaration* decl = new TiXmlDeclaration( XML_STANDARD, "", "" );  
		//doc.LinkEndChild( decl ); 

		//// Root
		//TiXmlElement * root = new TiXmlElement("ClientOptions");  
		//doc.LinkEndChild( root );  

		//// block: player options
		//{
		//	TiXmlElement* players = new TiXmlElement( "PlayerOptions" );  
		//	root->LinkEndChild( players ); 

		//	TiXmlElement* opt;

		//	PlayerOptionsList::iterator it;

		//	for (it=PlayerOptions.begin(); it != PlayerOptions.end(); ++it)
		//	{
		//		//??? should mPlayerOptions be a map? or should we iteratre by index (c-style) here?
		//		const std::string& key = (*it).first;
		//		const std::string& value = (*it).second;

		//		opt = new TiXmlElement();  
		//		opt->LinkEndChild( new TiXmlText(value.c_str()));  

		//		players->LinkEndChild( opt );
		//	}
		//}

		//// block: windows
		//{
		//	TiXmlElement* windowsNode = new TiXmlElement( "Windows" );  
		//	root->LinkEndChild( windowsNode );  

		//	list<WindowSettings>::iterator iter;

		//	for (iter=m_windows.begin(); iter != m_windows.end(); iter++)
		//	{
		//		const WindowSettings& w=*iter;

		//		TiXmlElement* window;
		//		window = new TiXmlElement( "Window" );  
		//		windowsNode->LinkEndChild( window );  
		//		window->SetAttribute("name", w.name.c_str());
		//		window->SetAttribute("x", w.x);
		//		window->SetAttribute("y", w.y);
		//		window->SetAttribute("w", w.w);
		//		window->SetAttribute("h", w.h);
		//	}
		//}

		//// block: connection
		//{
		//	TiXmlElement * cxn = new TiXmlElement( "Connection" );  
		//	root->LinkEndChild( cxn );  
		//	cxn->SetAttribute("ip", m_connection.ip.c_str());
		//	cxn->SetDoubleAttribute("timeout", m_connection.timeout); 
		//}

		//doc.SaveFile(pFilename);  

		return true;
	}

	bool ClientOptions::LoadFromFile(const std::string &filename)
	{
		return true;
	}

}
