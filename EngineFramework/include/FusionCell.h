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
*  File Author:
*    Elliot Hayward
*/

#ifndef H_FusionCell
#define H_FusionCell

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionTypes.h"

#include "FusionEntity.h"

//#include <tbb/concurrent_unordered_map.h>
#include <tbb/spin_mutex.h>
#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <memory>
#include <vector>

namespace FusionEngine
{

	//#define STREAMING_USEMAP
	//#define FSN_CELL_HISTORY

	struct CellEntry
	{
		enum State { Inactive = 0, Active = 1, Waiting = 2 };
		State active;
		bool pendingDeactivation;
		float pendingDeactivationTime;

		float x, y;

		ObjectID id;
		std::shared_ptr<RakNet::BitStream> data;

		CellEntry()
			: active(Inactive),
			pendingDeactivation(false),
			pendingDeactivationTime(0.0f),
			x(0.0f),
			y(0.0f)
		{}
	};

	class Cell
	{
	public:
		Cell()
			: inRange(false),
			objects(0)
		{
			loaded = false;
			active_entries = 0;
			waiting = Ready;
		}

		void Reset()
		{
			inRange = false;
			objects.clear();
			loaded = false;
			active_entries = 0;
			waiting = Ready;
		}

	private:
		Cell(const Cell& other)
			: objects(other.objects),
			active_entries(other.active_entries),
			loaded(other.loaded)
		{
			waiting = Ready;
		}

		Cell(Cell&& other)
			: objects(std::move(other.objects)),
			active_entries(other.active_entries),
			loaded(other.loaded)
		{}

		Cell& operator= (const Cell& other)
		{
			objects = other.objects;
			active_entries = other.active_entries;
			loaded = other.loaded;
			return *this;
		}

		Cell& operator= (Cell&& other)
		{
			objects = std::move(other.objects);
			active_entries = other.active_entries;
			loaded = other.loaded;
			return *this;
		}
	public:
#ifdef STREAMING_USEMAP
		typedef std::map<Entity*, CellEntry> CellEntryMap;
		CellEntryMap objects;
#else
		typedef std::pair<EntityPtr, CellEntry> EntityEntryPair;
		typedef std::vector<EntityEntryPair> CellEntryMap;
		CellEntryMap objects;
#endif
		tbb::atomic<unsigned int> active_entries;
		void EntryUnreferenced() { FSN_ASSERT(active_entries > 0); --active_entries; AddHist("EntryUnreferenced", active_entries); }
		void EntryReferenced() { ++active_entries; AddHist("EntryReferenced", active_entries); }
		bool IsActive() const { return active_entries > 0; }

		tbb::atomic<bool> loaded;
		bool IsLoaded() const { return loaded; }

		bool inRange;

		enum WaitingState { Ready, Retrieve, Store };
		tbb::atomic<WaitingState> waiting;

		bool IsRetrieved() const { return loaded && waiting != Store; }

		typedef tbb::spin_mutex mutex_t;
		mutex_t mutex;

#ifdef FSN_PROFILING_ENABLED
		bool loadTimeRecorded;
		tbb::tick_count timeRequested;
#endif

#ifdef FSN_CELL_HISTORY
		mutex_t historyMutex;
		std::vector<std::string> history;
#endif
		void AddHist(const std::string& hist, unsigned int num = -1);
	};

}

#endif
