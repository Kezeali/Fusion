/*
  Copyright (c) 2006 FusionTeam

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_Archive
#define Header_FusionEngine_Archive

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "../minizip/unzip.h"

namespace FusionEngine
{
	//! Decompression in-a-box.
	/*!
	 * ATM, you can't set the output path as I'm just putting everthing into 
	 * FusionEngine#TempPath.
	 */
	class Archive
	{
	public:
		//! Basic constructor
		Archive();
		//! Constructor +choose file
		Archive(const std::string &filename);

		//! Destructor
		~Archive();
	public:
		//! Associates an archive with this object.
		void Open(const std::string &filename);
		//! Decompresses the archive associated with this object (to TempPath)
		bool Decompress();
		//! Returns a list of paths to the files which have been decompressed by this object
		const StringVector &GetFileList() const;
		//! Finds the path to the file requested if it has been decompressed from this archive
		std::string GetFile(const std::string &file);
		//! Deletes all the files which have been decompressed by this object
		/*!
		 * \remarks MCS - This does nothing (but tells you it :D)
		 * \todo Cross-platform file deletion for Archive#DeleteTemps().
		 */
		void DeleteTemps();

	protected:
		//! List of the files which have been successfully decompressed from this archive.
		StringVector m_FileList;
		//! Path to the archive to decompress
		std::string m_ZipFile;

		//! Extracts a file from archive
		/*!
		 * If the currentfile file is successfully decompressed, this method will add
		 * it its relative path/name to m_FileList.
		 */
		int do_extract_currentfile(
			unzFile uf,
			const int popt_extract_without_path,
			const char* password);

		//! Extracts all files from archive
		int do_extract(
			unzFile uf,
			const int opt_extract_without_path,
			const char* password);

		//! Sets the date of extracted files
		void change_file_date(
			const char *filename,
			uLong dosdate,
			tm_unz tmu_date);

	};
}

#endif
