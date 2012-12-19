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

#include "PrecompiledHeaders.h"

#include "FusionNetworkedRegionCellCache.h"

#include "FusionNetDestinationHelpers.h"
#include "FusionNetworkManager.h"
#include "FusionNetworkTypes.h"
#include "FusionRakNetwork.h"
#include "FusionRegionFileLoadedCallbackHandle.h"

#include <BitStream.h>
#include <physfs.h>

namespace FusionEngine
{

	class FunctorPacketHandler : public PacketHandler
	{
	public:
		FunctorPacketHandler(std::function<void (RakNet::Packet*)> fn)
			: functor(fn)
		{
		}

		virtual void HandlePacket(RakNet::Packet* packet) 
		{
			functor(packet);
		}

		std::function<void (RakNet::Packet*)> functor;
	};

	NetworkedRegionCellCache::NetworkedRegionCellCache(const std::string& cache_path, int32_t region_size, bool readonly)
		: RegionCellCache(cache_path, region_size, readonly)
	{
		m_MapCellPacketHandler = std::make_shared<FunctorPacketHandler>([](RakNet::Packet* packet)
		{
		});
		NetworkManager::getSingleton().Subscribe(MTID_MAPCELL_REQUEST, this);
		NetworkManager::getSingleton().Subscribe(MTID_MAPCELL, this);
	}

	NetworkedRegionCellCache::~NetworkedRegionCellCache()
	{
		NetworkManager::getSingleton().Unsubscribe(MTID_MAPCELL_REQUEST, this);
		NetworkManager::getSingleton().Unsubscribe(MTID_MAPCELL, this);
	}

	void NetworkedRegionCellCache::HandlePacket(RakNet::Packet* packet)
	{
		auto network = NetworkManager::getSingleton().GetNetwork();

		RakNet::BitStream receivedData(packet->data, packet->length, false);

		unsigned char type;
		receivedData.Read(type);
		switch (type)
		{
		case MTID_MAPCELL_MODTIME_REQUEST:
			{
				CellCoord_t cell_coord;
				receivedData.Read(cell_coord);

				RegionCoord_t regionCoord = cellToRegionCoord(cell_coord.x, cell_coord.y);

				// Tell the requester our mod time
				RakNet::BitStream sendData;

				PHYSFS_sint64 modTime = PHYSFS_getLastModTime(GetRegionFilePath(regionCoord).c_str());
				sendData.Write(cell_coord);
				sendData.Write(modTime);

				network->Send(
					NetDestination(packet->guid), false,
					MTID_MAPCELL_MODTIME, &sendData,
					MEDIUM_PRIORITY, RELIABLE, CID_FILESYNC);
			}
			break;
		case MTID_MAPCELL_MODTIME:
			{
				CellCoord_t cell_coord;
				receivedData.Read(cell_coord);

				PHYSFS_sint64 modTime;
				receivedData.Read(modTime);

				auto entry = m_CellNetRequests.find(cell_coord);
				if (entry != m_CellNetRequests.end())
				{
					entry->second.responsesReceived.insert(packet->guid);
					if (modTime > entry->second.mostRecentModTime)
					{
						entry->second.peerWithMostRecentModTime = packet->guid;
						entry->second.mostRecentModTime = modTime;
					}

					auto responses = entry->second;
					const bool gotResponsesFromAll = network->AllPeers([responses](const RakNet::RakNetGUID& peer_guid)
					{
						return responses.responsesReceived.find(peer_guid) != responses.responsesReceived.end();
					});

					if (gotResponsesFromAll)
					{
						RequestCellFromPeer(entry->second.peerWithMostRecentModTime, cell_coord);
					}
				}
			}
			break;
		case MTID_MAPCELL_REQUEST:
			{

			}
			break;
		case MTID_MAPCELL:
			{
			}
			break;
		}
	}

	void NetworkedRegionCellCache::RequestCellFromNetwork(const CellCoord_t& coord)
	{
		auto network = NetworkManager::getSingleton().GetNetwork();

		RakNet::BitStream data;
		data.Write(coord);

		PHYSFS_sint64 modTime = PHYSFS_getLastModTime(GetRegionFilePath(coord).c_str());
		data.Write(modTime);

		network->Send(To::Populace(), false, MTID_MAPCELL_MODTIME_REQUEST, &data, MEDIUM_PRIORITY, RELIABLE, CID_FILESYNC);
	}

	void NetworkedRegionCellCache::RequestCellFromPeer(const RakNet::RakNetGUID& guid, const CellCoord_t& coord)
	{
		auto network = NetworkManager::getSingleton().GetNetwork();

		RakNet::BitStream data;
		data.Write(coord);

		PHYSFS_sint64 modTime = PHYSFS_getLastModTime(GetRegionFilePath(coord).c_str());
		data.Write(modTime);

		network->Send(NetDestination(guid), false, MTID_MAPCELL_REQUEST, &data, MEDIUM_PRIORITY, RELIABLE, CID_FILESYNC);
	}

	void NetworkedRegionCellCache::SendCellDataToPeer(const RakNet::RakNetGUID& guid, const CellCoord_t& coord)
	{
		using namespace std::placeholders;

		auto guidCopy = guid;
		auto coordCopy = coord;
		this->GetRawCellStreamForReading(std::bind(&NetworkedRegionCellCache::OnGotCellStreamForSending, this, _1, guidCopy, coordCopy), coord.x, coord.y);
	}

	void NetworkedRegionCellCache::OnGotCellStreamForSending(std::shared_ptr<std::istream> cellDataStream, RakNet::RakNetGUID send_to, CellCoord_t coord)
	{
		auto network = NetworkManager::getSingleton().GetNetwork();

		RakNet::BitStream toSend;
		toSend.Write(coord);
		
		{
			RakNet::BitStream data;

			std::vector<char> buffer(4096);
			while (!cellDataStream->eof())
			{
				auto sectorStart = cellDataStream->tellg();
				cellDataStream->read(buffer.data(), buffer.size());
				auto sectorLength = cellDataStream->tellg() - sectorStart;
				FSN_ASSERT(sectorLength >= 0);
				if (sectorLength > 0)
					data.Write(buffer.data(), (unsigned int)sectorLength);
			}

			toSend.Write(data.GetNumberOfBitsUsed());
			toSend.Write(data);
		}

		network->Send(NetDestination(send_to), false, MTID_MAPCELL, &toSend, MEDIUM_PRIORITY, RELIABLE, CID_FILESYNC);
	}

	void NetworkedRegionCellCache::GetRegionFile(const RegionCellCache::RegionLoadedCallback& loadedCallback, const RegionCellCache::RegionCoord_t& coord, bool load_if_uncached)
	{
		if (!IsCached(coord))
		{
			RequestCellFromNetwork(coord);
		}
		else
		{
			GetRegionFile(loadedCallback, coord, load_if_uncached);
		}
	}

}
