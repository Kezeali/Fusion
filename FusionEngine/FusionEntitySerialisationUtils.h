/*
*  Copyright (c) 2011 Fusion Project Team
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

#ifndef H_FusionEntitySerialisationUtils
#define H_FusionEntitySerialisationUtils

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

//#include <BitStream.h>
#include <ClanLib/core.h>

#include "FusionEntityComponent.h"

namespace RakNet
{
	class BitStream;
}

namespace FusionEngine
{

	typedef std::basic_istream<char> ICellStream;
	typedef std::basic_ostream<char> OCellStream;
	
	namespace EntitySerialisationUtils
	{
		bool SerialiseEntity(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode);
		EntityPtr DeserialiseEntity(RakNet::BitStream& in, EntityFactory* factory, EntityManager* manager);

		bool SerialiseContinuous(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode);
		void DeserialiseContinuous(RakNet::BitStream& in, const EntityPtr& entity, IComponent::SerialiseMode mode);

		bool SerialiseOccasional(RakNet::BitStream& out, std::vector<uint32_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode);
		void DeserialiseOccasional(RakNet::BitStream& in, std::vector<uint32_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode);

		//! Deserialises and returns the position data from the given state, if it contains such data
		std::pair<bool, Vector2> DeserialisePosition(RakNet::BitStream& in, const Vector2& origin, const float radius);

		void WriteComponent(OCellStream& out, IComponent* component);
		void ReadComponent(ICellStream& in, IComponent* component);

		//! Merge inactive entity data
		void MergeEntityData(ICellStream& in, OCellStream& out, RakNet::BitStream& incomming, RakNet::BitStream& incomming_occasional);

		//! Save an active entity
		void SaveEntity(OCellStream& out, EntityPtr entity, bool id_included);
		//! Load an entity
		EntityPtr LoadEntity(ICellStream& in, bool id_included, ObjectID override_id, EntityFactory* factory, EntityManager* manager, InstancingSynchroniser* synchroniser);
	}

}

#endif
