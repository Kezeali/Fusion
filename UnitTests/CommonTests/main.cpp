#include "PrecompiledHeaders.h"

#include <ClanLib/core.h>
#include "FusionPrerequisites.h"

// Logging
#include "FusionLogger.h"
#include "FusionPaths.h"

// Filesystem
#include "FusionPhysFS.h"

#include <gtest/gtest.h>

using namespace FusionEngine;

struct CommonTestsEnv : public testing::Environment
{
	virtual void SetUp()
	{
		auto dataFolder = std::string("UnitTestData") + PHYSFS_getDirSeparator();
		auto writep = std::string(CL_System::get_exe_path()) + dataFolder + "write_dir";

		SetupPhysFS::init(CL_System::get_exe_path().c_str());
		ASSERT_TRUE(PHYSFS_setWriteDir(writep.c_str()) != 0);
		ASSERT_TRUE(PHYSFS_mkdir(s_LogfilePath.c_str()) != 0);
		ASSERT_TRUE(SetupPhysFS::mount(dataFolder, "", "zip"));
		SetupPhysFS::mount_archives(dataFolder, "", "7z");
	}
	virtual void TearDown()
	{
		ASSERT_NO_FATAL_FAILURE(SetupPhysFS::deinit());
	}
};

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);

	CL_SetupCore core;

	testing::AddGlobalTestEnvironment(new CommonTestsEnv);

	return RUN_ALL_TESTS();
}
