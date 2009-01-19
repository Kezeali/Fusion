/*
  Copyright (c) 2009 Fusion Project Team

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


	File Author(s):

		Elliot Hayward
*/

#pragma once


#include <ClanLib/Core/IOData/virtual_file_source.h>
#include <ClanLib/Core/IOData/file.h>

#include "FusionPhysFSIODeviceProvider.h"

class CL_VirtualDirectoryListingEntry;


class VirtualFileSource_PhysFS : public CL_VirtualFileSource
{
//! \name Construction
//! \{

public:
	VirtualFileSource_PhysFS();

	~VirtualFileSource_PhysFS();


//! \}
//! \name Attributes
//! \{

public:
	CL_String get_path() const;


//! \}
//! \name Operations
//! \{

public:
	//! \brief Open a zip file
	/*! param: filename = The filename to use
	    param: mode = CL_File::OpenMode modes
	    param: access = CL_File::AccessFlags flags
	    param: share = CL_File::ShareFlags flags
	    param: flags = CL_File::Flags flags
	    \return The CL_IODevice*/
	CL_IODevice open_file(
		const CL_String &filename,
		CL_File::OpenMode mode = CL_File::open_existing,
		unsigned int access = CL_File::access_read | CL_File::access_write,
		unsigned int share = CL_File::share_all,
		unsigned int flags = 0);

	bool initialize_directory_listing(const CL_String &path);

	bool next_file(CL_VirtualDirectoryListingEntry &entry);


//! \}
//! \name Implementation
//! \{

private:
	CL_String m_Path;

	std::vector<CL_String> m_FileList;

	// Index of the current file being iterated
	//  (why the ClanLib devs didn't give next_file() an 
	//  iterator / index parameter is beyond me)
	unsigned int m_Index;
//! \}
};


