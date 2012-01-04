#include "PrecompiledHeaders.h"

#include "FusionPrerequisites.h"

#include "FusionResourceManager.h"

#include "FusionResourcePointer.h"

#include "FusionLogger.h"

#include <gtest/gtest.h>

namespace FusionEngine { namespace Test
{
	void UnloadTextResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		//ASSERT_TRUE(resource->IsLoaded());

		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<std::string*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

	void LoadTextResource(ResourceContainer* resource, CL_VirtualDirectory vdir, void* userData)
	{
		// TODO: should the resource manager be allowed to request reloads like this?
		//UnloadTextResource(resource, vdir, userData);
		// ... or should this be a valid assertion?
		ASSERT_FALSE(resource->IsLoaded());

		auto content = new std::string();
		try
		{
			auto file = vdir.open_file_read(resource->GetPath());
			content->resize(file.get_size());
			file.read(&(*content)[0], content->length());
		}
		catch (CL_Exception& e)
		{
			FSN_EXCEPT(FileSystemException, "Failed to load test resource: " + std::string(e.what()));
		}

		resource->SetDataPtr(content);
		resource->setLoaded(true);
	}

} }
	
using namespace FusionEngine;

class resource_manager_f : public testing::Test
{
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
		manager = std::make_shared<ResourceManager>(gc);

		std::stringstream str;
		n.resize(1024);
		for (size_t i = 0; i < 1024; ++i)
		{
			str << i;
			n[i] = str.str();
			str.str("");
		}
	}
	virtual void TearDown()
	{
		ASSERT_NO_FATAL_FAILURE(resourcesBeingLoaded.clear());

		ASSERT_NO_FATAL_FAILURE(manager.reset());
	}

	void AddStandardLoaders()
	{
		ASSERT_NO_FATAL_FAILURE(manager->AddResourceLoader(ResourceLoader("IMAGE", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource)));
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
		ResourceHelper() {}
		virtual ~ResourceHelper()
		{
			loadConnection.disconnect();
		}

		static std::shared_ptr<ResourceHelper> Load(const std::shared_ptr<ResourceManager>& manager, const std::string& path)
		{
			auto helper = std::make_shared<ResourceHelper>();
			std::weak_ptr<ResourceHelper> helperRef = helper;
			helper->loadConnection = manager->GetResource("IMAGE", path, [helperRef](ResourceDataPtr resource) { if (auto h = helperRef.lock()) h->resource = resource; });
			return helper;
		}
		boost::signals2::connection loadConnection;
		ResourceDataPtr resource;
	};
	
	std::vector<std::shared_ptr<ResourceHelper>> resourcesBeingLoaded;

	std::vector<std::string> n;
	size_t currentN;

	std::shared_ptr<ResourceManager> manager;
	CL_GraphicContext gc;
	std::shared_ptr<Logger> logger;
};

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
	ASSERT_NO_THROW(manager->AddResourceLoader("IMAGE", &FusionEngine::Test::LoadTextResource, &FusionEngine::Test::UnloadTextResource, nullptr));
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
	const size_t numIterations = 250;
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
			{
				resourcesBeingLoaded.push_back(helper);
			}
			if (i != 0 && (i % 14) == 0)
			{
				resourcesBeingLoaded.erase(resourcesBeingLoaded.begin());
			}
			//if ((i % 60) == 0)
			//{
			//	manager->UnloadUnreferencedResources();
			//	manager->DeliverLoadedResources();
			//}
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
	const size_t numIterations = 1000;
	for (size_t i = 0; i < numIterations; ++i)
	{
		ASSERT_NO_THROW(AttemptActualLoad());
	}

	manager->DeliverLoadedResources();
	manager->UnloadUnreferencedResources();
	manager->StopLoaderThreadWhenDone();
	manager->DeliverLoadedResources();

	// Delete the manager with active resource pointers
	ASSERT_NO_FATAL_FAILURE(manager.reset());

	ASSERT_NO_FATAL_FAILURE(resourcesBeingLoaded.clear());
}