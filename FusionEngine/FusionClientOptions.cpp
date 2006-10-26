
#include "FusionClientOptions.h"

using namespace FusionEngine;

NetworkSettings::NetworkSettings()
: MaxMessageRate(100)
{
}

ClientOptions::ClientOptions()
: NumPlayers(0)
{
	DefaultGlobalControls();
	for (PlayerInd i=0; i<g_MaxPlayers; i++)
	{
		DefaultPlayerControls(i);
	}
}

void ClientOptions::DefaultPlayerControls(PlayerInd player)
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
		PlayerInputs[0].secondary = CL_KEY_DECIMAL;
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
