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

#ifndef H_FusionFilesystemDataSource
#define H_FusionFilesystemDataSource

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Rocket/Controls/DataFormatter.h>
#include <Rocket/Controls/DataSource.h>

#include <unordered_map>

namespace FusionEngine
{

	class FilesystemDataFormatter : public Rocket::Controls::DataFormatter
	{
	public:
		FilesystemDataFormatter()
			: Rocket::Controls::DataFormatter("filesystem")
		{}

		void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data);
	};

	class FilesystemDataSource : public Rocket::Controls::DataSource
	{
	public:
		FilesystemDataSource()
			: Rocket::Controls::DataSource("filesystem")
		{}

		bool IsDirectory(const std::string& table, int row_index);
		std::string GetFilename(const std::string& table, int row_index);
		std::string GetPath(const std::string& table, int row_index);
		std::string PreproPath(const std::string& table) const;

		void ClearCache() { listings.clear(); }
		void Refresh()
		{
			for (auto it = listings.begin(), end = listings.end(); it != end; ++it)
				Check(it->second);
		}

		struct Entry
		{
			Entry() {}
			Entry(const Entry& other)
				: filesystem(other.filesystem),
				type(other.type),
				name(other.name),
				path(other.path),
				children(other.children)
			{}
			Entry(Entry&& other)
				: filesystem(other.filesystem),
				type(other.type),
				name(std::move(other.name)),
				path(std::move(other.path)),
				children(std::move(other.children))
			{}
			Entry& operator= (const Entry& other)
			{
				filesystem = other.filesystem;
				type = other.type;
				name = other.name;
				path = other.path;
				children = other.children;
				return *this;
			}
			Entry& operator= (Entry&& other)
			{
				filesystem = other.filesystem;
				type = other.type;
				name = std::move(other.name);
				path = std::move(other.path);
				children = std::move(other.children);
				return *this;
			}
			bool operator== (const Entry& other) const
			{
				return filesystem == other.filesystem && type == other.type && name == other.name && path == other.path;
			}
			bool operator!= (const Entry& other) const
			{
				return !(*this == other);
			}
			enum Filesystem { Physfs, Native };
			Filesystem filesystem;
			enum Type { None, Directory, File, SymbolicLink };
			Type type;
			std::string name;
			std::string path;
			std::vector<Entry> children;
		};

	private:
		Entry ConstructFilesystemEntry(const std::string& path) const;
		void Populate(Entry& entry) const;

		Entry GetFilesystemEntryRecursive(const std::string& path);

		void Check(Entry& entry);

		Entry AquireEntry(const std::string& path, bool check = false);

		std::unordered_map<std::string, Entry> listings;

		void GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns);
		int GetNumRows(const Rocket::Core::String& table);

	};

}

#endif
