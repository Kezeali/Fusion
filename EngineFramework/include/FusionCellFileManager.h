/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#ifndef H_FusionCellFileManager
#define H_FusionCellFileManager

#include "FusionPrerequisites.h"

#include <string>

namespace FusionEngine
{

	//! Interface for a class that knows where cell-data files are stored (used when compiling maps)
	class CellFileManager
	{
	public:
		virtual ~CellFileManager() {}
		//! Copies the entity database to the given path
		virtual void CopyDatabase(const std::string& dest_path) = 0;
		//! Copies cell data files to the given path
		virtual void CopyCellFiles(const std::string& dest_path) = 0;
	};

}

#endif
