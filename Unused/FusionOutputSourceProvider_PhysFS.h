/*
  Copyright (c) 2006 Fusion Project Team

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

#ifndef Header_OutputSourceProvider_PhysFS
#define Header_OutputSourceProvider_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"


	class OutputSourceProvider_PhysFS : public CL_OutputSourceProvider
	{
	public:
		//! Construtcs a physfs output source provider.
		/*!
		 * \remarks
		 * PhysFS only has one write path, so (path) isn't used.
		 *
		 * \param[in] path
		 * A requirement of CL_OutputSourceProvider. Not used.
		 */
		OutputSourceProvider_PhysFS(const std::string &path);

		//! Destructor
		virtual ~OutputSourceProvider_PhysFS();

	public:

		//! Opens an outputsource using the passed handle.
		/*!
		 * \param handle
		 * Filename to the output source requested.
		 *
		 * \returns
		 * The output source opened.
		 */
		virtual CL_OutputSource *open_source(const std::string &handle);

		//! Returns a copy of the current provider.
		/*!
		 * \returns
		 * The copy of the outputsource provider.
		 */
		virtual CL_OutputSourceProvider *clone();

	private:
		std::string m_Provider_path;
	};

#endif
