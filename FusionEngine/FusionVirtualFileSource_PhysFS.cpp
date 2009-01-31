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
// VirtualFileSource_PhysFS Construction:

VirtualFileSource_PhysFS::VirtualFileSource_PhysFS()
: m_Index(0)
{
}

VirtualFileSource_PhysFS::~VirtualFileSource_PhysFS()
{
}

/////////////////////////////////////////////////////////////////////////////
// VirtualFileSource_PhysFS Attributes:

CL_String VirtualFileSource_PhysFS::get_path() const
{
	return m_Path;
}

/////////////////////////////////////////////////////////////////////////////
// VirtualFileSource_PhysFS Operations:

CL_IODevice VirtualFileSource_PhysFS::open_file(const CL_String &wfilename,
	CL_File::OpenMode mode,
	unsigned int access,
	unsigned int share,
	unsigned int flags)
{
	PHYSFS_File *file = NULL;
	std::string filename(wfilename.begin(), wfilename.end());
	
	if (access & CL_File::access_write)
	{
		if (!PHYSFS_exists(filename.c_str()) || mode == CL_File::create_always)
			file = PHYSFS_openWrite(filename.c_str());
		else
			file = PHYSFS_openAppend(filename.c_str());
	}
	else if (access & CL_File::access_read)
		file = PHYSFS_openRead(filename.c_str());

	if (file == NULL)
		throw CL_Exception(cl_format("VirtualFileSource_PhysFS: Couldn't open the file '%1' with the requested access", wfilename));

	return CL_IODevice(new PhysFSIODeviceProvider(file));
}


bool VirtualFileSource_PhysFS::initialize_directory_listing(const CL_String &path)
{
	char **files = PHYSFS_enumerateFiles("");
	if (files != NULL)
	{
		char **i;
		for (i = files; *i != NULL; i++)
			m_FileList.push_back( CL_String(*i) );

		PHYSFS_freeList(files);
	}
	m_Index = 0;
	
	return !m_FileList.empty();
}

bool VirtualFileSource_PhysFS::next_file(CL_VirtualDirectoryListingEntry &entry)
{
	if( m_FileList.empty() ) 
		return false;

	if( m_Index > m_FileList.size() - 1 )
		return false;

	std::string cfilename(m_FileList[m_Index].begin(), m_FileList[m_Index].end());

	bool isDirectory = PHYSFS_isDirectory(cfilename.c_str()) != 0;

	entry.set_filename(m_FileList[m_Index]);
	entry.set_readable(true);
	entry.set_directory(isDirectory);
	entry.set_hidden(false); // why would you care?
	entry.set_writable(false); 
	m_Index++;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// VirtualFileSource_PhysFS Implementation:
