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
	
	namespace EntitySerialisationUtils
	{
		bool SerialiseEntity(RakNet::BitStream& out, EntityPtr entity, IComponent::SerialiseMode mode);
		void DerialiseEntity(RakNet::BitStream& in, EntityPtr entity, IComponent::SerialiseMode mode, EntityFactory* factory, EntityManager* manager);

		void WriteComponent(CL_IODevice& out, IComponent* component);
		void ReadComponent(CL_IODevice& in, IComponent* component);

		void SaveEntity(CL_IODevice& out, EntityPtr entity, bool id_included);
		EntityPtr LoadEntity(CL_IODevice& in, bool id_included, EntityFactory* factory, EntityManager* manager, InstancingSynchroniser* synchroniser);
	}

}

#endif
