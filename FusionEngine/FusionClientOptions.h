#ifndef Header_FusionEngine_ClientOptions
#define Header_FusionEngine_ClientOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionInputMap.h"

namespace FusionEngine
{
	const unsigned int g_MaxPlayers = 4;

	//! Settings for network related stuff.
	class NetworkSettings
	{
	public:
		NetworkSettings();

		unsigned int MaximumRate;
	};

	/*!
	 * Encapsulates client-side options.
	 */
	class ClientOptions
	{
	public:
		ClientOptions();

		unsigned int NumPlayers;

		typedef std::map<int, PlayerInputMap> PlayerInputMapContainer;

		PlayerInputMapContainer PlayerInputs;
		GlobalInputMap GlobalInputs;

		NetworkSettings NetworkOptions;

		void DefaultPlayerControls(unsigned int player);
		void DefaultGlobalControls();
	};

}

#endif
