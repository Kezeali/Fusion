
#include "FusionEngineCommon.h"

#include "FusionClientOptions.h"

using namespace FusionEngine;

NetworkSettings::NetworkSettings()
: MaximumRate(10000)
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
		PlayerOptions[0].input.thrust = CL_KEY_UP;
		PlayerOptions[0].input.reverse = CL_KEY_DOWN;
		PlayerOptions[0].input.left = CL_KEY_LEFT;
		PlayerOptions[0].input.right = CL_KEY_RIGHT;
		PlayerOptions[0].input.primary = CL_KEY_DIVIDE;
		PlayerOptions[0].input.secondary = CL_KEY_DECIMAL;
		PlayerOptions[0].input.bomb = ',';
		break;
		// Player 2
	case 1:
		PlayerOptions[1].input.thrust = CL_KEY_W;
		PlayerOptions[1].input.reverse = CL_KEY_S;
		PlayerOptions[1].input.left = CL_KEY_A;
		PlayerOptions[1].input.right = CL_KEY_D;
		PlayerOptions[1].input.primary = CL_KEY_Q;
		PlayerOptions[1].input.secondary = CL_KEY_E;
		PlayerOptions[1].input.bomb = CL_KEY_R;
		break;
		// Player 3
	case 2:
		PlayerOptions[2].input.thrust = CL_KEY_U;
		PlayerOptions[2].input.reverse = CL_KEY_J;
		PlayerOptions[2].input.left = CL_KEY_H;
		PlayerOptions[2].input.right = CL_KEY_K;
		PlayerOptions[2].input.primary = CL_KEY_Y;
		PlayerOptions[2].input.secondary = CL_KEY_I;
		PlayerOptions[2].input.bomb = CL_KEY_B;
		break;
		// Player 4
	case 3:
		PlayerOptions[3].input.thrust = CL_KEY_NUMPAD8;
		PlayerOptions[3].input.reverse = CL_KEY_NUMPAD5;
		PlayerOptions[3].input.left = CL_KEY_NUMPAD4;
		PlayerOptions[3].input.right = CL_KEY_NUMPAD6;
		PlayerOptions[3].input.primary = CL_KEY_NUMPAD7;
		PlayerOptions[3].input.secondary = CL_KEY_NUMPAD9;
		PlayerOptions[3].input.bomb = CL_KEY_NUMPAD0;
		break;
	}

}

void ClientOptions::DefaultGlobalControls()
{
	GlobalInputs.menu = CL_KEY_ESCAPE;
	GlobalInputs.console = '`';
}
