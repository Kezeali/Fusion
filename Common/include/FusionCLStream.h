/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionCLStream
#define H_FusionCLStream

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSpriteDefinition.h"

#include "FusionExceptionFactory.h"

//#include "FusionPhysFSIOStream.h"
#include <boost/iostreams/stream.hpp>

#include <yaml-cpp/yaml.h>

namespace FusionEngine { namespace IO
{

	class CLStreamDevice
	{
	public:
		struct category
			: boost::iostreams::seekable_device_tag/*, boost::iostreams::flushable_tag*/, boost::iostreams::closable_tag
		{};
		typedef char char_type;

		CLStreamDevice(CL_IODevice dev);

		void close();

		std::streamsize read(char* s, std::streamsize n);
		std::streamsize write(const char* s, std::streamsize n);
		std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

	private:
		CL_IODevice m_Device;
	};

	typedef boost::iostreams::stream<CLStreamDevice> CLStream;

	inline CLStreamDevice::CLStreamDevice(CL_IODevice dev)
		: m_Device(dev)
	{
		if (m_Device.is_null())
			FSN_EXCEPT(FileSystemException, "Tried to load data from an invalid source");
	}

	inline void CLStreamDevice::close()
	{
		m_Device = CL_IODevice();
	}

	inline std::streamsize CLStreamDevice::read(char* s, std::streamsize n)
	{
		int ret = m_Device.read(static_cast<void*>(s), (int)n);
		return std::streamsize(ret);
	}

	inline std::streamsize CLStreamDevice::write(const char* s, std::streamsize n)
	{
		int ret = m_Device.write(static_cast<const void*>(s), (int)n);
		return std::streamsize(ret);
	}

	inline std::streampos CLStreamDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{
		CL_IODevice::SeekMode clSeekMode;
		switch (way)
		{
		case std::ios_base::cur:
			clSeekMode = CL_IODevice::seek_cur;
			break;
		case std::ios_base::end:
			clSeekMode = CL_IODevice::seek_end;
			break;
		default:
			clSeekMode = CL_IODevice::seek_set;
			break;
		};

		if (m_Device.seek((int)off, clSeekMode))
			FSN_EXCEPT(FileSystemException, "Tried to go to an invalid position within a data-source");

		return std::streampos(m_Device.get_position());
	}

} }

#endif
