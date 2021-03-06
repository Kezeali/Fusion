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

#ifndef FE_PhysFSIODeviceProvider
#define FE_PhysFSIODeviceProvider

#if _MSC_VER > 1000
#pragma once
#endif

#include "ClanLib/Core/IOData/iodevice_provider.h"

#include "physfs.h"

class PhysFSIODeviceProvider : public clan::IODeviceProvider
{
/// \name Construction
/// \{

public:
	PhysFSIODeviceProvider(PHYSFS_File *handle);

	~PhysFSIODeviceProvider();


/// \}
/// \name Attributes
/// \{

public:
	virtual int get_size() const;

	virtual int get_position() const;


/// \}
/// \name Operations
/// \{

public:
	virtual int send(const void *data, int len, bool send_all);

	virtual int receive(void *data, int len, bool receive_all);

	virtual int peek(void *data, int len);

	virtual bool seek(int position, clan::IODevice::SeekMode mode);

	clan::IODeviceProvider *duplicate();


/// \}
/// \name Implementation
/// \{

private:
	void init();

	void deinit();

	PHYSFS_File* m_Handle;
/// \}
};


#endif