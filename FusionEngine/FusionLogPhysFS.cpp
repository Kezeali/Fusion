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

#include "FusionLogPhysFS.h"

//#include "PhysFS++.h"


namespace FusionEngine
{

	PhysFSLogFile::PhysFSLogFile()
		: m_Open(false)
	{
	}

	PhysFSLogFile::~PhysFSLogFile()
	{
		Close();
	}

	void PhysFSLogFile::Open(const std::string& filename)
	{
		m_File = PHYSFS_openAppend(filename.c_str());
		m_Open = m_File == NULL ? false : true;
		//m_FileStream.open(filename, PhysFS::OM_APPEND);
	}

	void PhysFSLogFile::Close()
	{
		if (m_Open)
		{
			PHYSFS_close(m_File);
			m_Open = false;
		}
	}

	void PhysFSLogFile::Write(const std::string& entry)
	{
		if (m_Open)
			PHYSFS_write( m_File, static_cast<const void*>(entry.c_str()), 1, entry.length() );
		//if (m_FileStream.is_open())
		//	m_FileStream << entry;
	}

	void PhysFSLogFile::Flush()
	{
		if (m_Open)
			PHYSFS_flush(m_File);
		//if (m_FileStream.is_open())
		//	m_FileStream.flush();
	}

}
