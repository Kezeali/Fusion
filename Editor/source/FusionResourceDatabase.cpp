/*
*  Copyright (c) 2011-2012 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "PrecompiledHeaders.h"

#include "FusionResourceDatabase.h"

#include "FusionResourceManager.h"

#include <physfs.h>

#if _MSC_VER > 1000
#pragma warning( push )
#pragma warning( disable: 4244 4351; )
#endif
#include <kchashdb.h>
#if _MSC_VER > 1000
#pragma warning( pop )
#endif

namespace FusionEngine
{

	ResourceDatabase::ResourceDatabase()
	{
		m_Database.reset(new kyotocabinet::HashDB);

		m_Database->tune_options(kyotocabinet::HashDB::TSMALL);
		m_Database->tune_defrag(8);
		m_Database->tune_map(2LL << 20); // 2MB memory-map

		m_Database->open(
			std::string(PHYSFS_getWriteDir()) + PHYSFS_getDirSeparator() + "Editor" + PHYSFS_getDirSeparator() +"resources.kc",
			kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE);

		using namespace std::placeholders;
		m_OnResourceLoadedConnection =
			ResourceManager::getSingleton().SignalResourceLoaded.connect(std::bind(&ResourceDatabase::OnResourceLoaded, this, _1));
	}

	ResourceDatabase::~ResourceDatabase()
	{
		m_OnResourceLoadedConnection.disconnect();

		m_Database->close();
	}

	void ResourceDatabase::Clear()
	{
		m_Database->clear();
	}

	std::string ResourceDatabase::GetResourceType(const std::string& path)
	{
		auto valueSize = m_Database->get(path.c_str(), path.length(), m_Buffer.data(), m_Buffer.size());
		if (valueSize > 0)
			return std::string(m_Buffer.data(), m_Buffer.data() + valueSize);
		else
			return std::string();
	}

	bool ResourceDatabase::RemoveResourceType(const std::string& path)
	{
		return m_Database->remove(path);
	}

	void ResourceDatabase::OnResourceLoaded(const ResourceDataPtr& resource)
	{
		std::string path = resource->GetPath();
		std::string type = resource->GetType();
		m_Database->set(path.c_str(), path.length(), type.c_str(), type.length());
	}

}