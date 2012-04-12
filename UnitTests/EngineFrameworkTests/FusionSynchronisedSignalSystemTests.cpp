#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionSynchronisedSignalingSystem.h"
#include "FusionPropertySyncSigDetail.h"
#include <string>

#include <gtest/gtest.h>
#include <boost/crc.hpp>

using namespace FusionEngine;
using namespace FusionEngine::SyncSig;

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

static int next_prop_id;

typedef SynchronisedSignalingSystem<int, PropertyCallback> PropertySignalingSystem_t;

template <class T>
struct Prop : public IProp
{
	Prop() : id(++next_prop_id) {}

	void Set(T v) { value = v; event_trigger(); }
	const T& Get() const { return value; }

	void AquireTrigger(PropertySignalingSystem_t& sigsys)
	{
		event_trigger = sigsys.MakeGenerator<const T&>(id, std::bind(&Prop::Get, this));
	}

	typename PropertySignalingSystem_t::GeneratorDetail_t::Impl<const T&>::GeneratorFn_t event_trigger;

	T value;

	int id;
};

class NoCopy
{
public:
	NoCopy() : value(0) {}
	explicit NoCopy(int v) : value(v) {}
	NoCopy(NoCopy&& other) : value(other.value) {}
	NoCopy& operator= (NoCopy&& other) { value = other.value; return *this; }
	int value;
private:
	NoCopy(const NoCopy&){}
};

struct S
{
	Prop<NoCopy> prop1;
	Prop<bool> prop2;
};

TEST(synchsig, callbackGenerator)
{
	next_prop_id = 0;

	PropertySignalingSystem_t propsigsys;

	S s;
	s.prop1.AquireTrigger(propsigsys);
	ASSERT_TRUE( s.prop1.event_trigger );
	s.prop2.AquireTrigger(propsigsys);
	ASSERT_TRUE( s.prop2.event_trigger );

	int testoutput = 0;

	boost::signals2::connection connection;
	connection = propsigsys.AddHandler<const NoCopy&>(s.prop1.id, [&](const NoCopy& v){ testoutput = v.value; });
	ASSERT_TRUE( connection.connected() );

	s.prop1.Set(NoCopy(2409));
	propsigsys.Run();
	ASSERT_EQ(2409, testoutput);

	connection.disconnect();
}

//TEST(synchsig, propKeys)
//{
//	SynchronisedSignalingSystem<IProp*> propsigsys;
//
//	S s;
//	s.prop1.generator = propsigsys.MakeGenerator<int>(&s.prop1);
//	ASSERT_TRUE( s.prop1.generator );
//	s.prop2.generator = propsigsys.MakeGenerator<bool>(&s.prop2);
//	ASSERT_TRUE( s.prop2.generator );
//
//	int testoutput = 0;
//
//	boost::signals2::connection connection;
//	connection = propsigsys.AddHandler<int>(&s.prop1, [&](int v){ ASSERT_EQ(2409, v); testoutput = v; });
//	ASSERT_TRUE( connection.connected() );
//
//	s.prop1.Set(2409);
//	propsigsys.Run();
//	ASSERT_EQ(2409, testoutput);
//
//	connection.disconnect();
//}

