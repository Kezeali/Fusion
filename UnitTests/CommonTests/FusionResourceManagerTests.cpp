#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionResourceManager.h"

#include "FusionResourcePointer.h"
#include "FusionPhysFSIOStream.h"

#include "FusionLogger.h"
#include "FusionProfiling.h"

#include <gtest/gtest.h>

#include <boost/crc.hpp>

class resource_manager_f;

namespace FusionEngine { namespace Test
{
	void UnloadTextResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	void LoadTextResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	bool ResourceFileDateHasChanged(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
	bool TextContentHasChanged(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data);
}}
	
using namespace FusionEngine;

class resource_manager_f : public testing::Test
{
public:
	std::string GetFile(std::string path)
	{
		auto entry = files.find(path);
		if (entry != files.end())
			return *entry;
		FSN_EXCEPT(FileNotFoundException, "File not found");
	}
protected:
	resource_manager_f()
		: gc(),
		currentN(0)
	{}
	virtual ~resource_manager_f()
	{
		logger.reset();
	}

	virtual void SetUp()
	{
		logger = std::make_shared<Logger>();
		logger->SetDefaultThreshold(LOG_INFO);
		profiling = std::make_shared<Profiling>();
		manager = std::make_shared<ResourceManager>(gc);

		if (n.empty())
		{
			std::stringstream str;
			n.resize(1024);
			for (size_t i = 0; i < 1024; ++i)
			{
				str << i;
				n[i] = str.str();
				str.str("");

				files.insert("resource" + n[i] + ".txt");
			}
		}
	}
	virtual void TearDown()
	{
		ASSERT_NO_FATAL_FAILURE(resourcesBeingLoaded.clear());

		ASSERT_NO_FATAL_FAILURE(manager.reset());
	}

	void AddStandardLoaders()
	{
		ASSERT_NO_FATAL_FAILURE(manager->AddResourceLoader(
			ResourceLoader("TestResource",
			&FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource,
			&FusionEngine::Test::TextContentHasChanged, boost::any(this))
			));
	}

	void AttemptActualLoad(bool next = true)
	{
		if (next)
			++currentN;
		resourcesBeingLoaded.push_back(ResourceHelper::Load(manager, "resource" + n[currentN] + ".txt"));
	}

	void AttemptMissingLoad()
	{
		resourcesBeingLoaded.push_back(ResourceHelper::Load(manager, "nothing"));
	}

	void VerifyLoadedResources()
	{
		auto list = manager->ListLoadedResources();
		for (auto it = list.begin(), end = list.end(); it != end; ++it)
		{
			std::string path = *it;
			bool shouldBeLoaded = std::any_of(resourcesBeingLoaded.begin(), resourcesBeingLoaded.end(), [path](const std::shared_ptr<ResourceHelper>& loaded)->bool
			{
				return loaded->resource && loaded->resource->GetPath() == path;
			});
			ASSERT_TRUE(shouldBeLoaded);
		}
	}

	class ResourceHelper
	{
	public:
		ResourceHelper(std::string path) : pathSoYouKnowWhatBroke(path), reloaded(false) {}
		virtual ~ResourceHelper()
		{
			loadConnection.disconnect();
		}

		static std::shared_ptr<ResourceHelper> Load(const std::shared_ptr<ResourceManager>& manager, const std::string& path);

		bool ValidateReload(ResourceDataPtr res, ResourceContainer::HotReloadEvent ev)
		{
			if (ev == ResourceContainer::HotReloadEvent::Validate)
			{
				resource.reset();
			}
			else if (ev == ResourceContainer::HotReloadEvent::PostReload)
			{
				resource = res;
				reloaded = true;
			}
			lastReloadEvent = ev;
			return true;
		}

		std::string pathSoYouKnowWhatBroke;
		boost::signals2::connection loadConnection;
		ResourceDataPtr resource;
		ResourceContainer::HotReloadEvent lastReloadEvent;
		bool reloaded;
	};
	
	std::vector<std::shared_ptr<ResourceHelper>> resourcesBeingLoaded;

	std::vector<std::string> n;
	size_t currentN;

	std::set<std::string> files;

	std::shared_ptr<ResourceManager> manager;
	clan::GraphicContext gc;
	std::shared_ptr<Logger> logger;
	std::shared_ptr<Profiling> profiling;
};

std::shared_ptr<resource_manager_f::ResourceHelper> resource_manager_f::ResourceHelper::Load(const std::shared_ptr<ResourceManager>& manager, const std::string& path)
{
	auto helper = std::make_shared<ResourceHelper>(path);
	std::weak_ptr<ResourceHelper> helperRef = helper;
	helper->loadConnection = manager->GetResource("TestResource", path, [helperRef](ResourceDataPtr resource)
	{
		if (auto h = helperRef.lock())
		{
			using namespace std::placeholders;
			h->resource = resource;
			h->loadConnection = resource->SigHotReloadEvents.connect(std::bind(&resource_manager_f::ResourceHelper::ValidateReload, h.get(), _1, _2));
		}
	});
	return helper;
}

namespace FusionEngine { namespace Test
{
	template <typename T>
	T processStream(IO::PhysFSStream& stream, std::function<T (const std::array<char, 4096>&, size_t)> func, T initial)
	{
		T result = initial;

		std::array<char, 4096> buffer;
		while (!stream.eof())
		{
			stream.read(buffer.data(), 4096);
			auto count = stream.gcount();

			if (count > 0)
			{
				result += func(buffer, (size_t)count);
			}
		}
		return result;
	}

	std::int64_t checksumStream(IO::PhysFSStream& stream)
	{
		boost::crc_32_type checksummer;

		processStream<int>(stream, [&checksummer](const std::array<char, 4096>& buffer, size_t length)->int
		{
			checksummer.process_bytes(buffer.data(), length);
			return 0;
		}, 0);

		return checksummer.checksum();
	}

	void UnloadTextResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		//ASSERT_TRUE(resource->IsLoaded());

		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<std::string*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadFakeTextResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		// TODO: should the resource manager be allowed to request reloads like this?
		//UnloadTextResource(resource, fs, userData);
		// ... or should this be a valid assertion?
		ASSERT_FALSE(resource->IsLoaded());

		auto content = new std::string();
		try
		{
			if (!user_data.empty())
			{
				auto resourceManager = boost::any_cast<resource_manager_f*>(user_data);
				*content = resourceManager->GetFile(resource->GetPath());
			}

			resource->SetDataPtr(content);
			resource->setLoaded(true);
		}
		catch (Exception& ex)
		{
			delete content;
			FSN_EXCEPT(FileSystemException, "Failed to load test resource: " + std::string(ex.what()));
		}
	}

	void LoadTextResource(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		// Resource manager should never attempt to load a resource that's already loaded
		ASSERT_FALSE(resource->IsLoaded());

		auto content = new std::string();
		try
		{
			//auto file = fs.open_file_read(resource->GetPath());
			//content->resize(file.get_size());
			//file.read(&(*content)[0], content->length());
			IO::PhysFSStream stream(resource->GetPath(), IO::Read);

			boost::crc_32_type checksummer;

			*content = processStream<std::string>(stream, [&checksummer](const std::array<char, 4096>& buffer, size_t length)->std::string
			{
				checksummer.process_bytes(buffer.data(), length);
				return std::string(buffer.data(), length);
			}, std::string());

			auto sum = checksummer.checksum();

			resource->SetMetadata((std::int64_t)sum);
			//auto modTime = PHYSFS_getLastModTime(resource->GetPath().c_str());
			//resource->SetMetadata(modTime);

			resource->SetDataPtr(content);
			resource->setLoaded(true);
		}
		catch (clan::Exception& e)
		{
			delete content;
			FSN_EXCEPT(FileSystemException, "Failed to load test resource: " + std::string(e.what()));
		}
	}

	bool ResourceFileDateHasChanged(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			auto modTime = PHYSFS_getLastModTime(resource->GetPath().c_str());
			auto storedModTime = resource->GetMetadataOrDefault(modTime);

			return modTime != storedModTime;
		}
		else
		{
			return false;
		}
	}

	bool TextContentHasChanged(ResourceContainer* resource, clan::FileSystem fs, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
				std::int64_t storedChecksum = resource->GetMetadataOrDefault<std::int64_t>(0);
				IO::PhysFSStream stream(resource->GetPath(), IO::Read);

				return checksumStream(stream) != storedChecksum;
		}
		else
		{
			return false;
		}
	}
}}

TEST_F(resource_manager_f, startStopThread)
{
	ASSERT_NO_THROW(manager->StartLoaderThread());
	ASSERT_NO_THROW(manager->StopLoaderThread());

	AddStandardLoaders();

	ASSERT_NO_THROW(manager->StartLoaderThread());
	ASSERT_NO_FATAL_FAILURE(AttemptActualLoad());
	ASSERT_NO_THROW(manager->StopLoaderThreadWhenDone());

	manager->DeliverLoadedResources();

	ASSERT_TRUE(resourcesBeingLoaded.back()->resource.get() != nullptr);
}

TEST_F(resource_manager_f, addLoaders)
{
	// Add a loader
	ASSERT_NO_THROW(manager->AddResourceLoader(ResourceLoader("IMAGE", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource)));
	// Replace the loader
	ASSERT_NO_THROW(manager->AddResourceLoader(ResourceLoader("IMAGE", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource)));
	// Fail to add a loader while the thread is running
	manager->StartLoaderThread();
	ASSERT_THROW(manager->AddResourceLoader(ResourceLoader("AUDIO", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource)), InvalidArgumentException);
	// Stop the thread and add the loader
	ASSERT_NO_FATAL_FAILURE(manager->StopLoaderThread());
	ASSERT_NO_THROW(manager->AddResourceLoader(ResourceLoader("AUDIO", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource)));
}

TEST_F(resource_manager_f, loadNonExistent)
{
	AddStandardLoaders();
	manager->StartLoaderThread();
	ASSERT_NO_THROW(AttemptMissingLoad());
	manager->StopLoaderThreadWhenDone();
	ASSERT_NO_FATAL_FAILURE(manager->DeliverLoadedResources());
	// No valid resource pointer should have been delivered
	ASSERT_TRUE(resourcesBeingLoaded.back()->resource.get() == nullptr);
	// The load connection should have been severed
	ASSERT_FALSE(resourcesBeingLoaded.back()->loadConnection.connected());
}

TEST_F(resource_manager_f, load)
{
	AddStandardLoaders();
	ASSERT_NO_THROW(AttemptActualLoad());
	ASSERT_NO_THROW(manager->StartLoaderThread());
	const size_t numIterations = 250;
	for (size_t i = 0; i < numIterations; ++i)
	{
		ASSERT_NO_THROW(AttemptActualLoad());
	}
	manager->DeliverLoadedResources();
	manager->UnloadUnreferencedResources();
	manager->StopLoaderThreadWhenDone();
	manager->DeliverLoadedResources();
	// All resources should be loaded when the thread stops
	for (auto it = resourcesBeingLoaded.begin(), end = resourcesBeingLoaded.end(); it != end; ++it)
	{
		ASSERT_TRUE((*it)->resource.get() != nullptr);
		ASSERT_TRUE((*it)->resource->IsLoaded());
	}
	// All resources with ref count > 1 should be loaded, and the rest should be unloaded
	ASSERT_TRUE(manager->VerifyResources(false));
}

TEST_F(resource_manager_f, load_unload)
{
	AddStandardLoaders();
	ASSERT_NO_THROW(AttemptActualLoad());
	ASSERT_NO_THROW(manager->StartLoaderThread());
	const size_t numIterations = 210;
	for (size_t i = 0; i < numIterations; ++i)
	{
		std::shared_ptr<ResourceHelper> helper;
		ASSERT_NO_THROW(helper = ResourceHelper::Load(manager, "resource" + n[i] + ".txt"));
		if ((i % 10) != 0)
		{
			resourcesBeingLoaded.push_back(helper);
		}
		if (i != 0 && (i % 14) == 0)
		{
			resourcesBeingLoaded.erase(resourcesBeingLoaded.begin());
		}
	}
	ASSERT_NO_FATAL_FAILURE(manager->DeliverLoadedResources());
	ASSERT_NO_FATAL_FAILURE(manager->UnloadUnreferencedResources());
	for (size_t u = 0; u < 12; ++u)
	{
		for (size_t i = 0; i < numIterations; ++i)
		{
			std::shared_ptr<ResourceHelper> helper;
			ASSERT_NO_THROW(helper = ResourceHelper::Load(manager, "resource" + n[i+u] + ".txt"));
			if ((i % 10) != 0)
				resourcesBeingLoaded.push_back(helper);
			if (i != 0 && (i % 14) == 0)
				resourcesBeingLoaded.erase(resourcesBeingLoaded.begin());
		}
		ASSERT_NO_FATAL_FAILURE(manager->DeliverLoadedResources());
		ASSERT_NO_FATAL_FAILURE(manager->UnloadUnreferencedResources());
		ASSERT_TRUE(manager->VerifyResources(true));
	}
	ASSERT_NO_FATAL_FAILURE(manager->StopLoaderThreadWhenDone());
	ASSERT_NO_FATAL_FAILURE(manager->DeliverLoadedResources());
	// All resources should be loaded
	for (auto it = resourcesBeingLoaded.begin(), end = resourcesBeingLoaded.end(); it != end; ++it)
	{
		ASSERT_TRUE((*it)->resource.get() != nullptr);
		ASSERT_TRUE((*it)->resource->IsLoaded());
	}
	ASSERT_TRUE(manager->VerifyResources(false));
	VerifyLoadedResources();
}

TEST_F(resource_manager_f, destruct_with_resources_in_use)
{
	const size_t numIterations = 70;
	for (size_t u = 0; u < 20; ++u)
	{
		for (size_t i = 0; i < numIterations; ++i)
		{
			std::shared_ptr<ResourceHelper> helper;
			ASSERT_NO_THROW(helper = ResourceHelper::Load(manager, "resource" + n[i+u] + ".txt"));
			if ((i % 10) != 0)
				resourcesBeingLoaded.push_back(helper);
			if (i != 0 && (i % 14) == 0)
				resourcesBeingLoaded.erase(resourcesBeingLoaded.begin());
		}
		manager->DeliverLoadedResources();
		manager->UnloadUnreferencedResources();
	}

	manager->DeliverLoadedResources();
	manager->UnloadUnreferencedResources();
	manager->StopLoaderThreadWhenDone();
	manager->DeliverLoadedResources();

	// Delete the manager with active resource pointers
	ASSERT_NO_FATAL_FAILURE(manager.reset());

	ASSERT_NO_FATAL_FAILURE(resourcesBeingLoaded.clear());
}

TEST_F(resource_manager_f, hot_reload)
{
	AddStandardLoaders();

	manager->SetHotReloadingAllowed(true);

	ASSERT_NO_THROW(manager->StartLoaderThread());
	//const size_t numIterations = 95;
	//for (size_t i = 0; i < numIterations; ++i)
	//{
	//	ASSERT_NO_THROW(AttemptActualLoad());
	//}

	const std::string testFilePath("testHotReload.txt");

	{
		IO::PhysFSStream newFile(testFilePath, IO::Write);
		newFile << "test data";
	}

	auto resourceToThatWillBeChanged = ResourceHelper::Load(manager, testFilePath);

	manager->DeliverLoadedResources();
	manager->UnloadUnreferencedResources();

	manager->StopLoaderThreadWhenDone();

	manager->DeliverLoadedResources();

	// All requested resources should be loaded when the thread stops
	ASSERT_TRUE(resourceToThatWillBeChanged->resource.get() != nullptr);
	ASSERT_TRUE(resourceToThatWillBeChanged->resource->IsLoaded());
	for (auto it = resourcesBeingLoaded.begin(), end = resourcesBeingLoaded.end(); it != end; ++it)
	{
		ASSERT_TRUE((*it)->resource.get() != nullptr);
		ASSERT_TRUE((*it)->resource->IsLoaded());
	}
	// All resources with ref count > 1 should be loaded, and the rest should be unloaded
	ASSERT_TRUE(manager->VerifyResources(false));

	auto initialMetadata = resourceToThatWillBeChanged->resource->GetMetadata<std::int64_t>();

	{
		IO::PhysFSStream existingFile(testFilePath, IO::Write);
		existingFile << "changed test data";
	}

	manager->StartLoaderThread();

	manager->CheckForChanges();

	auto startTime = clan::System::get_time();
	// Wait for reload, but give up after a few seconds
	while (!resourceToThatWillBeChanged->reloaded && (clan::System::get_time() - startTime) < 10000)
	{
		clan::System::sleep(100);
		manager->DeliverLoadedResources();
		manager->UnloadUnreferencedResources();
	}

	manager->StopLoaderThreadWhenDone();

	manager->DeliverLoadedResources();

	ASSERT_TRUE(resourceToThatWillBeChanged->resource.get() != nullptr);
	ASSERT_TRUE(resourceToThatWillBeChanged->resource->IsLoaded());
	for (auto it = resourcesBeingLoaded.begin(), end = resourcesBeingLoaded.end(); it != end; ++it)
	{
		ASSERT_TRUE((*it)->resource.get() != nullptr);
		ASSERT_TRUE((*it)->resource->IsLoaded());
	}
	ASSERT_TRUE(manager->VerifyResources(false));

	auto metadata = resourceToThatWillBeChanged->resource->GetMetadata<std::int64_t>();
	ASSERT_TRUE(metadata != initialMetadata);
}