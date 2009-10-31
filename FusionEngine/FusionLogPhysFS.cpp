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
		: m_Open(false),
		m_MaxLength(s_LogMaxLength),
		m_ExtraSpace(s_LogExtraSpace)
	{
	}

	PhysFSLogFile::~PhysFSLogFile()
	{
		Close();
	}

	void PhysFSLogFile::Open(const std::string& filename)
	{
		m_File = PHYSFS_openAppend(filename.c_str());
		if (m_File != NULL)
		{
			m_StartingLength = PHYSFS_tell(m_File);
			if (m_StartingLength > m_MaxLength)
			{
				PHYSFS_close(m_File);
				m_File = NULL;
				m_File = PHYSFS_openWrite(filename.c_str());
				m_StartingLength = 0;
			}
		}
		m_Open = m_File == NULL ? false : true;

		m_CurrentLength = m_StartingLength;
		m_SessionLength = 0;

		m_Filename = filename;
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
		{
			PHYSFS_write( m_File, static_cast<const void*>(entry.c_str()), 1, entry.length() );
			m_CurrentLength += entry.length();
			m_SessionLength += entry.length();
		}
	}

	void PhysFSLogFile::Flush()
	{
		if (m_Open)
		{
			PHYSFS_flush(m_File);
			if (m_CurrentLength > m_MaxLength)
			{
				PHYSFS_close(m_File);
				PHYSFS_File *reread = PHYSFS_openRead(m_Filename.c_str());

				PHYSFS_sint64 seekPosition = m_StartingLength;
				PHYSFS_uint64 keptLength = m_SessionLength;
				if (m_SessionLength > m_MaxLength)
				{
					seekPosition = m_StartingLength + m_ExtraSpace;
					keptLength = m_SessionLength - m_ExtraSpace;
				}

				PHYSFS_sint64 count = -1;
				std::string buffer("");
				if (PHYSFS_seek(reread, seekPosition) != 0)
				{
					buffer.resize((size_t)keptLength, '\0');
					count = PHYSFS_read(reread, static_cast<void*>(&buffer[0]), buffer.size(), 1);
				}

				PHYSFS_close(reread);

				m_File = PHYSFS_openWrite(m_Filename.c_str());

				//if (keptLength < m_SessionLength)
				//{
				//	static const char message[] = "***Log exceeded maximum size and was trimmed to this point***\n\0";
				//	static size_t messageLength = strlen(message);
				//	PHYSFS_write(m_File, &message, 1, messageLength);
				//}
				if (count != -1)
					PHYSFS_write(m_File, static_cast<const void*>(buffer.c_str()), 1, buffer.length());

				m_StartingLength = 0;
				m_CurrentLength = m_SessionLength = buffer.size();
			}
		}
	}

}
