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

#ifndef H_FusionSaveDataArchive
#define H_FusionSaveDataArchive

#include "FusionPrerequisites.h"

namespace FusionEngine
{

	//! Store and retreive custom data files
	class SaveDataArchive
	{
	public:
		virtual ~SaveDataArchive() {}

		//! Create a file for storing custom data
		virtual std::unique_ptr<std::ostream> CreateDataFile(const std::string& filename) = 0;
		//! Load a custom data file
		virtual std::unique_ptr<std::istream> LoadDataFile(const std::string& filename) = 0;
	};

}

#endif
