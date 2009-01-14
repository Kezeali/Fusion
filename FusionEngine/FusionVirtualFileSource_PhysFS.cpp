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


#include "Common.h"

#include "FusionVirtualFileSource_PhysFS.h"
#include <ClanLib/Core/IOData/iodevice.h>
#include <ClanLib/Core/IOData/virtual_directory_listing_entry.h>

/////////////////////////////////////////////////////////////////////////////
// CL_VirtualFileSource_Zip Construction:

VirtualFileSource_PhysFS::VirtualFileSource_PhysFS()
: m_Index(0)
{
}

VirtualFileSource_PhysFS::~VirtualFileSource_PhysFS()
{
}

/////////////////////////////////////////////////////////////////////////////
// CL_VirtualFileSource_Zip Attributes:

CL_String VirtualFileSource_PhysFS::get_path() const
{
	return m_Path;
}

/////////////////////////////////////////////////////////////////////////////
// CL_VirtualFileSource_Zip Operations:

CL_IODevice VirtualFileSource_PhysFS::open_file(const CL_String &filename,
	CL_File::OpenMode mode,
	unsigned int access,
	unsigned int share,
	unsigned int flags)
{
	return zip_archive.open_file(filename);
}


bool CL_VirtualFileSource_Zip::initialize_directory_listing(const CL_String &path)
{
	file_list = zip_archive.get_file_list();
	index = 0;
	
	return !file_list.empty();
}

bool CL_VirtualFileSource_Zip::next_file(CL_VirtualDirectoryListingEntry &entry)
{
	if( file_list.empty() ) 
		return false;

	if( index > file_list.size() - 1 )
		return false;

	entry.set_filename(file_list[index].get_archive_filename());
	entry.set_readable(true);
	entry.set_directory(false); // todo
	entry.set_hidden(false); // todo
	entry.set_writable(false); // todo
	index++;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CL_VirtualFileSource_Zip Implementation:
