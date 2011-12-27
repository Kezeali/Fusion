/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionPhysFSIOStream.h"

#include "FusionExceptionFactory.h"

namespace FusionEngine { namespace IO
{

	PhysFSDevice::PhysFSDevice(const std::string& path, OpenMode openmode)
	{
		m_Path = path;
		m_OpenMode = openmode;

		switch (openmode)
		{
		case Read:
			m_File = PHYSFS_openRead(path.c_str());
			break;
		case Write:
			m_File = PHYSFS_openWrite(path.c_str());
			break;
		case Append:
			m_File = PHYSFS_openAppend(path.c_str());
			break;
		}
		if (!m_File)
			FSN_EXCEPT(FileSystemException, cl_format("Can't open %1: %2", path, std::string(PHYSFS_getLastError())));
	}

	void PhysFSDevice::close()
	{
		PHYSFS_close(m_File);
	}

	bool PhysFSDevice::flush()
	{
		return PHYSFS_flush(m_File) != 0;
	}

	std::streamsize PhysFSDevice::read(char* s, std::streamsize n)
	{
		PHYSFS_sint64 ret = PHYSFS_read(m_File, s, 1, PHYSFS_uint32(n));
		if (ret == -1)
			FSN_EXCEPT(FileSystemException, PHYSFS_getLastError());
		return std::streamsize(ret);
	}

	std::streamsize PhysFSDevice::write(const char* s, std::streamsize n)
	{
		PHYSFS_sint64 ret = PHYSFS_write(m_File, s, 1, (PHYSFS_uint32)n);
		if (ret == -1)
			FSN_EXCEPT(FileSystemException, PHYSFS_getLastError());
		return std::streamsize(ret);
	}

	std::streampos PhysFSDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{
		PHYSFS_sint64 pos(off);
		if (way == std::ios_base::cur)
		{
			PHYSFS_sint64 cur = PHYSFS_tell(m_File);
			if (cur == -1)
				FSN_EXCEPT(FileSystemException, PHYSFS_getLastError());
			pos = cur + off;
		}
		else if (way == std::ios_base::end)
		{
			PHYSFS_sint64 end = PHYSFS_fileLength(m_File);
			if (end == -1)
				FSN_EXCEPT(FileSystemException, PHYSFS_getLastError());
			pos = end + off;
		}

		if (PHYSFS_seek(m_File, (PHYSFS_uint64)pos) == 0)
			FSN_EXCEPT(FileSystemException, PHYSFS_getLastError());

		return std::streampos(pos);
	}

} }
