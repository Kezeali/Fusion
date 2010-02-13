/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "Common.h"

#include "FusionStreamedResourceUser.h"

#include "FusionResourceManager.h"


namespace FusionEngine
{

	StreamedResourceUser::StreamedResourceUser()
		: m_ResourceManager(NULL),
		m_Priority(0)
	{
	}

	StreamedResourceUser::StreamedResourceUser(ResourceManager *res_man, const std::string &type, const std::string &path, int priority)
		: m_ResourceManager(res_man),
		m_ResourceType(type),
		m_ResourcePath(path),
		m_Priority(priority)
	{
	}

	void StreamedResourceUser::SetResource(ResourceManager *res_man, const std::string &path)
	{
		m_ResourceManager = res_man;
		m_ResourcePath = path;
	}

	const ResourceDataPtr& StreamedResourceUser::GetResource() const
	{
		return m_Resource;
	}

	void StreamedResourceUser::StreamIn(int priority)
	{
		m_LoadConnection = m_ResourceManager->GetResource(m_ResourceType, m_ResourcePath, boost::bind(&StreamedResourceUser::OnResourceLoad, this, _1), m_Priority + priority);
		
		OnStreamIn();
	}

	void StreamedResourceUser::StreamOut()
	{
		OnStreamOut();

		m_LoadConnection.disconnect();
		m_Resource.reset();
	}

}
