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

#ifndef H_FusionNetworkedRegionCellCache
#define H_FusionNetworkedRegionCellCache

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRegionCellCache.h"

#include "FusionCellSerialisationUtils.h"
#include "FusionPacketHandler.h"

namespace FusionEngine
{

	//! Region-file based cell data source
	class NetworkedRegionCellCache : public RegionCellCache, public PacketHandler
	{
	public:
		//! CTOR
		/*!
		* \param cache_path
		* The absolute path where the region files should be stored
		*
		* \param cells_per_region_square
		* Width & height in number of cells per region file, i.e. 16 makes 16x16 region files.
		*/
		NetworkedRegionCellCache(const std::string& cache_path, int32_t cells_per_region_square = 16, bool readonly = false);

		//! DTOR
		~NetworkedRegionCellCache();

		//! Returns a RegionFile for the given coord
		/*!
		* \param[in] load_if_uncached
		* If false, null will be returned if the region file isn't already cached
		*/
		void GetRegionFile(const RegionLoadedCallback& loadedCallback, const RegionCoord_t& coord, bool load_if_uncached);

		virtual void HandlePacket(RakNet::Packet* packet);

		void OnGotCellStreamForSending(std::shared_ptr<std::istream> cellDataStream, RakNet::RakNetGUID send_to, CellCoord_t coord);

	private:
		std::shared_ptr<PacketHandler> m_MapCellPacketHandler;

		struct ModTimeResponses
		{
			RakNet::RakNetGUID peerWithMostRecentModTime;
			int64_t mostRecentModTime;

			std::set<RakNet::RakNetGUID> responsesReceived;
		};
		std::unordered_map<CellCoord_t, ModTimeResponses, boost::hash<CellCoord_t>> m_CellNetRequests;

		void RequestCellFromNetwork(const CellCoord_t& coord);
		void RequestCellFromPeer(const RakNet::RakNetGUID& guid, const CellCoord_t& coord);

		void SendCellDataToPeer(const RakNet::RakNetGUID& guid, const CellCoord_t& coord);
	};

}

#endif
