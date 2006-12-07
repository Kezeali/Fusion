
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
		for (PlayerInd i=0; i<g_MaxPlayers; i++)
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

	void ClientOptions::DefaultPlayerControls(PlayerInd player)
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
		return true;
	}

	bool ClientOptions::SaveToFile(const std::string &filename)
	{
		CL_DomDocument doc;
		return true;
	}

	bool ClientOptions::LoadFromFile(const std::string &filename)
	{
		return true;
	}

}
