
#include "FusionServerOptions.h"

namespace FusionEngine
{

	ServerOptions::ServerOptions()
		: mMaxClients(32),
		mNetDelay(0),
		mBitmaskResolution(100),
		mMaxHealth(100)
	{
	}

	ServerOptions::ServerOptions(const std::string &filename)
	{
		ServerOptions();

		if (!LoadFromFile(filename))
			SaveToFile(filename);
	}

	bool ServerOptions::Save()
	{
		return true;
	}

	bool ServerOptions::SaveToFile(const std::string &filename)
	{
		return true;
	}

	bool ServerOptions::LoadFromFile(const std::string &filename)
	{
		return true;
	}

}
