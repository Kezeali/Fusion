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

TEST_F(synchsig_f, addRemoveGenerator)
{
	const std::string handlerKey = "test_signal";

	HandlerConnection_t connection;
	auto handlerFn = [&](int v){ ASSERT_EQ(2409, v); };

	{
		auto generatorCallback = sigsys->MakeGenerator<int>(handlerKey);
		ASSERT_TRUE( generatorCallback );

		ASSERT_NO_THROW( connection = sigsys->AddHandler<int>(handlerKey, handlerFn) );
		ASSERT_TRUE( connection->connected() );
	} // generatorCallback should be destroyed here, which should result in the generator being removed

	ASSERT_THROW( connection = sigsys->AddHandler<int>(handlerKey, handlerFn), InvalidArgumentException );
	ASSERT_FALSE( connection->connected() );
}

TEST_F(synchsig_f, basicHandlers)
{
	const std::string handlerKey = "test_signal";
	auto generatorCallback = sigsys->MakeGenerator<int>(handlerKey);
	ASSERT_TRUE( generatorCallback );

	int testoutput = 0;

	// This will throw if the handler type is not compatible with the generator type...
	HandlerConnection_t connection;
	connection = sigsys->AddHandler<int>(handlerKey, [&](int v){ ASSERT_EQ(2409, v); testoutput = v; });
	ASSERT_TRUE( connection->connected() );
	// ... like this:
	auto invalid_handler = [&](float v){};
	ASSERT_THROW( sigsys->AddHandler<float>(handlerKey, invalid_handler), InvalidArgumentException );

	generatorCallback(2409);
	sigsys->Fire();
	ASSERT_EQ(2409, testoutput);

	connection.reset();

	generatorCallback(1189);
	sigsys->Fire();
	ASSERT_EQ(2409, testoutput); // Handler has been disconnected, so the value shouldn't have changed
}

// This has to be done in a function call because macro parsing sucks.
HandlerConnection_t InvalidAdd(std::unique_ptr<SynchronisedSignalingSystem<std::string>>& sigsys, std::string handlerKey)
{
	return sigsys->AddHandler< std::tuple<int, float> >(handlerKey, [&](std::tuple<int, float> v){});
}

TEST_F(synchsig_f, multiArgEvents)
{
	const std::string handlerKey = "test_signal";
	auto generatorCallback = sigsys->MakeGenerator<std::tuple<int, double>>(handlerKey);
	ASSERT_TRUE( generatorCallback );

	std::tuple<int, double> testoutput(0, 0.0);

	HandlerConnection_t connection;
	connection = sigsys->AddHandler<std::tuple<int, double>>(handlerKey, [&](std::tuple<int, double> v){ testoutput = v; });
	ASSERT_TRUE( connection->connected() );

	// See above for implementation of InvalidAdd
	ASSERT_THROW(connection = InvalidAdd(sigsys, handlerKey), InvalidArgumentException);

	generatorCallback(std::make_tuple(2409, 24.09));
	sigsys->Fire();
	ASSERT_EQ(2409, std::get<0>(testoutput));
	ASSERT_EQ(24.09, std::get<1>(testoutput));
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
	// TODO: make non-copyables work (ScriptUtils::Calling::Caller is incompatible with non-copyables)
//private:
	NoCopy(const NoCopy& other) : value(other.value) {}
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

	HandlerConnection_t connection;
	connection = propsigsys.AddExactHandler<const NoCopy&>(s.prop1.id, [&](const NoCopy& v){ testoutput = v.value; });
	ASSERT_TRUE( connection->connected() );

	s.prop1.Set(NoCopy(2409));
	propsigsys.Fire();
	ASSERT_EQ(2409, testoutput);
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
//	HandlerConnection_t connection;
//	connection = propsigsys.AddHandler<int>(&s.prop1, [&](int v){ ASSERT_EQ(2409, v); testoutput = v; });
//	ASSERT_TRUE( connection );
//
//	s.prop1.Set(2409);
//	propsigsys.Fire();
//	ASSERT_EQ(2409, testoutput);
//}

