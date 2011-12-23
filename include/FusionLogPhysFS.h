/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionEngine_PhysFSLogFile
#define H_FusionEngine_PhysFSLogFile

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionLog.h"

#include "PhysFS.h"
//#include "PhysFS++.h"

namespace FusionEngine
{

	static PHYSFS_sint64 s_LogMaxLength = 1048576;
	static PHYSFS_sint64 s_LogExtraSpace = fe_lround(s_LogMaxLength*0.1);

	//! Log file that writes to a PHYSFS file
	/*!
	* \todo Enforce some size limit
	*/
	class PhysFSLogFile : public ILogFile
	{
	public:
		PhysFSLogFile();
		~PhysFSLogFile();

	public:
		std::string GetType() const { return "physfs"; }

	public:
		void Open(const std::string& filename);
		void Close();
		void Write(const std::string& entry);
		void Flush();
		
	protected:
		PHYSFS_File* m_File;
		bool m_Open;

		PHYSFS_sint64 m_MaxLength;
		PHYSFS_sint64 m_ExtraSpace;
		PHYSFS_sint64 m_SessionLength;
		PHYSFS_sint64 m_CurrentLength; // =StartingLength + SessionLength
		PHYSFS_sint64 m_StartingLength;
		std::string m_Filename;
	};

}

#endif