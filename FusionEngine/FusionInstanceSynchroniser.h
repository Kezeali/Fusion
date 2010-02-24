/*
*  Copyright (c) 2010 Fusion Project Team
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

#ifndef Header_FusionEntityInstancer
#define Header_FusionEntityInstancer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntity.h"
#include "FusionIDStack.h"
#include "FusionPacketHandler.h"
#include "FusionRakNetwork.h"

namespace FusionEngine
{

	//! Synchronises the creation of new instances
	class InstanceSynchroniser : public PacketHandler
	{
	public:
		InstanceSynchroniser(EntityFactory *factory, EntityManager *manager);
		~InstanceSynchroniser();

		//! The next id after the last id used in the loaded map or save-game
		void SetMapBaseID(ObjectID id);
		//! The next id after the last id used by the given peer
		void SetPeerBaseID(ObjectID id);

		//! Tries to create a new entity (only succeeds if this peer has authority to do so)
		/*!
		* \param[in] requester
		* The Entity calling this function. The EntityAdded callback will
		* be called on this entity when the new entity is created.
		* Also used to validate the call - GetOwnerID() must be a valid 
		* player ID unless this peer is the arbitrator.
		* \param[in] syncable
		* True if the new Entity should be synchronisable (not pseudo-)
		* \param[in] type
		* The type of the Entity to create.
		* \param[in] instance_owner
		* The player who will own the new instance. 0 for none
		*/
		void RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, const std::string &name, PlayerID instance_owner = 0);

		//! Tries to create a new entity (only succeeds if this peer has authority to do so)
		/*!
		* \param[in] requester
		* The Entity calling this function. The EntityAdded callback will
		* be called on this entity when the new entity is created.
		* Also used to validate the call - GetOwnerID() must be a valid 
		* player ID unless this peer is the arbitrator.
		* \param[in] syncable
		* True if the new Entity should be synchronisable (not pseudo-)
		* \param[in] type
		* The type of the Entity to create.
		* \param[in] instance_owner
		* The player who will own the new instance. 0 for none
		*/
		void RequestInstance(EntityPtr &requester, bool syncable, const std::string &type, PlayerID instance_owner = 0);

		//! Pick up entity creation packets
		void HandlePacket(Packet *packet);

	protected:
		EntityFactory *m_Factory;
		EntityManager *m_EntityManager;

		RakNetwork *m_Network;

		ObjectIDStack m_LocalIdGenerators[s_MaxPeers];
		ObjectIDStack m_WorldIdGenerator;

		//boost::signals2::connection m_EntityInstancedCnx;

		//! Gets a new ID for local-authroity instancing
		ObjectID generateLocalId();
		void sendInstancingMessage(ObjectID requester_id, ObjectID id, const std::string &type, const std::string &name, PlayerID owner_id);
	};

}

#endif
