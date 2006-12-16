/*
  Copyright (c) 2006 FusionTeam

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

#ifndef Header_InputSourceProvider_PhysFS
#define Header_InputSourceProvider_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"

//namespace FusionEngine
//{

	class InputSourceProvider_PhysFS : public CL_InputSourceProvider
	{
	public:
		//! Construtcs a file input source provider.
		/*!
		 * If the (path) given is not already in the PhysFS search path, it will be
		 * added, and removed when this object is destroyed. <br>
		 * If the (path) given <i>is</i> in the PhysFS search path, it will not be
		 * added, and will of course remain in the search path after this object is
		 * destroyed.
		 *
		 *
		 * \param[in] path
		 * Absolute Path, or absolute path to archive, to add to the search path.
		 * Use "." or an empty string if you don't wan't to add a path.
		 */
		InputSourceProvider_PhysFS(const std::string &path = "");

		//! Destructor
		~InputSourceProvider_PhysFS();

	public:
		//: Open input source using file as its source.
		//param filename: Filename to open.
		//return: InputSource for the file.
		virtual CL_InputSource *open_source(const std::string &filename);

		//: Get full path to source:
		virtual std::string get_pathname(const std::string &filename);

		//: Returns a new inputsource provider object that uses a new path relative to current one.
		virtual CL_InputSourceProvider *create_relative(const std::string &path);

		//: Clone the provider.
		//- <p>Creates a new instance of the class with the same internal data.
		//- This is equivaled to <i>new CL_InputSourceProvider_File(get_path())</i>.</p>
		//return: New instance of the current provider.
		virtual CL_InputSourceProvider *clone();

	private:
		std::string m_Provider_path;
		//! Will be true if the path had to be added to the search path (and thus will be removed upon destruction.)
		bool m_PathAdded;
	};

//}

#endif
