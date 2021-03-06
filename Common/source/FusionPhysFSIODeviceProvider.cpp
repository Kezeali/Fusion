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

#include "PrecompiledHeaders.h"

#include "FusionPhysFSIODeviceProvider.h"
#include <ClanLib/Core/IOData/file.h>
#include <ClanLib/Core/System/exception.h>

/////////////////////////////////////////////////////////////////////////////
// construction:

PhysFSIODeviceProvider::PhysFSIODeviceProvider(PHYSFS_File* handle)
: m_Handle(handle)
{
	init();
}

PhysFSIODeviceProvider::~PhysFSIODeviceProvider()
{
	deinit();
}

/////////////////////////////////////////////////////////////////////////////
// attributes:

int PhysFSIODeviceProvider::get_size() const
{
	return (int)PHYSFS_fileLength(m_Handle);
}

int PhysFSIODeviceProvider::get_position() const
{
	return (int)PHYSFS_tell(m_Handle);
}

/////////////////////////////////////////////////////////////////////////////
// operations:

int PhysFSIODeviceProvider::send(const void *data, int len, bool send_all)
{
	PHYSFS_sint64 lenRet = PHYSFS_write(m_Handle, data, sizeof(char), len);
	if (lenRet == -1)
		throw clan::Exception("PhysFS: Failed to write to file");

	return (int)lenRet;
}

int PhysFSIODeviceProvider::receive(void *buffer, int size, bool receive_all)
{
	if (size == 0)
		return 0;

	PHYSFS_sint64 lenRet = PHYSFS_read(m_Handle, buffer, 1, size);
	if (lenRet == -1)
		throw clan::Exception("PhysFS: Failed to read file"  + std::string(PHYSFS_getLastError()));

	return (int)lenRet;
}

int PhysFSIODeviceProvider::peek(void *data, int len)
{
	PHYSFS_sint64 startPos = PHYSFS_tell(m_Handle);
	PHYSFS_sint64 lengthRead = PHYSFS_read(m_Handle, data, 1, len);
	int seekRet = PHYSFS_seek(m_Handle, startPos);

	if (lengthRead == -1 || seekRet == -1)
		throw clan::Exception("PhysFS: Failed to read file or return to original position during peek operation");

	return (int)lengthRead;
}

bool PhysFSIODeviceProvider::seek(int seek_pos, clan::IODevice::SeekMode mode)
{
	PHYSFS_uint64 absolute_pos = 0;
	
	switch (mode)
	{
	case clan::IODevice::seek_set:
		if (seek_pos < 0)
			return false;
		absolute_pos = static_cast<PHYSFS_uint64>(seek_pos);
		break;

	case clan::IODevice::seek_cur:
		{
			const auto curPos = PHYSFS_tell(m_Handle);
			if (curPos == -1) // Indicates error
				return false;
 			const auto newPos = curPos + seek_pos;
			if (newPos >= 0)
				absolute_pos = static_cast<PHYSFS_uint64>(newPos);
			else
				return false;
		}
		break;

	case clan::IODevice::seek_end:
		{
			const auto end = PHYSFS_fileLength(m_Handle);
			if (seek_pos <= 0 && end + seek_pos > 0)
				absolute_pos = PHYSFS_fileLength(m_Handle) + seek_pos;
			else
				return false;
		}
		break;
	}

	return PHYSFS_seek(m_Handle, absolute_pos) != 0;
}

clan::IODeviceProvider *PhysFSIODeviceProvider::duplicate()
{
	PhysFSIODeviceProvider *new_provider = new PhysFSIODeviceProvider(m_Handle);
	return new_provider;
}


void PhysFSIODeviceProvider::init()
{
}

void PhysFSIODeviceProvider::deinit()
{
	if (PHYSFS_close(m_Handle) == 0)
		throw clan::Exception("File failed to close");
}
