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

class clan::VirtualDirectoryListingEntry;


class VirtualFileSource_PhysFS : public clan::VirtualFileSource
{
//! \name Construction
//! \{

public:
	VirtualFileSource_PhysFS();

	~VirtualFileSource_PhysFS();

//! \}
//! \name Operations
//! \{

public:
	//! \brief Open a zip file
	/*! param: filename = The filename to use
	    param: mode = clan::File::OpenMode modes
	    param: access = clan::File::AccessFlags flags
	    param: share = clan::File::ShareFlags flags
	    param: flags = clan::File::Flags flags
	    \return The clan::IODevice*/
	clan::IODevice open_file(
		const std::string &filename,
		clan::File::OpenMode mode = clan::File::open_existing,
		unsigned int access = clan::File::access_read | clan::File::access_write,
		unsigned int share = clan::File::share_all,
		unsigned int flags = 0);

	bool initialize_directory_listing(const std::string &path);

	bool next_file(clan::VirtualDirectoryListingEntry &entry);

	std::string get_path() const;
	std::string get_identifier() const;

//! \}
//! \name Implementation
//! \{

private:
	std::string m_Path;

	std::vector<std::string> m_FileList;

	// Index of the current file being iterated
	//  (why the ClanLib devs didn't give next_file() an 
	//  iterator / index parameter is beyond me)
	unsigned int m_Index;
//! \}
};
