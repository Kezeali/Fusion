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

#ifndef Header_OutputSource_PhysFS
#define Header_OutputSource_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"


	class OutputSource_PhysFS : public CL_OutputSource
	{
	public:
		//! Input Souce PhysFS constructor.
		OutputSource_PhysFS(const std::string &filename);

		//! Cloning constructor
		OutputSource_PhysFS(const OutputSource_PhysFS *source);

		//! Destructor
		virtual ~OutputSource_PhysFS();

	public:
		//! Returns current position in input source.
		virtual int tell() const;

		//! Returns the size of the input source
		virtual int size() const;

	public:
		//! Writes a signed 64 bit integer to output source.
		virtual void write_int64(cl_int64 data);

		//! Writes an unsigned 64 bit integer to output source.
		virtual void write_uint64(cl_uint64 data);

		//! Writes a signed 32 bit integer to output source.
		virtual void write_int32(cl_int32 data);

		//! Writes an unsigned 32 bit integer to output source.
		virtual void write_uint32(cl_uint32 data);

		//! Writes a signed 16 bit integer to output source.
		virtual void write_int16(cl_int16 data);

		//! Writes an unsigned 16 bit integer to output source.
		virtual void write_uint16(cl_uint16 data);

		//! Reads larger amounts of data
		/*!
		 * \param[in] data
		 * Points to the array from which data should be written.
		 *
		 * \param[in] size
		 * Number of bytes to write.
		 *
		 * \returns
		 * Number of bytes actually written.
		 */
		virtual int write(const void *data, int size);

		//! Opens the output source. By default, it is open.
		virtual void open();

		//! Closes the output source.
		virtual void close();

		//! Makes a copy of the current outputsource; starts at the same write position.
		/*!
		 * \returns
		 * The copy of the output source.
		 */
		virtual CL_OutputSource *clone();

	private:
		unsigned int m_Position;
		PHYSFS_file *m_PhysFile;
		std::string m_Filename;

	};

#endif