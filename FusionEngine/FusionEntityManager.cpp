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

#include "FusionStableHeaders.h"

#include "FusionEntityManager.h"

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>
#include <BitStream.h>
#include <RakNetStatistics.h>

#include "FusionCameraSynchroniser.h"
#include "FusionClientOptions.h"
#include "FusionDeltaTime.h"
#include "FusionEntityFactory.h"
#include "FusionEntitySynchroniser.h"
#include "FusionExceptionFactory.h"
#include "FusionNetDestinationHelpers.h"
#include "FusionNetworkTypes.h"
#include "FusionPlayerRegistry.h"
#include "FusionRakNetwork.h"
#include "FusionRenderer.h"
#include "FusionScriptTypeRegistrationUtils.h"
// For script registration (the script method EntityManager::instance() returns a script object)
#include "FusionScriptedEntity.h"

#include "FusionEntitySerialisationUtils.h"

#include "FusionProfiling.h"

#include <tbb/parallel_do.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>

using namespace std::placeholders;
using namespace RakNet;

namespace FusionEngine
{
	using namespace EntitySerialisationUtils;

	ConsolidatedInput::ConsolidatedInput(InputManager *input_manager)
		: m_LocalManager(input_manager),
		m_ChangedCount(0)
	{
		m_InputChangedConnection = m_LocalManager->SignalInputChanged.connect( std::bind(&ConsolidatedInput::onInputChanged, this, _1) );
	}

	ConsolidatedInput::~ConsolidatedInput()
	{
		m_InputChangedConnection.disconnect();
	}

	void ConsolidatedInput::SetState(PlayerID player, const std::string input, bool active, float position)
	{
		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			_where->second->SetState(input, active, position);
		}
	}

	PlayerInputPtr ConsolidatedInput::GetInputsForPlayer(PlayerID player)
	{
		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			return _where->second;
		}
		else
		{
			// Create an entry for the given player, if it is a valid player
			//if (PlayerRegistry::GetPlayer(player).NetID != 0)
			if (player != 0)
				return m_PlayerInputs[player] = PlayerInputPtr( new PlayerInput(m_LocalManager->GetDefinitionLoader()->GetInputDefinitions()) );
			else
				return PlayerInputPtr();
		}
	}

	const ConsolidatedInput::PlayerInputsMap &ConsolidatedInput::GetPlayerInputs() const
	{
		return m_PlayerInputs;
	}

	unsigned short ConsolidatedInput::ChangedCount() const
	{
		return m_ChangedCount;
	}

	void ConsolidatedInput::ChangesRecorded()
	{
		m_ChangedCount = 0;
	}

	PlayerID ConsolidatedInput::LocalToNetPlayer(unsigned int local)
	{
		return PlayerRegistry::GetPlayerByLocalIndex(local).NetID;
	}

	void ConsolidatedInput::onInputChanged(const InputEvent &ev)
	{
		if (ev.Player < 0)
			return;
		PlayerID player = LocalToNetPlayer((unsigned int)ev.Player);
		PlayerInputsMap::iterator _where = m_PlayerInputs.find(player);
		if (_where != m_PlayerInputs.end())
		{
			PlayerInputPtr &playerInput = _where->second;

			bool changed = playerInput->HasChanged();
			
			if (ev.Type == InputEvent::Binary)
				playerInput->SetActive(ev.Input, ev.Down);
			else
				playerInput->SetPosition(ev.Input, (float)ev.Value);

			if (changed != playerInput->HasChanged())
				++m_ChangedCount;
		}
	}

	EntitySynchroniser::EntitySynchroniser(InputManager *input_manager, CameraSynchroniser* camera_synchroniser)
		: m_InputManager(input_manager),
		m_PlayerInputs(new ConsolidatedInput(input_manager)),
		m_CameraSynchroniser(camera_synchroniser),
		m_SendTick(0),
		m_JitterBufferTargetLength(100),
		m_UseJitterBuffer(true),
		m_MinEntsInAPacket(std::numeric_limits<size_t>::max()),
		m_MaxEntsInAPacket(0),
		m_MaxContinuousStateSize(0)
	{
		NetworkManager::getSingleton().Subscribe(MTID_IMPORTANTMOVE, this);
		NetworkManager::getSingleton().Subscribe(MTID_ENTITYMOVE, this);
		//NetworkManager::getSingleton().Subscribe(MTID_STARTSYNC, this);
	}

	EntitySynchroniser::~EntitySynchroniser()
	{
		m_EntityInstancedCnx.disconnect();

		NetworkManager::getSingleton().Unsubscribe(MTID_IMPORTANTMOVE, this);
		NetworkManager::getSingleton().Unsubscribe(MTID_ENTITYMOVE, this);
		//NetworkManager::getSingleton().Unsubscribe(MTID_STARTSYNC, this);

		std::stringstream str;
		str << "MinStatesInAPacket: " << m_MinEntsInAPacket;
		Logger::getSingleton().Add(str.str(), "Network");
		str.str("");
		str << "MaxStatesInAPacket: " << m_MaxEntsInAPacket;
		Logger::getSingleton().Add(str.str(), "Network");
		str.str("");
		str << "MaxStateSize: " << m_MaxContinuousStateSize;
		Logger::getSingleton().Add(str.str(), "Network");
	}

	void EntitySynchroniser::WriteHeaderAndInput(bool important, RakNet::BitStream& packetData)
	{
		//packetData.Write((MessageID)ID_TIMESTAMP);
		//packetData.Write(RakNet::GetTime());
		if (!important)
		{
			packetData.Write((MessageID)MTID_ENTITYMOVE);

			packetData.Write(m_SendTick);
		}
		else
		{
			packetData.Write((MessageID)MTID_IMPORTANTMOVE);

			packetData.Write(m_SendTick);

			const ConsolidatedInput::PlayerInputsMap &inputs = m_PlayerInputs->GetPlayerInputs();

			unsigned short numChangedInputCollections = std::count_if(inputs.cbegin(), inputs.cend(), [](const ConsolidatedInput::PlayerInputsMap::value_type& entry)
			{
				return entry.second->HasChanged();
			});

			//FSN_ASSERT_MSG(numChangedInputCollections > 0, "Important moves should have changed input!");
			FSN_ASSERT(numChangedInputCollections <= inputs.size());

			packetData.Write(numChangedInputCollections);

			for (ConsolidatedInput::PlayerInputsMap::const_iterator it = inputs.begin(), end = inputs.end(); it != end; ++it)
			{
				PlayerID playerId = it->first;
				const PlayerInputPtr &playerInput = it->second;

				if (playerInput->HasChanged())
				{
					packetData.Write(playerId);

					playerInput->Serialise(&packetData);
				}
			}
		}
		//packetData.Write(DeltaTime::GetTick());
	}

//#define FSN_PARALLEL_SERIALISE

#ifdef FSN_PARALLEL_SERIALISE
	typedef tbb::concurrent_vector<std::tuple<ObjectID, PlayerID, std::shared_ptr<RakNet::BitStream>>> DataToSend_t;
#else
	typedef std::vector<std::tuple<ObjectID, PlayerID, std::shared_ptr<RakNet::BitStream>>> DataToSend_t;
#endif

	static void writeStates(RakNet::BitStream& packetData, const DataToSend_t& dataToSend)
	{
		auto numStates = (unsigned short)dataToSend.size();
		packetData.Write(numStates);
		for (auto it = dataToSend.begin(), end = dataToSend.end(); it != end; ++it)
		{
			ObjectID id = std::get<0>(*it);
			PlayerID authority = std::get<1>(*it);
			auto& state = std::get<2>(*it);

			state->ResetReadPointer();

			const auto bitsUsed = state->GetNumberOfBitsUsed();
			if (bitsUsed < std::numeric_limits<uint16_t>::max())
			{
				const uint16_t bitsUsed16 = uint16_t(bitsUsed);

				packetData.Write(id);
				packetData.Write(authority);
				packetData.Write(bitsUsed16);
				packetData.Write(*state, state->GetNumberOfBitsUsed());
			}
			FSN_ASSERT_MSG(bitsUsed < std::numeric_limits<uint16_t>::max(), "Entity state is too big");
		}
	}

	void EntitySynchroniser::SendPackets()
	{
		auto network = NetworkManager::GetNetwork();
		
		struct PersonalisedData
		{
			BitSize_t dataUsed;
			DataToSend_t continuousData;
			DataToSend_t occasionalData;
			bool important;
			PersonalisedData() : important(false) {}
		};
#ifdef FSN_PARALLEL_SERIALISE
		tbb::concurrent_unordered_map<RakNet::RakNetGUID, PersonalisedData> dataToSendToSystems;
#else
		std::map<RakNet::RakNetGUID, PersonalisedData> dataToSendToSystems;
#endif

		bool important = true;

		auto processEntity = [&](const EntityPtr& entity)
		{
			auto& remoteViewers = std::set<RakNet::RakNetGUID>();//entity->GetViewers();
			for (auto it = PlayerRegistry::PlayersBegin(), end = PlayerRegistry::PlayersEnd(); it != end; ++it)
				if (it->LocalIndex >= s_MaxLocalPlayers)
					remoteViewers.insert(it->GUID);

			if (!remoteViewers.empty())
			{
				auto state = std::make_shared<RakNet::BitStream>();
				state->Write1();
				if (SerialiseContinuous(*state, entity, IComponent::All))
				{
					const BitSize_t stateSize = state->GetNumberOfBitsUsed();
#ifdef _DEBUG
					m_MaxContinuousStateSize = std::max<BitSize_t>(stateSize, m_MaxContinuousStateSize);
#endif

					// Check whether this will fit within the quota
					if (m_PacketDataBudget - (stateSize * remoteViewers.size()) >= 0)
					{
						for (auto vit = remoteViewers.begin(), vend = remoteViewers.end(); vit != vend; ++vit)
						{
							auto& dataInfo = dataToSendToSystems[*vit];

							dataInfo.continuousData.push_back(std::make_tuple(entity->GetID(), entity->GetAuthority(), state));
							dataInfo.important |= important; // If the dataset contains any important entity changes, it must be sent as Important data

							m_PacketDataBudget -= stateSize;

							entity->AddedToPacket();
						}
					}
				}
				state = std::make_shared<RakNet::BitStream>();
				state->Write1();
				auto& existingChecksums = m_SentStates[entity->GetID()];
				//auto newChecksums = existingChecksums;
				if (SerialiseOccasional(*state, existingChecksums/*newChecksums*/, entity, IComponent::Changes))
				{
					const BitSize_t stateSize = state->GetNumberOfBitsUsed();
					
					//boost::crc_32_type crc;
					//crc.process_bytes(state->GetData(), state->GetNumberOfBytesUsed());
					//auto newChecksum = crc.checksum();

					// Compare the checksums
					//if (newChecksums != existingChecksums)
					{
						// Check whether this will fit within the quota
						if (m_PacketDataBudget - (stateSize * remoteViewers.size()) >= 0)
						{
							for (auto vit = remoteViewers.begin(), vend = remoteViewers.end(); vit != vend; ++vit)
							{
								auto& dataInfo = dataToSendToSystems[*vit];

								dataInfo.occasionalData.push_back(std::make_tuple(entity->GetID(), entity->GetAuthority(), state));
								dataInfo.important |= important; // If the dataset contains any important entity changes, it must be sent as Important data

								m_PacketDataBudget -= stateSize;

								entity->AddedToPacket();
							}
						}
						// Update the stored checksums
						//existingChecksums = newChecksums;
					}
				}
			}
		};

		std::for_each(m_ImportantEntities.cbegin(), m_ImportantEntities.cend(), processEntity);

		important = false;

#ifdef FSN_PARALLEL_SERIALISE
		tbb::parallel_do(m_EntityPriorityQueue.begin(), m_EntityPriorityQueue.end(), [&](EntityPriorityMap::const_reference val)
		{
			auto& entity = val.second;
#else
		for (auto it = m_EntityPriorityQueue.begin(), end = m_EntityPriorityQueue.end(); it != end; ++it)
		{
			auto& entity = it->second;
#endif
			processEntity(entity);
		}
#ifdef FSN_PARALLEL_SERIALISE
		);
#endif

		RakNet::BitStream contPacketData, occnPacketData;
		for (auto sit = dataToSendToSystems.cbegin(), send = dataToSendToSystems.cend(); sit != send; ++sit)
		{
			const auto& dest = sit->first;
			auto& contData = sit->second.continuousData;
			auto& occnData = sit->second.occasionalData;
			bool important = sit->second.important;

			if (important)
			{
				// Write input if this is an important packet
				WriteHeaderAndInput(important, contPacketData);

				contPacketData.Write1();
				contPacketData.Write1();

				writeStates(contPacketData, contData);
				writeStates(contPacketData, occnData);

				network->SendAsIs(NetDestination(dest, false), &contPacketData, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYSYNC);
			}
			else
			{
				if (!contData.empty())
				{
#ifdef _DEBUG
					const auto num = contData.size();
					m_MinEntsInAPacket = std::min(num, m_MinEntsInAPacket);
					m_MaxEntsInAPacket = std::max(num, m_MaxEntsInAPacket);
#endif
					WriteHeaderAndInput(important, contPacketData);
					// Packet contains continuous data and no occasional data
					contPacketData.Write1();
					contPacketData.Write0();

					writeStates(contPacketData, contData);

					network->SendAsIs(NetDestination(dest, false), &contPacketData, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, CID_ENTITYSYNC);
				}

				if (!occnData.empty())
				{
					WriteHeaderAndInput(important, occnPacketData);
					// Packet contains no continuous data but does contain occasional data
					occnPacketData.Write0();
					occnPacketData.Write1();
					// Write the actual data
					writeStates(occnPacketData, occnData);

					network->SendAsIs(NetDestination(dest, false), &occnPacketData, MEDIUM_PRIORITY, RELIABLE_SEQUENCED, CID_ENTITYSYNC);
				}
			}

			contPacketData.Reset();
			occnPacketData.Reset();
		}

		{
		RakNet::BitStream packetData;
		for (auto it = m_ToInform.cbegin(), end = m_ToInform.cend(); it != end; ++it)
		{
			const auto& dest = it->first;
			auto& states = it->second;

			for (auto sit = states.cbegin(), send = states.cend(); sit != send; ++sit)
			{
				ObjectID id = sit->first;
				auto& state = sit->second;

				packetData.Write(id);
				packetData.Write(state->GetNumberOfBitsUsed());
				packetData.Write(state);
			}

			network->Send(NetDestination(dest, false), false, MTID_CORRECTION, &packetData, LOW_PRIORITY, RELIABLE_ORDERED, CID_ENTITYSYNC);

			packetData.Reset();
		}
		}

		m_EntityPriorityQueue.clear();
		m_ImportantEntities.clear();
		m_PlayerInputs->ChangesRecorded();
	}

	void EntitySynchroniser::OnEntityActivated(const EntityPtr &entity)
	{
		PlayerInputPtr playerInput = m_PlayerInputs->GetInputsForPlayer(entity->GetOwnerID());
		if (playerInput)
			entity->_setPlayerInput(playerInput);
	}

	void EntitySynchroniser::OnEntityDeactivated(const EntityPtr &entity)
	{
		{
			auto entry = std::find(m_EntitiesToReceive.begin(), m_EntitiesToReceive.end(), entity);
			if (entry != m_EntitiesToReceive.end())
				m_EntitiesToReceive.erase(entry);
		}

		{
			auto entry = std::find_if(m_EntityPriorityQueue.begin(), m_EntityPriorityQueue.end(), [&](const EntityPriorityMap::value_type& val) { return val.second == entity; });
			if (entry != m_EntityPriorityQueue.end())
				m_EntityPriorityQueue.erase(entry);
		}

		m_SentStates.erase(entity->GetID());
	}

	bool EntitySynchroniser::ReceiveSync(EntityPtr &entity, EntityManager* entity_manager, EntityFactory* factory)
	{
		//ObjectStatesMap::const_iterator _where = m_ReceivedStates.find(entity->GetID());
		//if (_where != m_ReceivedStates.end())
		//	entity->DeserialiseState(_where->second, false, entity_deserialiser);

		auto _where = m_ReceivedStates.find(entity->GetID());
		if (_where != m_ReceivedStates.end())
		{
			auto& synchInfo = _where->second;

			const bool owned = entity->GetOwnerID() != 0;
			const bool defaultAuthority = NetworkManager::IsSenior(synchInfo.guid) ? synchInfo.authority == 0 : (entity->GetOwnerID() == 0 && entity->GetAuthority() == 0);
			auto remoteAuthority = PlayerRegistry::GetPlayer(synchInfo.authority);
			auto currentAuthority = PlayerRegistry::GetPlayer(entity->GetAuthority());
			if (currentAuthority.IsLocal())
				currentAuthority.GUID = NetworkManager::GetNetwork()->GetLocalGUID();

			// The sender is authoritative when
			//  a) it owns the given entity
			//  b) the entity is under the sender's authority
			//  c) the entity is under default authority and the sender is older
			const bool isAuthoritativeState = owned
				|| (remoteAuthority.GUID == synchInfo.guid && NetworkManager::IsSenior(synchInfo.guid, currentAuthority.GUID))
				|| (!currentAuthority.IsLocal() && NetworkManager::IsSenior(synchInfo.guid));

			if (isAuthoritativeState && !PlayerRegistry::IsLocal(synchInfo.authority))
			{
				IComponent::SerialiseMode mode = synchInfo.full ? IComponent::All : IComponent::Changes;
				auto& continuous = synchInfo.continuous;
				auto& occasional = synchInfo.occasional;

				//FSN_ASSERT(owned || synchInfo.authority != 0 || NetworkManager::IsSenior(synchInfo.guid));

				if (!owned && remoteAuthority.GUID == synchInfo.guid)
				{
					entity->SetAuthority(synchInfo.authority);
					//std::stringstream str; str << (uint32_t)synchInfo.authority;
					//SendToConsole("accepting remote authority " + str.str());
				}

				if (continuous)
					EntitySerialisationUtils::DeserialiseContinuous(*continuous, entity, mode);
				if (occasional)
				{
					EntitySerialisationUtils::DeserialiseOccasional(*occasional, m_SentStates[entity->GetID()], entity, mode);
				}
			}
			// Make sure the local peer isn't giving auth. to remote peers that don't want it
			if (currentAuthority.GUID == synchInfo.guid && remoteAuthority.GUID != currentAuthority.GUID)
				entity->SetAuthority(0);

			m_ReceivedStates.erase(_where);

			return true;
		}

		return false;
	}

	void EntitySynchroniser::EnqueueInactive(PlayerID viewer, ObjectID id, const std::shared_ptr<RakNet::BitStream>& state)
	{
		RakNet::RakNetGUID guid = PlayerRegistry::GetPlayer(viewer).GUID;
		m_ToInform[guid].push_back(std::make_pair(id, state));
	}

	bool EntitySynchroniser::Enqueue(EntityPtr &entity)
	{
		if (!entity->IsSyncedEntity())
			return false;

		if (!m_TEMPQueuedEntities.insert(entity->GetID()).second)
			return false;

		const bool arbitor = NetworkManager::ArbitratorIsLocal();
		const bool isOwnedLocally = PlayerRegistry::IsLocal(entity->GetOwnerID());
		const bool isUnderLocalAuthority = isOwnedLocally || PlayerRegistry::IsLocal(entity->GetAuthority());
		const bool isUnderDefaultAuthority = entity->GetOwnerID() == 0 && entity->GetAuthority() == 0;

		if (!isOwnedLocally)
		{
			m_EntitiesToReceive.push_back(entity);
		}
		if (isUnderLocalAuthority || isUnderDefaultAuthority)
		{
			// Calculate priority
			unsigned int priority;

			if (isOwnedLocally && m_PlayerInputs->GetInputsForPlayer(entity->GetOwnerID())->HasChanged())
			{
				m_ImportantEntities.push_back(entity);
			}
			else
			{
				priority = entity->GetSkippedPacketsCount();
				if (isOwnedLocally)
					priority *= 1000;
				else if (isUnderLocalAuthority)
					priority *= 2;

				m_EntityPriorityQueue.insert(std::make_pair(priority, entity));
			}

			// Obviously the packet might not be skipped, but if it is actually sent
			//  it's skipped-count gets reset to zero, so the following operation will
			//  be over-ruled
			entity->PacketSkipped();
		}

		return true;
	}

	static RakNet::Time nextPacketTime = 0;

	void EntitySynchroniser::ProcessQueue(EntityManager* entity_manager, EntityFactory* factory)
	{
		// TODO: OnDisconected handler (need to add a signal or something for that) that removes jitter buffer
		if (m_UseJitterBuffer)
			ProcessJitterBuffer();

		for (auto it = m_EntitiesToReceive.begin(), end = m_EntitiesToReceive.end(); it != end; ++it)
			ReceiveSync(*it, entity_manager, factory);

		m_EntitiesToReceive.clear();

		// Process states for inactive entities
		for (auto it = m_ReceivedStates.begin(), end = m_ReceivedStates.end(); it != end; ++it)
		{
			using namespace EntitySerialisationUtils;

			const ObjectID id = it->first;
			const auto& state = it->second;

			//m_Archivist->Store(id, state.continuous, state.occasional);

			if (state.continuous)
			{
				auto result = DeserialisePosition(*state.continuous, Vector2(), 0.f);
				if (result.first)
					m_CameraSynchroniser->SetCameraPosition(id, result.second);
			}
			else if (state.occasional)
			{
				auto result = DeserialisePosition(*state.occasional, Vector2(), 0.f);
				if (result.first)
					m_CameraSynchroniser->SetCameraPosition(id, result.second);
			}
		}

		// TODO: remote data from m_ToInform that has been superceeded by remote data

		m_TEMPQueuedEntities.clear();

		if (m_PacketDataBudget < s_MaxDataPerTick * 2)
			m_PacketDataBudget += s_MaxDataPerTick;
		if (m_PacketDataBudget < -int64_t(s_MaxDataPerTick * 2))
			m_PacketDataBudget = -int64_t(s_MaxDataPerTick * 2);

		SendPackets();

		++m_SendTick;
	}

	//void EntitySynchroniser::Send()
	//{
	//	RakNetwork* network = NetworkManager::getSingleton().GetNetwork();

	//	if (m_ImportantMove)
	//	{
	//		network->SendAsIs(To::Populace(), &m_PacketData, HIGH_PRIORITY, RELIABLE_ORDERED, CID_ENTITYSYNC);
	//	}
	//	else
	//	{
	//		// TODO: send continious data in a separate packet to occasional data
	//		//  Continious: HIGH_PRIORITY, UNRELIABLE_SEQUENCED
	//		//  Occasional: LOW_PRIORITY, RELIABLE_ORDERED
	//		network->SendAsIs(To::Populace(), &m_PacketData, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, CID_ENTITYSYNC);
	//	}

	//	m_PacketData.Reset();
	//}

	static void readState(RakNet::BitStream& sourceStr, std::shared_ptr<RakNet::BitStream>& state)
	{
		if (state)
			state->Reset();
		else
			state = std::make_shared<RakNet::BitStream>();

		uint16_t dataLength;
		sourceStr.Read(dataLength);
		sourceStr.Read(*state, dataLength);
#ifdef _DEBUG
		FSN_ASSERT(state->ReadBit());
		state->ResetReadPointer();
#endif
	}

#ifdef _DEBUG
	static size_t jb_size = 0;
#endif

	void EntitySynchroniser::HandlePacket(RakNet::Packet *packet)
	{
		auto bitStream = std::make_shared<RakNet::BitStream>(packet->data, packet->length, true);

		Profiling::getSingleton().AddTime("Incomming Packets", (unsigned long)1);

		if (m_UseJitterBuffer)
		{
			// Ignore message ID
			bitStream->IgnoreBytes(sizeof(unsigned char));

			SendTick_t remoteTick;
			bitStream->Read(remoteTick);

			auto& jitterBuffer = m_JitterBuffers[packet->guid];

			bitStream->ResetReadPointer();

			JitterBufferPacket jitterPacket;
			//jitterPacket.guid = packet->guid;
			//jitterPacket.timestamp = timestamp;
			jitterPacket.tick = remoteTick;
			jitterPacket.data = /*std::move*/(bitStream);
			jitterBuffer.buffer.push_back(/*std::move*/(jitterPacket));

			// Prevent the jitterbuffer from growing out of control:
			while (jitterBuffer.buffer.size() > 60)
				jitterBuffer.buffer.pop_front();

#ifdef _DEBUG
			//FSN_ASSERT(jitterBuffer.buffer.size() < 50);
			if (std::abs<int>((int)std::max(jb_size, jitterBuffer.buffer.size()) - (int)std::min(jb_size, jitterBuffer.buffer.size())) > 2)
			{
				jb_size = jitterBuffer.buffer.size();
				std::stringstream str; str << jb_size;
				SendToConsole("Jitter Buffer for " + std::string(packet->guid.ToString()) + " changed size by > 2: " + str.str());
			}
#endif
		}
		else
		{
			ProcessPacket(packet->guid, *bitStream);
		}
	}

	void EntitySynchroniser::ProcessPacket(const RakNet::RakNetGUID& guid, RakNet::BitStream& bitStream)
	{
		//bitStream.IgnoreBytes(sizeof(unsigned char) + sizeof(RakNet::Time));

		Profiling::getSingleton().AddTime("Packets Processed", (unsigned long)1);

		unsigned char type;
		bitStream.Read(type);
		// Read the tick for this packet
		SendTick_t tick;
		bitStream.Read(tick);

		switch (type)
		{
		case MTID_IMPORTANTMOVE:
			{
				//std::stringstream tickStr; tickStr << tick;
				//SendToConsole("Processing important move at " + tickStr.str());
				// Get Input data
				unsigned short playerCount;
				bitStream.Read(playerCount);
				//if (playerCount == 0)
				//	SendToConsole(" No changes?");
				for (unsigned short pi = 0; pi < playerCount; pi++)
				{
					PlayerID player;
					bitStream.Read(player);

					auto playerInput = m_PlayerInputs->GetInputsForPlayer(player);
					if (playerInput)
						playerInput->Deserialise(&bitStream);
				}
			}
		case MTID_ENTITYMOVE:
			{
				const bool includesCon = bitStream.ReadBit();
				const bool includesOca = bitStream.ReadBit();

				unsigned short entityCount;
				if (includesCon)
				{
					auto& lastTick = m_RemoteTicks[guid];

					bitStream.Read(entityCount);
					for (unsigned short i = 0; i < entityCount; i++)
					{
						ObjectID entityID;
						bitStream.Read(entityID);

						PlayerID authority;
						bitStream.Read(authority);

						StateData state;
						auto& info = state;

						info.full = true; // indicates full data (not just changes)

						readState(bitStream, info.continuous);

						if (tick > lastTick)
						{
							auto& existingState = m_ReceivedStates[entityID];
							// Only override the state if the new peer has seniority over the old one
							if (guid == existingState.guid || NetworkManager::IsSenior(guid, existingState.guid))
							{
								existingState.conTick = tick;//DeltaTime::GetTick() + (tick - lastTick);
								existingState.continuous = state.continuous;
								existingState.guid = guid;
								existingState.authority = authority;
							}
						}
						//else
						//	SendToConsole("Old state, ignored");
					}

					lastTick = tick;
				}
				if (includesOca)
				{
					bitStream.Read(entityCount);
					for (unsigned short i = 0; i < entityCount; i++)
					{
						ObjectID entityID;
						bitStream.Read(entityID);

						PlayerID authority;
						bitStream.Read(authority);

						auto& existingState = m_ReceivedStates[entityID];

						// Accept states from peers with higher authority
						if (guid == existingState.guid || NetworkManager::IsSenior(guid, existingState.guid))
						{
							existingState.full = true; // indicates full data (not just changes)

							readState(bitStream, existingState.occasional);

							existingState.guid = guid;
							existingState.authority = authority;
							existingState.ocaTick = tick;

							//boost::crc_32_type crc;
							//crc.process_bytes(existingState.occasional->GetData(), existingState.occasional->GetNumberOfBytesUsed());
							//m_SentStates[packet->guid][entityID] = crc.checksum();
						}
						else
						{
							StateData trashState;
							readState(bitStream, trashState.occasional);
						}
					}
				}
			}
			break;
		}
	}

	void EntitySynchroniser::ProcessJitterBuffer()
	{
		const auto sendDt = DeltaTime::GetDeltaTime() * 1000.0;
		auto timeNow = RakNet::GetTime();
		for (auto it = m_JitterBuffers.begin(), end = m_JitterBuffers.end(); it != end; ++it)
		{
			const auto& guid = it->first;
			const auto lastPopTime = it->second.lastPopTime;
			const auto popRate = it->second.popRate;
			auto& jitterBuffer = it->second.buffer;
			auto& jitterBufferState = it->second;

			if (!jitterBuffer.empty()/* && timeNow >= (RakNet::Time)(lastPopTime + sendDt * popRate + 0.5)*/)
			{
				auto newEnd = jitterBuffer.begin(), end = jitterBuffer.end();
				auto firstTickToProcess = newEnd->tick;

				unsigned int numTicksToProcess = 1;

				Profiling::getSingleton().AddTime("Buffer size", (unsigned long)jitterBuffer.size());

				auto bufferLength = sendDt * (jitterBuffer.back().tick - firstTickToProcess);
				if ((jitterBufferState.filling && bufferLength < m_JitterBufferTargetLength) || bufferLength < m_JitterBufferTargetLength * 0.5)
				{
#ifdef _DEBUG
					const bool started = !jitterBufferState.filling;
#endif
					jitterBufferState.filling = true;
					jitterBufferState.emptying = false;

					auto ticksBetweenLess = ((m_JitterBufferTargetLength - bufferLength) / m_JitterBufferTargetLength) * 4;
					if (++jitterBufferState.lastTickSkipped >= ticksBetweenLess)
					{
						--numTicksToProcess; // slow down to fill up the buffer
						jitterBufferState.lastTickSkipped = 0;
					}

#ifdef _DEBUG
					if (started)
					{
						std::stringstream str; str << ((ticksBetweenLess > 0) ? (1 / ticksBetweenLess) : std::numeric_limits<double>::infinity());
						SendToConsole("Jitter buffer too small: refilling at " + str.str() + "x speed");
					}
#endif
				}
				else if((jitterBufferState.emptying && bufferLength > m_JitterBufferTargetLength * 1.5) || bufferLength > m_JitterBufferTargetLength * 2)
				{
#ifdef _DEBUG
					const bool started = !jitterBufferState.emptying;
#endif
					it->second.filling = false;
					it->second.emptying = true;

					auto ticksBetweenExtra = (1 / ((bufferLength - m_JitterBufferTargetLength) / m_JitterBufferTargetLength)) * 4;
					if (++jitterBufferState.lastTickSkipped >= ticksBetweenExtra)
					{
						++numTicksToProcess; // speed up to fill the buffer
						jitterBufferState.lastTickSkipped = 0;
					}

#ifdef _DEBUG
					if (started)
					{
						std::stringstream str; str << ticksBetweenExtra;
						SendToConsole("Jitter buffer too big: emptying at " + str.str() + "x speed");
					}
#endif
				}
				else
				{
#ifdef _DEBUG
					if (jitterBufferState.filling)
					{
						std::stringstream str; str << bufferLength;
						SendToConsole("Done filling. Buffer length is now " + str.str());
					}
					if (jitterBufferState.emptying)
					{
						std::stringstream str; str << bufferLength;
						SendToConsole("Done emptying. Buffer length is now " + str.str());
					}
#endif
					jitterBufferState.filling = false;
					jitterBufferState.emptying = false;
					jitterBufferState.lastTickSkipped = 0;
				}
				if (numTicksToProcess > 0)
				{
					unsigned int ticksProcessed = 0;
					auto tickToProcess = firstTickToProcess;
					do
					{
						const auto& front = *newEnd;
						//if (RakNet::LessThan(timeNow, front.timestamp + m_JitterBufferTargetLength))
						//	break;
						if (front.tick > tickToProcess)
						{
							++tickToProcess;
							if (++ticksProcessed >= numTicksToProcess)
								break;
						}
						ProcessPacket(guid, *front.data);
						++newEnd;
					} while (newEnd != end);
					jitterBuffer.erase(jitterBuffer.begin(), newEnd);
				}
			}
		}
	}

	// TODO: set domain modes (and / or replace domains with mode flags in entities?)

	EntityManager::EntityManager(InputManager *input_manager, EntitySynchroniser *entity_synchroniser, StreamingManager *streaming)
		: m_InputManager(input_manager),
		m_EntitySynchroniser(entity_synchroniser),
		m_StreamingManager(streaming),
		m_UpdateBlockedFlags(0),
		m_DrawBlockedFlags(0),
		m_EntitiesLocked(false),
		m_ClearWhenAble(false),
		m_ReferenceTokens(1)
	{
		for (size_t i = 0; i < s_EntityDomainCount; ++i)
			m_DomainState[i] = DS_ALL;

		m_ActivationEventConnection = m_StreamingManager->SignalActivationEvent.connect(std::bind(&EntityManager::OnActivationEvent, this, _1));
		m_RemoteActivationEventConnection = m_StreamingManager->SignalRemoteActivationEvent.connect(std::bind(&EntityManager::OnRemoteActivationEvent, this, _1));
	}

	EntityManager::~EntityManager()
	{
		m_ActivationEventConnection.disconnect();
		m_RemoteActivationEventConnection.disconnect();
	}

	void EntityManager::CompressIDs()
	{
		IDEntityMap::iterator it = m_Entities.begin();
		ObjectID nextSequentialId = 1;
		for (IDEntityMap::iterator end = m_Entities.end(); it != end; ++it)
		{
			if (it->first != nextSequentialId)
			{
				m_Entities[nextSequentialId] = it->second;
				it->second->SetID(nextSequentialId);
			}

			++nextSequentialId;
		}

		// Reset the ID collection
		//m_UnusedIds.takeAll(nextSequentialId);

		m_Entities.erase(it, m_Entities.end());
	}

	void EntityManager::EnableDefaultNameGeneration(bool enable)
	{
		m_GenerateDefaultNames = true;
	}

	bool EntityManager::IsGeneratingDefaultNames() const
	{
		return m_GenerateDefaultNames;
	}

	void EntityManager::AddEntity(EntityPtr &entity)
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex);

		if (entity->GetName().empty() || entity->GetName() == "default")
			entity->_notifyDefaultName(generateName(entity));

		//if (entity->GetID() != 0)
		//{
		//	IDEntityMap::iterator _where = m_Entities.find(entity->GetID());
		//	if (_where != m_Entities.end())
		//		FSN_EXCEPT(ExCode::InvalidArgument, "An entity with the ID " + boost::lexical_cast<std::string>(entity->GetID()) + " already exists");

		//	m_Entities.insert(_where, std::make_pair( entity->GetID(), entity ));
		//}
		//else
		//	m_PseudoEntities.insert(entity);

		//if (!entity->GetName().empty()) // TODO: log a warning about this (empty name is kind of an error)
		//	m_EntitiesByName[entity->GetName()] = entity;

		//m_StreamingManager->AddEntity(entity);
		//m_EntitySynchroniser->OnEntityActivated(entity);
		
		// Immeadiately activate this entity if it isn't within a streaming domain
		//if (!CheckState(entity->GetDomain(), DS_STREAMING))
		{
			m_NewEntitiesToActivate.push(entity);
		}

		//entity->SetPropChangedQueue(&m_PropChangedQueue);
	}

	void EntityManager::RemoveEntity(const EntityPtr &entity)
	{
		// Mark the entity so it will be removed from the active list, and other secondary containers
		entity->MarkToRemove();

		m_EntitiesToRemove.push(entity);
	}

	void EntityManager::RemoveEntityNamed(const std::string &name)
	{
		RemoveEntity(GetEntity(name));
	}

	void EntityManager::RemoveEntityById(ObjectID id)
	{
		RemoveEntity(GetEntity(id));
	}

	void EntityManager::ReplaceEntity(ObjectID id, EntityPtr &entity)
	{
		if (m_EntitiesLocked)
			FSN_EXCEPT(ExCode::NotImplemented, "EntityManager is currently updating: Can't replace Entities while updating");
		
		// Make sure the entity to be inserted has the given ID
		entity->SetID(id);

		// Generate a name if necessary
		if (entity->GetName() == "default")
			entity->_notifyDefaultName(generateName(entity));

		// Try to insert the entity - overwrite if another is present
		EntityPtr &value = m_Entities[id];
		if (value.get() != NULL)
		{
			// Erase the existing Entity from the name map
			m_EntitiesByName.erase(value->GetName());
			// Mark the entity so it will be removed from it's update domain
			value->MarkToRemove();
		}
		// Replace the entry in the ID map (referenced by 'value') with the new entity
		value = entity;
		// Add the entity to the other indexes
		m_EntitiesByName[entity->GetName()] = entity;

		m_EntitySynchroniser->OnEntityActivated(entity);
	}

	void EntityManager::RenameEntity(EntityPtr &entity, const std::string &new_name)
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex);

		NameEntityMap::iterator _where = m_EntitiesByName.find(entity->GetName());
		if (_where != m_EntitiesByName.end())
		{
			m_EntitiesByName.erase(_where);

			entity->SetName(new_name);
			m_EntitiesByName.insert( make_pair(new_name, entity) );
		}
	}

	void EntityManager::RenameEntity(const std::string &current_name, const std::string &new_name)
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex);

		NameEntityMap::iterator _where = m_EntitiesByName.find(current_name);
		if (_where != m_EntitiesByName.end())
		{
			EntityPtr entity = _where->second;
			m_EntitiesByName.erase(_where);

			entity->SetName(new_name);
			m_EntitiesByName.insert( make_pair(new_name, entity) );
		}
	}

	bool isNamed(EntityManager::IDEntityMap::value_type &element, const std::string &name)
	{
		return element.second->GetName() == name;
	}

	EntityPtr EntityManager::GetEntity(const std::string &name, bool throwIfNotFound) const
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex, false);

		//IDEntityMap::const_iterator _where = std::find_if(m_Entities.begin(), m_Entities.end(), std::bind(&isNamed, _1, name));
		NameEntityMap::const_iterator _where = m_EntitiesByName.find(name);
		if (_where == m_EntitiesByName.end())
			if (throwIfNotFound)
				FSN_EXCEPT(ExCode::InvalidArgument, std::string("There is no entity called: ") + name);
			else
				return EntityPtr();
		return _where->second;
	}

	EntityPtr EntityManager::GetEntity(ObjectID id, bool load) const
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex, false);

		IDEntityMap::const_iterator _where = m_Entities.find(id);
		if (_where == m_Entities.end())
		{
			if (load)
			{
				if (!m_StreamingManager->ActivateEntity(id))
					FSN_ASSERT_FAIL("Entity doesn't exist, so it can't be loaded");
			}
			return EntityPtr();
		}
		return _where->second;
	}

	const EntityManager::IDEntityMap &EntityManager::GetEntities() const
	{
		return m_Entities;
	}

	//const EntityManager::EntitySet &EntityManager::GetPseudoEntities() const
	//{
	//	return m_PseudoEntities;
	//}

	//EntityArray &EntityManager::GetDomain(EntityDomain idx)
	//{
	//	return m_EntitiesToUpdate[idx];
	//}

	bool EntityManager::AddTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			return AddTag(entity, tag);
		}
		catch (InvalidArgumentException &)
		{
			return false;
		}
	}

	bool EntityManager::AddTag(EntityPtr entity, const std::string &tag)
	{
		entity->AddTag(tag);
		return true;
	}

	void EntityManager::RemoveTag(const std::string &entity_name, const std::string &tag)
	{
		try
		{
			EntityPtr entity = GetEntity(entity_name);
			RemoveTag(entity, tag);
		}
		catch (InvalidArgumentException &)
		{
		}
	}

	void EntityManager::RemoveTag(EntityPtr entity, const std::string &tag)
	{
		try
		{
			entity->RemoveTag(tag);
		}
		catch (InvalidArgumentException &) // The given tag doesn't exist
		{
		}
	}

	bool EntityManager::CheckTag(const std::string &entity_name, const std::string &tag)
	{
		EntityPtr entity = GetEntity(entity_name);
		if (entity != NULL)
			return entity->CheckTag(tag);
		return false;
	}

	inline bool EntityManager::IsBlocked(EntityPtr entity, const BlockingChangeMap &tags)
	{
		for (BlockingChangeMap::const_iterator it = tags.begin(), end = tags.end(); it != end; ++it)
		{
			if (!it->second && entity->CheckTag(it->first))
				return true;
		}
		return false;
	}

	void EntityManager::PauseEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = false;
	}

	void EntityManager::ResumeEntitiesWithTag(const std::string &tag)
	{
		m_ChangedUpdateStateTags[tag] = true;
	}

	void EntityManager::HideEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = false;
	}

	void EntityManager::ShowEntitiesWithTag(const std::string &tag)
	{
		m_ChangedDrawStateTags[tag] = true;
	}

	void EntityManager::RemoveEntitiesWithTag(const std::string &tag)
	{
		for (IDEntityMap::iterator it = m_Entities.begin(), end = m_Entities.end(); it != end; ++it)
		{
			if (it->second->CheckTag(tag))
				RemoveEntity(it->second);
		}
	}

	void EntityManager::clearEntities(bool real_only)
	{
		if (m_EntitiesLocked)
			std::for_each(m_ActiveEntities.begin(), m_ActiveEntities.end(), [](const EntityPtr &entity){ entity->MarkToRemove(); });
		else
			m_ActiveEntities.clear();

		m_EntitiesByName.clear();
		m_Entities.clear();
		//m_PseudoEntities.clear();

		//m_UnusedIds.freeAll();

		m_ChangedUpdateStateTags.clear();
		m_ChangedDrawStateTags.clear();
	}

	void EntityManager::Clear()
	{
		clearEntities(false);
	}

	void EntityManager::ClearSyncedEntities()
	{
		clearEntities(true);
	}

	void EntityManager::ClearDomain(EntityDomain idx)
	{
		//EntityArray &domain = m_EntitiesToUpdate[idx];
		//for (EntityArray::iterator it = domain.begin(), end = domain.end(); it != end; ++it)
		//{
		//	EntityPtr &entity = *it;

		//	entity->MarkToRemove();
		//	if (entity->IsPseudoEntity())
		//		m_PseudoEntities.erase(entity);
		//	else
		//	{
		//		m_UnusedIds.freeID(entity->GetID());
		//		m_Entities.erase(entity->GetID());
		//	}
		//	m_EntitiesByName.erase(entity->GetName());
		//}
		//// If the manager isn't updating, we can immeadiately clear the
		////  domain (otherwise it will be cleared next time it is updated
		////  due to the MarkToRemove() call above)
		//if (!m_EntitiesLocked)
		//	domain.clear();
	}

	static bool hasNoActiveReferences(EntityPtr& entity)
	{
		bool allMarked = true;

		//tbb::concurrent_queue<EntityPtr> stack;
		std::deque<EntityPtr> stack;

		stack.push_back(entity);

		EntityPtr ref;
		//while (stack.try_pop(ref))
		while (!stack.empty())
		{
			ref.swap(stack.back());
			stack.pop_back();

			ref->SetGCFlag(true);

			if (!ref->IsMarkedToDeactivate())
			{
				allMarked = false;
				stack.clear();
				break;
			}

			{
			tbb::mutex::scoped_lock lock(ref->m_InRefsMutex);
			for (auto it = ref->m_ReferencingEntities.cbegin(), end = ref->m_ReferencingEntities.cend(); it != end; ++it)
			{
				auto& referencingEntity = (*it).lock();
				// Not being able to lock means that this entity has not only been deactivated, but also
				//  destroyed, so it is clearly no longer a valid reference (doesn't need to be checked)
				if (referencingEntity && !referencingEntity->GetGCFlag())
					stack.push_back(referencingEntity);
			}
			}
		}

		// Clear the GC flag
		stack.push_back(entity);
		while (!stack.empty())
		{
			ref.swap(stack.back());
			stack.pop_back();

			ref->SetGCFlag(false);

			{
			tbb::mutex::scoped_lock lock(ref->m_InRefsMutex);
			for (auto it = ref->m_ReferencingEntities.cbegin(), end = ref->m_ReferencingEntities.cend(); it != end; ++it)
			{
				auto& referencingEntity = (*it).lock();
				if (referencingEntity && referencingEntity->GetGCFlag())
					stack.push_back(referencingEntity);
			}
			}
		}

		return allMarked;
	}

	void EntityManager::updateEntities(EntityArray &entityList, float split)
	{
		bool entityRemoved = false;

		//auto playerAddedEvents = m_PlayerAddedEvents;
		//m_PlayerAddedEvents.clear();

		EntityArray::iterator it = entityList.begin(),
			end = entityList.end();
		while (it != end)
		{
			EntityPtr &entity = *it;

			if (entity->GetTagFlags() & m_ToDeleteFlags)
			{
				RemoveEntity(entity); // Remove the entity from the lookup maps, and mark it to be removed from the active list
			}
			// Check for reasons to remove the
			//  entity from the active list
			if (entity->IsMarkedToRemove() || entity->IsMarkedToDeactivate())
			{
				if (entity->IsMarkedToRemove())
					entityRemoved = true;

				if ((*it)->IsActive())
					m_EntitiesToDeactivate.push_back(*it);

				// Keep entities active untill they are no longer referenced
				if (!entity->IsReferenced() || hasNoActiveReferences(entity))
				{
					m_EntitiesUnreferenced.push_back(*it);

					entity->RemoveDeactivateMark();

					it = entityList.erase(it);
					end = entityList.end();

					continue;
				}
			}
			
			{
				// Also make sure the entity isn't blocked by a flag
				if ((entity->GetTagFlags() & m_UpdateBlockedFlags) == 0)
				{
					EntityDomain domainIndex = entity->GetDomain();

					if (CheckState(domainIndex, DS_STREAMING))
						m_StreamingManager->OnUpdated(entity, split);

					if (CheckState(domainIndex, DS_SYNCH))
						m_EntitySynchroniser->Enqueue(entity);
				}

				// Next entity
				++it;
			}
		}

		// Clear the ToDeleteFlags
		m_ToDeleteFlags = 0;

		if (entityRemoved) // Do a full GC cycle if entities were removed
		{
			ScriptManager::getSingleton().GetEnginePtr()->GarbageCollect();
		}
	}

	void EntityManager::ProcessActivationQueues()
	{
		// Process newly added components
		{
			std::pair<EntityPtr, ComponentPtr> toActivate;
			while (m_ComponentsToAdd.try_pop(toActivate))
			{
				if (toActivate.first->IsStreamedIn())
				{
					m_ComponentsToActivate.push_back(toActivate);
				}
			}
		}

		// Process newly added entities
		{
			EntityPtr entityToActivate;
			while (m_NewEntitiesToActivate.try_pop(entityToActivate))
			{
				if (CheckState(entityToActivate->GetDomain(), DS_STREAMING))
					m_StreamingManager->AddEntity(entityToActivate);
				else
					m_EntitiesToActivate.push_back(entityToActivate);
			}
		}

		// Dectivate entities
		for (auto it = m_EntitiesToDeactivate.begin(), end = m_EntitiesToDeactivate.end(); it != end; ++it)
		{
			deactivateEntity(*it);
		}
		m_EntitiesToDeactivate.clear();
		// Drop local references
		for (auto it = m_EntitiesUnreferenced.begin(), end = m_EntitiesUnreferenced.end(); it != end; ++it)
		{
			dropEntity(*it);
		}
		m_EntitiesUnreferenced.clear();

		// Process removed entities
		{
			EntityPtr entityToRemove;
			while (m_EntitiesToRemove.try_pop(entityToRemove))
			{
				removeEntity(entityToRemove);
			}
		}

		// Activate entities
		{
		auto it = m_EntitiesToActivate.begin(), end = m_EntitiesToActivate.end();
		while (it != end)
		{
			if (attemptToActivateEntity(*it))
			{
				//FSN_ASSERT(std::find(m_ActiveEntities.begin(), m_ActiveEntities.end(), *it) == m_ActiveEntities.end());
				auto& entity = (*it);

				entity->StreamIn();
				m_ActiveEntities.push_back(entity);

				m_EntitySynchroniser->OnEntityActivated(entity);

				if (entity->IsSyncedEntity())
					m_Entities[entity->GetID()] = entity;

				if (entity->GetName() == "default")
					entity->SetName(generateName(entity));
				if (!entity->GetName().empty())
					m_EntitiesByName[entity->GetName()] = entity;

				it = m_EntitiesToActivate.erase(it);
				end = m_EntitiesToActivate.end();
			}
			else
				++it;
		}
		}
		// Activate components
		{
		auto it = m_ComponentsToActivate.begin(), end = m_ComponentsToActivate.end();
		while (it != end)
		{
			auto worldEntry = m_EntityFactory->m_ComponentInstancers.find(it->second->GetType());
			if (attemptToActivateComponent(worldEntry->second, it->second))
			{
				it = m_ComponentsToActivate.erase(it);
				end = m_ComponentsToActivate.end();
			}
			else
				++it;
		}
		}
	}

	void EntityManager::ProcessActiveEntities(float dt)
	{
		//m_EntitiesLocked = true;

		//m_EntitySynchroniser->BeginPacket();

		updateEntities(m_ActiveEntities, dt);

		//m_EntitySynchroniser->EndPacket();
		//m_EntitySynchroniser->Send();

		//m_EntitiesLocked = false;
	}

	void EntityManager::UpdateActiveRegions()
	{
		m_StreamingManager->Update();
	}

	void EntityManager::queueEntityToActivate(const EntityPtr& entity)
	{

		// It's possible that the entity was marked to deactivate, then reactivated before it was updated,
		//  so that mark must be removed
		if (entity->IsMarkedToDeactivate())
			entity->RemoveDeactivateMark();
		else
		{
#ifdef _DEBUG
		FSN_ASSERT(std::find(m_EntitiesToActivate.begin(), m_EntitiesToActivate.end(), entity) == m_EntitiesToActivate.end());
#endif
			m_EntitiesToActivate.push_back(entity);
		}
	}

	bool EntityManager::prepareEntity(const EntityPtr &entity)
	{
		bool allAreReady = true;
		for (auto it = entity->GetComponents().begin(), end = entity->GetComponents().end(); it != end; ++it)
		{
			auto& com = *it;
			if (com->GetReadyState() == IComponent::NotReady)
			{
				auto _where = m_EntityFactory->m_ComponentInstancers.find( com->GetType() );
				if (_where != m_EntityFactory->m_ComponentInstancers.end())
				{
					com->SetReadyState(IComponent::Preparing);
					_where->second->Prepare(com);
					allAreReady &= com->IsReady();
				}
				else
					FSN_EXCEPT(InvalidArgumentException, "Unknown component type (this would be impossable if I had planned ahead successfully, but alas)");
			}
		}
		return allAreReady;
	}

	bool EntityManager::attemptToActivateEntity(const EntityPtr &entity)
	{
		bool allAreActive = true;

		//allAreActive &= entity->m_UnloadedReferencedEntities.empty();
		//auto& refedEnts = entity->m_UnloadedReferencedEntities;
		//for (auto it = refedEnts.begin(), end = refedEnts.end(); it != end;)
		//{
		//	ObjectID id = it->first;
		//	auto refedEntity = GetEntity(id);
		//	if (!refedEntity)
		//	{
		//		//size_t cellIndex = *m_EntityDirectory.find(id);
		//		// m_StreamingManager (or archivist) should store the id's of entities against cell-indicies as they are unloaded.
		//		// ActivateEntity returns false if the entity is not known, in which case it can be ignored
		//		allAreActive &= !m_StreamingManager->ActivateEntity(id);
		//	}
		//	else
		//	{
		//		allAreActive &= true;
		//		entity->HoldReference(refedEntity);
		//		it = refedEnts.erase(it);
		//		end = refedEnts.end();
		//		continue;
		//	}
		//	++it;
		//}
		//if (!allAreActive)
		//	return false;

		for (auto it = entity->GetComponents().begin(), end = entity->GetComponents().end(); it != end; ++it)
		{
			auto& com = *it;
			auto _where = m_EntityFactory->m_ComponentInstancers.find( com->GetType() );
			if (_where != m_EntityFactory->m_ComponentInstancers.end())
			{
				allAreActive &= attemptToActivateComponent(_where->second, com);
			}
			else
				FSN_EXCEPT(InvalidArgumentException, "Unknown component type (this would be impossable if I had planned ahead properly, but alas)");
		}
		return allAreActive;
	}

	bool EntityManager::attemptToActivateComponent(const std::shared_ptr<ISystemWorld>& world, const ComponentPtr& component)
	{
		if (component->GetReadyState() == IComponent::NotReady)
		{
			component->SetReadyState(IComponent::Preparing);
			world->Prepare(component);
		}
		if (component->IsReady())
		{
			world->OnActivation(component);
			component->GetParent()->OnComponentActivated(component); // Tell the siblings
			component->SetReadyState(IComponent::Active);
			component->SynchronisePropertiesNow();
			return true;
		}
		else if (component->IsActive())
		{
			return true;
		}
		return false;
	}
	
	//bool EntityManager::attemptToActivateEntity(bool& has_begun_preparing, const EntityPtr &entity)
	//{
	//	if (!has_begun_preparing)
	//	{
	//		has_begun_preparing = true;
	//		if (!prepareEntity(entity))
	//			return false; // Not all ready, return and try again next time this is called
	//	}
	//	else
	//	{
	//		// Check unreadyness
	//		for (auto it = entity->GetComponents().begin(), end = entity->GetComponents().end(); it != end; ++it)
	//		{
	//			auto& com = *it;
	//			if (!com->IsReady())
	//				return false;
	//		}
	//	}
	//	// Getting here means all components are ready: activate!
	//	activateEntity(entity);
	//	return true;
	//}
	
	void EntityManager::activateEntity(const EntityPtr &entity)
	{
		if (!entity->IsStreamedIn())
		{
			FSN_ASSERT(std::find(m_ActiveEntities.begin(), m_ActiveEntities.end(), entity) == m_ActiveEntities.end());

			for (auto it = entity->GetComponents().begin(), end = entity->GetComponents().end(); it != end; ++it)
			{
				auto& com = *it;
				auto _where = m_EntityFactory->m_ComponentInstancers.find( com->GetType() );
				if (_where != m_EntityFactory->m_ComponentInstancers.end())
				{
					_where->second->OnActivation(com);
				}
				else
					FSN_EXCEPT(InvalidArgumentException, "I made a mistake, because am dum");
			}
			entity->StreamIn();

			m_ActiveEntities.push_back(entity);
		}
	}

	void EntityManager::deactivateEntity(const EntityPtr& entity)
	{
		entity->StreamOut();

		m_EntitySynchroniser->OnEntityDeactivated(entity);

		//entity->RemoveDeactivateMark(); // Otherwise the entity will be immeadiately re-deactivated if it is activated later
		for (auto cit = entity->GetComponents().begin(), cend = entity->GetComponents().end(); cit != cend; ++cit)
		{
			auto& com = *cit;
			auto _where = m_EntityFactory->m_ComponentInstancers.find( com->GetType() );
			if (_where != m_EntityFactory->m_ComponentInstancers.end())
			{
				_where->second->OnDeactivation(com);
				com->SetReadyState(IComponent::NotReady);
			}
			else
				FSN_EXCEPT(InvalidArgumentException, "Herp derp");
		}
	}

	void EntityManager::dropEntity(const EntityPtr& entity)
	{
		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex);

		if (entity->IsSyncedEntity())
			m_Entities.erase(entity->GetID());

		m_EntitiesByName.erase(entity->GetName());

		m_StreamingManager->OnUnreferenced(entity);
	}

	void EntityManager::removeEntity(const EntityPtr& entity)
	{
		FSN_ASSERT(!entity->IsActive());

		tbb::spin_rw_mutex::scoped_lock lock(m_EntityListsMutex);
		if (entity->IsSyncedEntity())
			m_Entities.erase(entity->GetID());

		m_EntitiesByName.erase(entity->GetName());

		m_StreamingManager->RemoveEntity(entity);

		if (entity->IsSyncedEntity())
		{
			ObjectID id = entity->GetID();

			tbb::spin_rw_mutex::scoped_lock lock(m_StoredReferencesMutex);
			// Free all tokens for references from this entity (clearly it can't use them anymore)
			auto& refsFromContainer = m_StoredReferences.get<1>();
			auto refsFromRemovedEntity = refsFromContainer.equal_range(id);
			{
			tbb::spin_rw_mutex::scoped_lock lock(m_ReferenceTokensMutex);
			for (; refsFromRemovedEntity.first != refsFromRemovedEntity.second; ++refsFromRemovedEntity.first)
			{
				m_ReferenceTokens.freeID( refsFromRemovedEntity.first->token );
			}
			}
			refsFromContainer.erase(refsFromRemovedEntity.first, refsFromRemovedEntity.second);

			// Invalidate all references to this entity
			auto& refsToContainer = m_StoredReferences.get<2>();
			refsToContainer.erase(id);
		}
	}

	void EntityManager::queueEntityToSynch(ObjectID id, PlayerID viewer, const std::shared_ptr<RakNet::BitStream>& state)
	{
		m_EntitySynchroniser->EnqueueInactive(viewer, id, state);
	}

	void EntityManager::OnComponentAdded(const EntityPtr &entity, const ComponentPtr& component)
	{
		m_ComponentsToAdd.push(std::make_pair(entity, component));
	}

	void EntityManager::OnActivationEvent(const ActivationEvent &ev)
	{
		// TODO: post stream-out / stream in events (messages) to the entity in question
		switch (ev.type)
		{
		case ActivationEvent::Activate:
			queueEntityToActivate(ev.entity);
			break;
		case ActivationEvent::Deactivate:
			ev.entity->MarkToDeactivate();
			break;
		}
	}

	void EntityManager::OnRemoteActivationEvent(const RemoteActivationEvent &ev)
	{
		switch (ev.type)
		{
		case ActivationEvent::Activate:
			queueEntityToSynch(ev.entity, ev.viewer, ev.state);
			break;
		}
	}

	uint32_t EntityManager::StoreReference(ObjectID from, ObjectID to)
	{
		if (from > 0 && to > 0)
		{
			tbb::spin_rw_mutex::scoped_lock tlock(m_ReferenceTokensMutex);
			StoredReference r;
			r.from = from;
			r.to = to;
			auto token = r.token = m_ReferenceTokens.getFreeID();

			// TODO: the first few bits of the token could be the StoredReferences database-id (to ensure that references
			//  are being loaded from the same save-game that the manager currently has loaded)

			tbb::spin_rw_mutex::scoped_lock lock(m_StoredReferencesMutex);
			m_StoredReferences.insert(r);

			return token;
		}
		else
			return 0;
	}

	ObjectID EntityManager::RetrieveReference(uint32_t token)
	{
		//{
		//tbb::spin_rw_mutex::scoped_lock lock(m_ReferenceTokensMutex);
		//m_ReferenceTokens.freeID(token);
		//}

		tbb::spin_rw_mutex::scoped_lock lock(m_StoredReferencesMutex, false);
		auto& tokens = m_StoredReferences.get<0>();
		auto entry = tokens.find(token);
		if (entry != tokens.end())
		{
			//FSN_ASSERT(entry->from == from);
			//tokens.erase(entry);
			return entry->to;
		}
		else
			return 0;
	}

	void EntityManager::DropReference(uint32_t token)
	{
		{
		tbb::spin_rw_mutex::scoped_lock lock(m_ReferenceTokensMutex);
		m_ReferenceTokens.freeID(token);
		}

		tbb::spin_rw_mutex::scoped_lock lock(m_StoredReferencesMutex);
		auto& tokens = m_StoredReferences.get<0>();
		auto entry = tokens.find(token);
		if (entry != tokens.end())
		{
			tokens.erase(token);
		}
	}

	void EntityManager::OnPlayerAdded(unsigned int local_index, PlayerID net_id)
	{
		m_PlayerAddedEvents.emplace_back(std::make_pair(local_index, net_id));
	}

	void EntityManager::Draw(Renderer *renderer, const ViewportPtr &viewport, size_t layer)
	{
		//renderer->Draw(m_ActiveEntities, viewport, layer);
	}

	EntityArray& EntityManager::GetActiveEntities()
	{
		return m_ActiveEntities;
	}

	void EntityManager::SetDomainState(EntityDomain domain_index, char modes)
	{
		m_DomainState[domain_index] = modes;
	}

	bool EntityManager::CheckState(EntityDomain domain_index, DomainState mode) const
	{
		return (m_DomainState[domain_index] & mode) == mode;
	}

	char EntityManager::GetDomainState(EntityDomain domain_index) const
	{
		return m_DomainState[domain_index];
	}

	void EntityManager::SetModule(ModulePtr module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( std::bind(&EntityManager::OnModuleRebuild, this, _1) );
	}

	void EntityManager::OnModuleRebuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			ev.manager->RegisterGlobalObject("EntityManager entity_manager", this);
		}
	}

	void EntityManager_AddEntity(asIScriptObject *script_entity, EntityManager *obj)
	{
		//obj->AddEntity( EntityPtr(ScriptedEntity::GetAppObject(script_entity)) );
		//script_entity->Release();
	}

	void EntityManager_RemoveEntity(const std::string &name, EntityManager *obj)
	{
		obj->RemoveEntityNamed(name);
	}

	void EntityManager_RemoveEntity(ObjectID id, EntityManager *obj)
	{
		obj->RemoveEntityById(id);
	}

	void EntityManager_RemoveEntity(asIScriptObject *script_entity, EntityManager *obj)
	{
		//obj->RemoveEntity( EntityPtr(ScriptedEntity::GetAppObject(script_entity)) );
		//script_entity->Release();
	}

	void EntityManager::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<EntityManager>("EntityManager", engine);

		r = engine->RegisterObjectMethod("EntityManager",
			"void add(IEntity@)",
			asFUNCTIONPR(EntityManager_AddEntity, (asIScriptObject*, EntityManager*), void), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"void add(Entity@)",
			asMETHOD(EntityManager, AddEntity), asCALL_THISCALL);
		
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(const string &in)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (const std::string &, EntityManager*), void), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(uint16)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (ObjectID, EntityManager*), void), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(IEntity@)",
			asFUNCTIONPR(EntityManager_RemoveEntity, (asIScriptObject*, EntityManager*), void), asCALL_CDECL_OBJLAST);

		r = engine->RegisterObjectMethod("EntityManager",
			"void remove(Entity@)",
			asMETHODPR(EntityManager, RemoveEntity, (const EntityPtr&), void), asCALL_THISCALL);
	}

	std::string EntityManager::generateName(const EntityPtr &entity)
	{
		std::stringstream stream;
		if (entity->IsPseudoEntity())
			stream << "__entity_pseudo_" << reinterpret_cast<uintptr_t>(entity.get());
		else
			stream << "__entity_id_" << entity->GetID();
		return stream.str();
	}

	void EntityManager::updateTags(EntityPtr &entity) const
	{
		for (BlockingChangeMap::const_iterator it = m_ChangedUpdateStateTags.begin(), end = m_ChangedUpdateStateTags.end(); it != end; ++it)
		{
			if (it->second)
				entity->_notifyResumedTag(it->first);
			else
				entity->_notifyHiddenTag(it->first);
		}

		for (BlockingChangeMap::const_iterator it = m_ChangedDrawStateTags.begin(), end = m_ChangedDrawStateTags.end(); it != end; ++it)
		{
			if (it->second)
				entity->_notifyShownTag(it->first);
			else
				entity->_notifyHiddenTag(it->first);
		}
	}

}
