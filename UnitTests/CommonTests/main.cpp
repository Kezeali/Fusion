#include "PrecompiledHeaders.h"

#include <ClanLib/core.h>
#include "FusionPrerequisites.h"

// Logging
#include "FusionLogger.h"

// Filesystem
#include "FusionPhysFS.h"

#include <gtest/gtest.h>

using namespace FusionEngine;

struct CommonTestsEnv : public testing::Environment
{
	CommonTestsEnv(std::string path)
		: p(path)
	{}

	virtual void SetUp()
	{
		SetupPhysFS::init(CL_System::get_exe_path().c_str());
		PHYSFS_setWriteDir(p.c_str());
	}
	virtual void TearDown()
	{
		SetupPhysFS::deinit();
	}

	std::string p;
};

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	testing::AddGlobalTestEnvironment(new CommonTestsEnv(CL_System::get_exe_path()));
	return RUN_ALL_TESTS();
}
