#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionSynchronisedSignalingSystem.h"
#include <string>

#include <gtest/gtest.h>
#include <boost/crc.hpp>

using namespace FusionEngine;

struct synchsig_f : public testing::Test
{
	synchsig_f()
	{
	}

	virtual void SetUp()
	{
	}
	virtual void TearDown()
	{
	}

	int testoutput;

	SynchronisedSignalingSystem<std::string> sigsys;
};

TEST_F(synchsig_f, testSignals)
{
	const std::string handlerKey = "test_signal";
	auto generatorCallback = sigsys.MakeGenerator<int>(handlerKey);

	testoutput = 0;

	sigsys.AddHandler<int>(handlerKey, ([this](int v){ ASSERT_EQ(2409, v); testoutput = v; }));

	generatorCallback(2409);

	sigsys.Run();

	ASSERT_EQ(2409, testoutput);
}
