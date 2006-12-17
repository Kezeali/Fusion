/*
  Copyright (c) 2006 Fusion Project Team

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

#include "FusionOutputSource_PhysFS.h"

OutputSource_PhysFS::OutputSource_PhysFS(const std::string &filename)
{
	m_Filename = filename;
	m_PhysFile = NULL;
	open();
}

OutputSource_PhysFS::OutputSource_PhysFS(const OutputSource_PhysFS *source)
{
	m_Filename = source->m_Filename;
	m_PhysFile = NULL;

	open();
	PHYSFS_seek(m_PhysFile, PHYSFS_tell(source->m_PhysFile));
}

OutputSource_PhysFS::~OutputSource_PhysFS()
{
	close();
}

int OutputSource_PhysFS::tell() const
{
	return m_Position;
}

int OutputSource_PhysFS::size() const
{
	return m_Position;
}


// Int64
void OutputSource_PhysFS::write_int64(cl_int64 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeSLE64(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeSBE64(m_PhysFile, data);
	}	
}

void OutputSource_PhysFS::write_uint64(cl_uint64 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeULE64(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeUBE64(m_PhysFile, data);
	}	
}

// Int32
void OutputSource_PhysFS::write_int32(cl_int32 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeSLE32(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeSBE32(m_PhysFile, data);
	}	
}

void OutputSource_PhysFS::write_uint32(cl_uint32 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeULE32(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeUBE32(m_PhysFile, data);
	}	
}

// Int16
void OutputSource_PhysFS::write_int16(cl_int16 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeSLE16(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeSBE16(m_PhysFile, data);
	}	
}

void OutputSource_PhysFS::write_uint16(cl_uint16 data)
{
	if (little_endian_mode)
	{
		PHYSFS_writeULE16(m_PhysFile, data);
	}
	else
	{
		PHYSFS_writeUBE16(m_PhysFile, data);
	}	
}


int OutputSource_PhysFS::write(const void *data, int size)
{
	return PHYSFS_write(m_PhysFile, data, sizeof(char), size);
}

void OutputSource_PhysFS::open()
{
	if (m_PhysFile != NULL) return;


	m_PhysFile = PHYSFS_openWrite(m_Filename.c_str());

	if (m_PhysFile == NULL)
	{
		throw CL_Error(PHYSFS_getLastError());
	}

}

void OutputSource_PhysFS::close()
{
	if (m_PhysFile == NULL) return;
	PHYSFS_close(m_PhysFile);

	m_PhysFile = NULL;
}

CL_OutputSource *OutputSource_PhysFS::clone()
{
	return new OutputSource_PhysFS(this);
}
