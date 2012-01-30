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

#include "FusionAssert.h"

#include "FusionFilesystemDataSource.h"

#include <boost/filesystem.hpp>
#include <deque>
#include <physfs.h>

namespace bfs = boost::filesystem;

namespace FusionEngine
{

	inline std::string TypeToString(FilesystemDataSource::Entry::Type type)
	{
		switch (type)
		{
		case FilesystemDataSource::Entry::Directory:
			return "d";
		case FilesystemDataSource::Entry::File:
			return "f";
		case FilesystemDataSource::Entry::SymbolicLink:
			return "l";
		default:
			return "";
		};
	}

	inline std::string PreprocessPath(const std::string& table)
	{
		const std::string writeDirStr = "#write_dir";
		if (table.compare(0, writeDirStr.length(), writeDirStr) == 0)
			return PHYSFS_getWriteDir() + table.substr(writeDirStr.length());
		else
			return table;
	}

	void FilesystemDataFormatter::FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data)
	{
		if (!raw_data.empty())
		{
			auto type =	raw_data[0];
			if (type == "d")
				formatted_data += "<div class=\"folder_icon\" />";
			else if (type == "f")
				formatted_data += "<div class=\"file_icon\" />";
			else if (type == "l")
				formatted_data += "<div class=\"link_icon\" />";
		}
	}

	FilesystemDataSource::Entry FilesystemDataSource::ConstructFilesystemEntry(const std::string& path) const
	{
		Entry entry;

		// Check for type identifiers
		const std::string physfs_dev = "physfs:";
		const std::string native_dev = "native:";
		bool forcePhysfs = false;
		std::string actualPath = path;
		if (path.compare(0, physfs_dev.length(), physfs_dev) == 0)
		{
			forcePhysfs = true;
			actualPath.erase(0, physfs_dev.length());
		}
		else if (path.compare(0, native_dev.length(), native_dev) == 0)
		{
			actualPath.erase(0, native_dev.length());
		}

		entry.path = actualPath;
		entry.name = bfs::path(actualPath).filename().string();

		if (bfs::exists(actualPath) && !forcePhysfs)
		{
			entry.filesystem = Entry::Native;

			if (bfs::is_directory(actualPath))
				entry.type = Entry::Directory;
			else if (bfs::is_symlink(actualPath))
				entry.type = Entry::SymbolicLink;
			else if (bfs::is_regular_file(actualPath))
				entry.type = Entry::File;
			else
				entry.type = Entry::None;
		}
		else
		{
			entry.filesystem = Entry::Physfs;

			if (PHYSFS_isDirectory(actualPath.c_str()))
				entry.type = Entry::Directory;
			else if (PHYSFS_isSymbolicLink(actualPath.c_str()))
				entry.type = Entry::SymbolicLink;
			else
				entry.type = Entry::File;
		}

		return entry;
	}

	void FilesystemDataSource::Populate(FilesystemDataSource::Entry& entry) const
	{
		if (entry.filesystem == Entry::Physfs)
		{
			auto files = PHYSFS_enumerateFiles(entry.path.c_str());
			for (auto it = files; *it != NULL; ++it)
			{
				entry.children.push_back(ConstructFilesystemEntry(entry.path + "/" + *it));
			}
			PHYSFS_freeList(files);
		}
		else if (entry.filesystem == Entry::Native)
		{
			FSN_ASSERT(bfs::exists(entry.path));
			for (bfs::directory_iterator it(entry.path); it != bfs::directory_iterator(); it++)
			{
				entry.children.push_back(ConstructFilesystemEntry(it->path().generic_string()));
			}
		}
	}

	FilesystemDataSource::Entry FilesystemDataSource::GetFilesystemEntryRecursive(const std::string& path)
	{
		auto entry = ConstructFilesystemEntry(path);
		std::deque<Entry> toList;
		toList.push_back(entry);
		while (!toList.empty())
		{
			auto& current = toList.back();
			toList.pop_back();
			if (current.type == Entry::Directory)
			{
				Populate(current);
				toList.insert(toList.end(), current.children.begin(), current.children.end());
			}
		}
		return entry;
	}

	void FilesystemDataSource::Check(FilesystemDataSource::Entry& entry)
	{
		Rocket::Core::String table(entry.path.data(), entry.path.data() + entry.path.length());

		Entry copy;
		copy.filesystem = entry.filesystem;
		copy.type = entry.type;
		copy.name = entry.name;
		copy.path = entry.path;
		// Short-circuit removed native folders
		if (copy.filesystem == Entry::Native && !bfs::exists(entry.path))
		{
			NotifyRowRemove(table, 0, (int)entry.children.size());
			return;
		}
		Populate(copy);

		int first = 0, num = 0;
		bool run = false;
		for (auto it = copy.children.begin(), end = copy.children.end(), it2 = entry.children.begin(), end2 = entry.children.end(); it != end && it2 != end2; ++it, ++it2)
		{
			if (run)
			{
				if (*it == *it2)
				{
					NotifyRowChange(table, first, num);
				}
				else
					++num;
			}
			else if (*it != *it2)
			{
				first = std::distance(entry.children.begin(), it2);
				num = 1;
				run = true;
			}
		}
		if (run)
		{
			NotifyRowChange(table, first, num);
		}
		if (copy.children.size() > entry.children.size())
			NotifyRowAdd(table, (int)entry.children.size(), (int)(copy.children.size() - entry.children.size()));
		if (copy.children.size() < entry.children.size())
			NotifyRowRemove(table, (int)copy.children.size(), (int)(entry.children.size() - copy.children.size()));

		entry.children.swap(copy.children);
	}

	FilesystemDataSource::Entry FilesystemDataSource::AquireEntry(const std::string& path, bool check)
	{
		auto r = listings.insert(std::make_pair(path, ConstructFilesystemEntry(path)));
		if (r.second)
			Populate(r.first->second);
		else if (check)
		{
			Check(r.first->second);
		}
		return r.first->second;
	}

	void FilesystemDataSource::GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns)
	{
		auto entry = AquireEntry(PreprocessPath(table.CString()));

		FSN_ASSERT(row_index >= 0 && (size_t)row_index < entry.children.size());

		const auto& file = entry.children[(size_t)row_index];
		for (auto it = columns.begin(), end = columns.end(); it != end; ++it)
		{
			if (*it == "type")
				row.push_back(TypeToString(file.type).c_str());
			if (*it == "filename")
				row.push_back(file.name.c_str());
		}
	}

	int FilesystemDataSource::GetNumRows(const Rocket::Core::String& table)
	{
		auto entry = AquireEntry(PreprocessPath(table.CString()), true);

		return (int)entry.children.size();
	}

	bool FilesystemDataSource::IsDirectory(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (row_index >= 0 && entry.children.size() > (size_t)row_index)
			return entry.children[row_index].type == Entry::Directory;
		else
			return false;
	}

	std::string FilesystemDataSource::GetFilename(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (row_index >= 0 && entry.children.size() > (size_t)row_index)
			return entry.children[row_index].name;
		else
			return "";
	}

	std::string FilesystemDataSource::GetPath(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (row_index >= 0 && entry.children.size() > (size_t)row_index)
			return entry.children[row_index].path;
		else
			return "";
	}
	
	std::string FilesystemDataSource::PreproPath(const std::string& table) const
	{
		return FusionEngine::PreprocessPath(table);
	}

}
