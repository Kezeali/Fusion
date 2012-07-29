/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionMultiEntityLoader
#define H_FusionMultiEntityLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntitySerialisationUtils.h"

#include <memory>
#include <list>

namespace FusionEngine
{
	
	//! Multi-entity loader
	class EntityLoader
	{
	public:
		//! CTOR
		EntityLoader();
		//! Move CTOR
		EntityLoader(EntityLoader&& other);

		//! Add an future to the list
		void Add(const std::shared_ptr<EntitySerialisationUtils::EntityFuture>& future);

		//! Outputs all the currently available entities
		template <typename IterT>
		void Publish(IterT output);

	private:
		std::list<std::shared_ptr<EntitySerialisationUtils::EntityFuture>> m_ExpectedEntities;

		//! Non-copyable
		EntityLoader(const EntityLoader&) {}
	};

	template <typename IterT>
	inline void EntityLoader::Publish(IterT output)
	{
		for (auto it = m_ExpectedEntities.begin(); it != m_ExpectedEntities.end();)
		{
			const auto& future = *it;
			if (future->is_ready())
			{
				*output = future->get();
				++output;
				it = m_ExpectedEntities.erase(it);
			}
			else
				++it;
		}
	}

}

#endif
