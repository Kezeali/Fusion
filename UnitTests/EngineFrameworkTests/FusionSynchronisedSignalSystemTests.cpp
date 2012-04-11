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
		sigsys.reset(new SynchronisedSignalingSystem<std::string>);
	}
	virtual void TearDown()
	{
		sigsys.reset();
	}

	std::unique_ptr<SynchronisedSignalingSystem<std::string>> sigsys;
};

TEST_F(synchsig_f, testSignals)
{
	const std::string handlerKey = "test_signal";
	auto generatorCallback = sigsys->MakeGenerator<int>(handlerKey);
	ASSERT_TRUE( generatorCallback );

	int testoutput = 0;

	// This will throw if the handler type is not compatible with the generator type
	boost::signals2::connection connection;
	connection = sigsys->AddHandler<int>(handlerKey, [&](int v){ ASSERT_EQ(2409, v); testoutput = v; });
	ASSERT_TRUE( connection.connected() );

	auto invalid_handler = [&](float v){};
	ASSERT_THROW( sigsys->AddHandler<float>(handlerKey, invalid_handler), InvalidArgumentException );

	generatorCallback(2409);
	sigsys->Run();
	ASSERT_EQ(2409, testoutput);

	connection.disconnect();

	generatorCallback(1189);
	sigsys->Run();
	ASSERT_EQ(2409, testoutput); // Handler has been disconnected, so the value shouldn't have changed
}

struct IProp
{
	virtual ~IProp() {}
};

template <class T>
struct Prop : public IProp
{
	Prop() {}

	void Set(T v) { value = v; generator(v); }
	T Get() const { return value; }

	std::function<void (T)> generator;

	T value;
};

struct S
{
	Prop<int> prop1;
	Prop<bool> prop2;
};

TEST(propKeys)
{
	SynchronisedSignalingSystem<IProp*> propsigsys;

	S s;
	s.prop1.generator = propsigsys.MakeGenerator<int>(&s.prop1);
	ASSERT_TRUE( s.prop1.generator );
	s.prop2.generator = propsigsys.MakeGenerator<bool>(&s.prop2);
	ASSERT_TRUE( s.prop2.generator );

	int testoutput = 0;

	boost::signals2::connection connection;
	connection = propsigsys.AddHandler<int>(&s.prop1, [&](int v){ ASSERT_EQ(2409, v); testoutput = v; });
	ASSERT_TRUE( connection.connected() );

	s.prop1.Set(2409);
	propsigsys.Run();
	ASSERT_EQ(2409, testoutput);

	connection.disconnect();
}

