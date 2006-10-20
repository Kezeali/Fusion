/*
  Copyright (c) 2006 Elliot Hayward

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
*/

#ifndef Header_FusionEngine_Archive
#define Header_FusionEngine_Archive

namespace FusionEngine
{
	class Archive
	{
	public:
		//! Basic constructor
		Archive();
		//! Constructor +choose file
		Archive(std::string filename);

		//! Destructor
		~Archive();
	public:
		//! Associates an archive with this object.
		void Open(std::string filename);
		//! Decompresses the archive associated with this object (to TempPath)
		void Decompress();
		//! Returns a list of paths to the files which have been decompressed by this object
		StringVector GetFileList();
		//! Deletes all the files which have been decompressed by this object
		void DeleteTemps();

	};
}

#endif
