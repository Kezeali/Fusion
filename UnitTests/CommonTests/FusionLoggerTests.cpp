#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionLogger.h"

#include <gtest/gtest.h>

using namespace FusionEngine;

struct logger_f : public testing::Test
{
	virtual void SetUp()
	{
		logger = std::make_shared<Logger>();
	}
	virtual void TearDown()
	{
	}

	std::shared_ptr<Logger> logger;
};

TEST_F(logger_f, initialisesLogsCorrectly)
{
	logger->SetDefaultThreshold(LOG_INFO);

	auto log1 = logger->GetLog("Log1", Logger::ReturnNull);
	ASSERT_FALSE((bool)log1);

	log1 = logger->GetLog("TestLog1");
	ASSERT_TRUE((bool)log1);

	ASSERT_EQ(LOG_INFO, log1->GetThreshold());

	// No (simple) way to test this...
	log1->AddEntry("Test Entry", LOG_INFO);
	log1->SetThreshold(LOG_NORMAL);
	log1->AddEntry("No output", LOG_INFO);

	// Assert that log1 == log2 (since they both have the same tag)
	auto log2 = logger->GetLog("TestLog1");
	ASSERT_TRUE((bool)log2);
	ASSERT_EQ(log1, log2);
	ASSERT_EQ(log2->GetThreshold(), LOG_NORMAL);

	logger->RemoveLog(log1);
	ASSERT_NO_FATAL_FAILURE(log1->AddEntry("Still available", LOG_NORMAL));
}

TEST_F(logger_f, setsLogThresholdCorrectly)
{
	logger->SetDefaultThreshold(LOG_INFO);
	ASSERT_EQ(logger->GetDefaultThreshold(), LOG_INFO);

	auto log1 = logger->GetLog("Log1");
	ASSERT_EQ(LOG_INFO, log1->GetThreshold());

	log1->SetThreshold(LOG_NORMAL);
	ASSERT_EQ(LOG_NORMAL, log1->GetThreshold());

	log1 = logger->GetLog("Log1", Logger::CreateIfNotExist); // Should just return the log as-is
	ASSERT_EQ(LOG_NORMAL, log1->GetThreshold());

	log1 = logger->GetLog("Log1", Logger::CreateIfNotExist, LOG_CRITICAL); // Should still return the log as-is
	ASSERT_EQ(LOG_NORMAL, log1->GetThreshold());

	log1 = logger->GetLog("Log1", Logger::ReplaceIfExist, LOG_DEFAULT); // Should reset the log to default threshold
	ASSERT_EQ(logger->GetDefaultThreshold(), log1->GetThreshold());

	log1 = logger->OpenLog("Log1", LOG_CRITICAL);
	ASSERT_EQ(LOG_CRITICAL, log1->GetThreshold());

	logger->RemoveLog(log1);
	log1 = logger->GetLog("Log1");
	ASSERT_EQ(logger->GetDefaultThreshold(), log1->GetThreshold());
}