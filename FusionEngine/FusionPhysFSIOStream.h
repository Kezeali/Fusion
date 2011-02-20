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

#ifndef Header_FusionPhysFSIOStream
#define Header_FusionPhysFSIOStream

#if _MSC_VER > 1000
#pragma once
#endif

extern "C"
{
#include <physfs.h>
}

#include <string>
#include <boost/iostreams/stream.hpp>

namespace FusionEngine { namespace IO
{
	enum OpenMode { Read, Write, Append };

	class PhysFSDevice
	{
	public:
		struct category
			: boost::iostreams::seekable_device_tag/*, boost::iostreams::flushable_tag*/, boost::iostreams::closable_tag
		{};
		typedef char char_type;

		PhysFSDevice(const std::string& name, OpenMode openmode = Read);

		void close();

		bool flush();

		std::streamsize read(char* s, std::streamsize n);
		std::streamsize write(const char* s, std::streamsize n);
		std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

	private:
		PHYSFS_File* m_File;
		std::string m_Path;
		OpenMode m_OpenMode;
	};

	typedef boost::iostreams::stream<PhysFSDevice> PhysFSStream;

} }

#endif
