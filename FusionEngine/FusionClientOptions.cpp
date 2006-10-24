
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
		PlayerOptions[0].Inputs.thrust = CL_KEY_UP;
		PlayerOptions[0].Inputs.reverse = CL_KEY_DOWN;
		PlayerOptions[0].Inputs.left = CL_KEY_LEFT;
		PlayerOptions[0].Inputs.right = CL_KEY_RIGHT;
		PlayerOptions[0].Inputs.primary = CL_KEY_DIVIDE;
		PlayerOptions[0].Inputs.secondary = CL_KEY_DECIMAL;
		PlayerOptions[0].Inputs.bomb = ',';
		break;
		// Player 2
	case 1:
		PlayerOptions[1].Inputs.thrust = CL_KEY_W;
		PlayerOptions[1].Inputs.reverse = CL_KEY_S;
		PlayerOptions[1].Inputs.left = CL_KEY_A;
		PlayerOptions[1].Inputs.right = CL_KEY_D;
		PlayerOptions[1].Inputs.primary = CL_KEY_Q;
		PlayerOptions[1].Inputs.secondary = CL_KEY_E;
		PlayerOptions[1].Inputs.bomb = CL_KEY_R;
		break;
		// Player 3
	case 2:
		PlayerOptions[2].Inputs.thrust = CL_KEY_U;
		PlayerOptions[2].Inputs.reverse = CL_KEY_J;
		PlayerOptions[2].Inputs.left = CL_KEY_H;
		PlayerOptions[2].Inputs.right = CL_KEY_K;
		PlayerOptions[2].Inputs.primary = CL_KEY_Y;
		PlayerOptions[2].Inputs.secondary = CL_KEY_I;
		PlayerOptions[2].Inputs.bomb = CL_KEY_B;
		break;
		// Player 4
	case 3:
		PlayerOptions[3].Inputs.thrust = CL_KEY_NUMPAD8;
		PlayerOptions[3].Inputs.reverse = CL_KEY_NUMPAD5;
		PlayerOptions[3].Inputs.left = CL_KEY_NUMPAD4;
		PlayerOptions[3].Inputs.right = CL_KEY_NUMPAD6;
		PlayerOptions[3].Inputs.primary = CL_KEY_NUMPAD7;
		PlayerOptions[3].Inputs.secondary = CL_KEY_NUMPAD9;
		PlayerOptions[3].Inputs.bomb = CL_KEY_NUMPAD0;
		break;
		// Greater than 4 players (just in case)
	default:
		PlayerOptions[player].Inputs.thrust = CL_KEY_W;
		PlayerOptions[player].Inputs.reverse = CL_KEY_S;
		PlayerOptions[player].Inputs.left = CL_KEY_A;
		PlayerOptions[player].Inputs.right = CL_KEY_D;
		PlayerOptions[player].Inputs.primary = CL_KEY_Q;
		PlayerOptions[player].Inputs.secondary = CL_KEY_E;
		PlayerOptions[player].Inputs.bomb = CL_KEY_R;
		break;
	}

}

void ClientOptions::DefaultGlobalControls()
{
	GlobalInputs.menu = CL_KEY_ESCAPE;
	GlobalInputs.console = '`';
}
