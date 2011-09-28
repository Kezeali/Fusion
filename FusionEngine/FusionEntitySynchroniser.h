/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionEntitySynchroniser
#define H_FusionEntitySynchroniser

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Bitstream.h>
#include <RakNetTypes.h>

#include "FusionEntity.h"
#include "FusionIDStack.h"
#include "FusionInputHandler.h"
#include "FusionPacketHandler.h"
#include "FusionPlayerInput.h"

namespace FusionEngine
{
	
	//! Updates input states for each player (local and remote)
	class ConsolidatedInput
	{
	public:
		typedef std::tr1::unordered_map<PlayerID, PlayerInputPtr> PlayerInputsMap;

	public:
		ConsolidatedInput(InputManager *input_manager);
		~ConsolidatedInput();

		void SetState(PlayerID player, const std::string input, bool active, float position);
		PlayerInputPtr GetInputsForPlayer(PlayerID player);

		const PlayerInputsMap &GetPlayerInputs() const;

		unsigned short ChangedCount() const;
		void ChangesRecorded();

		PlayerID LocalToNetPlayer(unsigned int local);

	protected:
		InputManager *m_LocalManager;

		void onInputChanged(const InputEvent &event);

		unsigned short m_ChangedCount;

		PlayerInputsMap m_PlayerInputs;

		boost::signals2::connection m_InputChangedConnection;

	};

	static const unsigned int s_EntitiesPerPacket = 8;
	static const unsigned int s_BodiesPerPacket = 12;

	static const RakNet::BitSize_t s_MaxDataPerTick = 4000;

	class EntitySynchroniser : public PacketHandler
	{
	public:
		EntitySynchroniser(InputManager *input_manager);
		~EntitySynchroniser();

		const EntityArray &GetReceivedEntities() const;

		//! Sends data
		//void Send();

		//! Prepares the given entity for synch
		void OnEntityActivated(EntityPtr &entity);
		//! Removes the given entity from the queue, if it hasn't been cleared yet
		void OnEntityDeactivated(EntityPtr &entity);

		//! Enqueues the given entity to be processed for synch
		bool Enqueue(EntityPtr &entity);
		//! Sends enqueued entities
		void ProcessQueue(bool send, EntityManager* entity_manager, EntityFactory* factory);
		//! Implements PacketHandler
		void HandlePacket(RakNet::Packet *packet);

	private:
		void ProcessPacket(const RakNet::RakNetGUID& guid, RakNet::BitStream& bitStream);

		// Things called by ProcessQueue
		void WriteHeaderAndInput(bool important, RakNet::BitStream& packetData);
		void SendPackets();
		bool ReceiveSync(EntityPtr &entity, EntityManager* entity_manager, EntityFactory* factory);

		ConsolidatedInput *m_PlayerInputs;
		InputManager *m_InputManager;

		RakNetwork *m_Network;

		//typedef std::tr1::unordered_map<ObjectID, EntityArray> EntityPreparedInstancesMap;
		//EntityPreparedInstancesMap m_EntityPreparedInstances;
		boost::signals2::connection m_EntityInstancedCnx;

		std::vector<EntityPtr> m_ReceivedEntities;

		std::vector<EntityPtr> m_EntitiesToReceive;

		std::set<ObjectID> m_TEMPQueuedEntities;

		struct StateData
		{
			bool full;
			uint32_t tick;
			uint32_t conTick;
			uint32_t ocaTick;
			RakNet::RakNetGUID guid;
			std::shared_ptr<RakNet::BitStream> continuous;
			std::shared_ptr<RakNet::BitStream> occasional;
		};

		std::map<RakNet::RakNetGUID, uint32_t> m_RemoteTicks;

		struct JitterBufferPacket
		{
			//RakNet::RakNetGUID guid;
			RakNet::Time timestamp;
			uint32_t tick;
			std::shared_ptr<RakNet::BitStream> data;

			JitterBufferPacket() : tick(0) {}
			//JitterBufferPacket(/*const RakNet::RakNetGUID& id, */RakNet::Time time, std::shared_ptr<RakNet::BitStream>&& pdat)
			//	: /*guid(id),*/
			//	timestamp(time),
			//	data(std::move(pdat))
			//{
			//}
			JitterBufferPacket(JitterBufferPacket&& other)
				: /*guid(other.guid),*/
				timestamp(other.timestamp),
				data(std::move(other.data)),
				tick(other.tick)
			{}
			JitterBufferPacket(const JitterBufferPacket& other)
				: /*guid(other.guid),*/
				timestamp(other.timestamp),
				data(other.data),
				tick(other.tick)
			{}
		};

		struct JitterBuffer
		{
			RakNet::Time remoteTime;
			RakNet::Time localTime;
			RakNet::Time lastPopTime;
			double popRate;
			uint32_t lastTickPopped;
			uint32_t remoteTick;
			bool filling;
			bool emptying;
			uint32_t lastTickSkipped;
			std::vector<JitterBufferPacket> buffer;

			JitterBuffer()
				: remoteTime(0), lastPopTime(0), popRate(1.0), filling(true), emptying(false)
			{}
		};

		//std::map<RakNet::RakNetGUID, RakNet::Time> m_RemoteTime;
		std::map<RakNet::RakNetGUID, JitterBuffer> m_JitterBuffers;
		RakNet::Time m_JitterBufferTargetLength;

		typedef std::map<ObjectID, StateData> ObjectStatesMap;
		ObjectStatesMap m_ReceivedStates;

		std::map<ObjectID, std::vector<uint16_t>> m_SentStates;

		struct EntityPacketData
		{
			ObjectID ID;
			SerialisedData State;
		};

		std::vector<EntityPtr> m_ImportantEntities; // entities which have player inputs that have changed state

		typedef std::multimap<unsigned int, EntityPtr, std::greater<unsigned int>> EntityPriorityMap;
		EntityPriorityMap m_EntityPriorityQueue;

		int64_t m_PacketDataBudget;

		// Stats
		size_t m_MinEntsInAPacket;
		size_t m_MaxEntsInAPacket;
		RakNet::BitSize_t m_MaxContinuousStateSize;

		struct SystemPriority
		{
			RakNet::RakNetGUID System;
			float Distance; // Average Entity distance
			unsigned int SkippedSteps;
		};

		typedef std::vector<SystemPriority> SystemArray;
		SystemArray m_PacketDestinations;

		bool m_ImportantMove;
		bool m_FullSynch;
		//std::string m_PacketData;
		RakNet::BitStream m_PacketData;
	};

}

#endif
