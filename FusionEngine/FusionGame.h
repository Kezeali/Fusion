#ifndef Header_FusionEngine_FusionGame
#define Header_FusionEngine_FusionGame

#if _MSC_VER > 1000
#pragma once
#endif

/// Fusion
#include "FusionClientOptions.h"
#include "FusionServerOptions.h"

namespace FusionEngine
{

	class FusionGame
	{
	public:
		FusionGame() {}

		void RunClient(const std::string hostname, const std::string port, ClientOptions *options);
		void RunServer(const std::string port, ServerOptions options);

	private:

	};

}

#endif
