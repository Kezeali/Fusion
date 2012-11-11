#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionArchetypeFactory.h"
#include "FusionArchetype.h"
#include "FusionEngineManager.h"
#include "FusionEntitySerialisationUtils.h"
#include "FusionEntity.h"
#include "FusionP2PEntityInstantiator.h"
#include "FusionPhysFSIOStream.h"

#include <gtest/gtest.h>
#include <boost/crc.hpp>

using namespace FusionEngine;

struct archetype_f : public testing::Test
{
	archetype_f()
	{
	}

	virtual void SetUp()
	{
		manager.reset(new EngineManager(args));
		ASSERT_NO_THROW(factory.reset(new ArchetypeFactory()));
	}
	virtual void TearDown()
	{
		ASSERT_NO_FATAL_FAILURE(factory.reset());
		manager.reset();
	}

	std::vector<CL_String> args;
	std::unique_ptr<EngineManager> manager;
	std::unique_ptr<ArchetypeFactory> factory;
};

//TEST_F(archetype_f, createInstance)
//{
//	IO::PhysFSStream input("test.archetype", IO::Read);
//	auto archetype = factory->GetArchetype("test");
//	archetype->Load(input);
//
//	EntityPtr entity;
//	// TODO instantiate an empty entity
//
//	factory->MakeInstance(entity, "test", Vector2(), 0.f);
//}
//
//TEST_F(archetype_f, saveArchetype)
//{
//	EntityPtr entity;
//	// TODO load entity from existing data
//
//	factory->DefineArchetypeFromEntity("test", entity);
//
//	IO::PhysFSStream output("saved.archetype", IO::Write);
//	auto archetype = factory->GetArchetype("test");
//	archetype->Save(output);
//}
//
//TEST_F(archetype_f, defineArchetype)
//{
//	EntityPtr definitionEntity, instanceEntity;
//
//	// TODO load definition from existing data
//
//	ASSERT_TRUE(definitionEntity);
//	ASSERT_TRUE(instanceEntity);
//
//	factory->DefineArchetypeFromEntity("test", definitionEntity);
//
//	factory->MakeInstance(instanceEntity, "test", Vector2(), 0.f);
//
//	// Compare number and type of components
//	auto defComs = definitionEntity->GetComponents();
//	auto insComs = instanceEntity->GetComponents();
//	ASSERT_EQ(defComs.size(), insComs.size());
//	for (auto def_it = defComs.begin(), ins_it = insComs.begin(); def_it != defComs.end(); ++def_it, ++ins_it)
//	{
//		auto defCom = *def_it; auto insCom = *ins_it;
//		ASSERT_EQ(defCom->GetType(), insCom->GetType());
//		ASSERT_EQ(defCom->GetIdentifier(), insCom->GetIdentifier());
//	}
//
//	// Serialise entities so the resulting data can be compared
//	std::stringstream defOutput;
//	EntitySerialisationUtils::SaveEntity(defOutput, definitionEntity, false, false);
//	std::stringstream insOutput;
//	EntitySerialisationUtils::SaveEntity(insOutput, instanceEntity, false, false);
//
//	// Compare serialised data
//	{
//		boost::crc_32_type crc;
//		std::array<char, 4096> data;
//
//			while (defOutput.good())
//			{
//				defOutput.read(data.data(), data.size());
//				crc.process_bytes(data.data(), (size_t)defOutput.gcount());
//			}
//
//		uint32_t expectedChecksum = crc.checksum();
//
//		crc.reset();
//
//		while (insOutput.good())
//		{
//			insOutput.read(data.data(), data.size());
//			crc.process_bytes(data.data(), (size_t)insOutput.gcount());
//		}
//
//		uint32_t actualChecksum = crc.checksum();
//
//		ASSERT_EQ(expectedChecksum, actualChecksum);
//	}
//}
