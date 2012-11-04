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

#ifndef H_FusionRegionCellCache
#define H_FusionRegionCellCache

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionHashable.h"
#include "FusionRegionFile.h"
#include "FusionResourcePointer.h"
#include "FusionVector2.h"

#include "FusionCellCache.h"

#include <array>
#include <functional>
#include <memory>
#include <unordered_map>

#include <ClanLib/Core/Math/rect.h>

#include <boost/dynamic_bitset.hpp>

#include <tbb/recursive_mutex.h>
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_map.h"

namespace FusionEngine
{

	class RegionFileLoadedCallbackHandle;

	//! Region-file based cell data source
	class RegionCellCache : public CellDataSource
	{
	public:
		typedef Vector2T<int32_t> RegionCoord_t;
		
		//! CTOR
		/*!
		* \param cache_path
		* The absolute path where the region files should be stored
		*
		* \param cells_per_region_square
		* Width & height in number of cells per region file, i.e. 16 makes 16x16 region files.
		*/
		RegionCellCache(const std::string& cache_path, int32_t cells_per_region_square = 16);

		//! Unload held files (doesn't delete them from disk)
		void DropCache();

		typedef std::function<void (RegionFile*)> RegionLoadedCallback;

		//! Clear the cache for the given file and reload it
		void ReloadRegionFile(const RegionLoadedCallback& loadedCallback, const RegionCoord_t& coord);
		//! Returns a RegionFile for the given coord
		/*!
		* \param[in] load_if_uncached
		* If false, null will be returned if the region file isn't already cached
		*/
		void GetRegionFile(const RegionLoadedCallback& loadedCallback, const RegionCoord_t& coord, bool load_if_uncached);

		//! Callback handler that delivers cell data from loaded regions
		void OnRegionFileLoaded(ResourceDataPtr& resource, const RegionCoord_t& coord);

		int32_t GetRegionSize() const { return m_RegionSize; }

		//! Returns the given cell data
		void GetCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y);
		std::unique_ptr<ArchiveOStream> GetCellStreamForWriting(int32_t cell_x, int32_t cell_y);

		//! Writes data produced by BureaucraticCellBuffer
		void WriteCellData(std::pair<int32_t, int32_t> cellIndex, std::shared_ptr<SmartArrayDevice::DataArray_t> data);

		//! Returns the compressed cell data
		void GetRawCellStreamForReading(const GotCellForReadingCallback& callback, int32_t cell_x, int32_t cell_y);

		//! In edit mode, the cell cache records the maximum and minimum cell coordinates
		void SetupEditMode(bool record_bounds, CL_Rect initial_bounds = CL_Rect());
		//! Gets the recorded bounds of the cache (max/min coords of cells accessed this session)
		CL_Rect GetUsedBounds() const { return m_Bounds; }

	private:
		typedef tbb::concurrent_hash_map<RegionCoord_t, RegionFileLoadedCallbackHandle> CallbackHandles_t;
		CallbackHandles_t m_CallbackHandles;

		typedef tbb::concurrent_unordered_map<RegionCoord_t, ResourcePointer<RegionFile>, boost::hash<RegionCoord_t>> CacheMap_t;
		CacheMap_t m_Cache;
		std::list<RegionCoord_t> m_CacheImportance;
		size_t m_MaxLoadedFiles;
		typedef tbb::spin_mutex CacheMutex_t;
		CacheMutex_t m_CacheMutex;

		std::string m_CachePath;

		int32_t m_RegionSize;

		bool m_EditMode;
		CL_Rect m_Bounds;

		//! Returns the region in which the given cell resides, and converts the coords to region-relative coords
		RegionCoord_t cellToRegionCoord(int32_t* in_out_x, int32_t* in_out_y) const;
	};

}

#endif
