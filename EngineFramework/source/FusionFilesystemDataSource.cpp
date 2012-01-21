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

		entry.path = path;
		entry.name = bfs::path(path).filename().string();

		const std::string physfs_dev = "physfs://";
		const std::string native_dev = "native://";
		if (bfs::exists(path) || path.substr(0, native_dev.length()) == native_dev)
		{
			entry.filesystem = Entry::Native;

			if (bfs::is_directory(path))
				entry.type = Entry::Directory;
			else if (bfs::is_symlink(path))
				entry.type = Entry::SymbolicLink;
			else if (bfs::is_regular_file(path))
				entry.type = Entry::File;
			else
				entry.type = Entry::None;
		}
		else
		{
			entry.filesystem = Entry::Physfs;

			if (PHYSFS_isDirectory(path.c_str()))
				entry.type = Entry::Directory;
			else if (PHYSFS_isSymbolicLink(path.c_str()))
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
				entry.children.push_back(ConstructFilesystemEntry(entry.path + PHYSFS_getDirSeparator() + *it));
			}
			PHYSFS_freeList(files);
		}
		else if (entry.filesystem == Entry::Native)
		{
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

	FilesystemDataSource::Entry FilesystemDataSource::AquireEntry(const std::string& path)
	{
		auto r = listings.insert(std::make_pair(path, ConstructFilesystemEntry(path)));
		if (r.second)
			Populate(r.first->second);
		return r.first->second;
	}

	void FilesystemDataSource::GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns)
	{
		auto entry = AquireEntry(PreprocessPath(table.CString()));

		const auto& file = entry.children[row_index];
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
		auto entry = AquireEntry(PreprocessPath(table.CString()));

		return entry.children.size();
	}

	bool FilesystemDataSource::IsDirectory(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (entry.children.size() > row_index)
			return entry.children[row_index].type == Entry::Directory;
		else
			return false;
	}

	std::string FilesystemDataSource::GetFilename(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (entry.children.size() > row_index)
			return entry.children[row_index].name;
		else
			return "";
	}

	std::string FilesystemDataSource::GetPath(const std::string& table, int row_index)
	{
		auto entry = AquireEntry(PreprocessPath(table));
		if (entry.children.size() > row_index)
			return entry.children[row_index].path;
		else
			return "";
	}
	
	std::string FilesystemDataSource::PreproPath(const std::string& table) const
	{
		return FusionEngine::PreprocessPath(table);
	}

}
