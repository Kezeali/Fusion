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

#ifndef Header_InputSource_PhysFS
#define Header_InputSource_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"


	class InputSource_PhysFS : public CL_InputSource
	{
	public:
		//! Input Souce PhysFS constructor.
		InputSource_PhysFS(const std::string &filename);

		//! Cloning constructor
		InputSource_PhysFS(const InputSource_PhysFS *source);

		//! Destructor
		virtual ~InputSource_PhysFS();

	public:
		//! Returns current position in input source.
		virtual int tell() const;

		//! Returns the size of the input source
		virtual int size() const;

	public:
		//! Reads larger amounts of data
		/*!
		 * \param[out] data
		 * Pass an array here, where the read data is to be stored.
		 *
		 * \param[in] size
		 * Number of bytes to read.
		 *
		 * \returns
		 * Number of bytes actually read.
		 */
		virtual int read(void *data, int size);

		//! Opens the input source. By default, it is open.
		virtual void open();

		//! Closes the input source.
		/*!
		 * This is called automatically at destruction.
		 */
		virtual void close();

		//! Make a copy of the current inputsource, at the same read position.
		/*!
		 * \returns
		 * The copy of the input source.
		 */
		virtual CL_InputSource *clone() const;

		//! Seeks to the specified position in the input source.
		/*!
		 * \param[in] pos
		 * Position relative to 'seek_type'.
		 *
		 * \param[in] seek_type
		 * Defines what (pos) is relative to. Can be either seek_set, seek_cur or seek_end.
		 */
		virtual void seek(int pos, SeekEnum seek_type);

		//! Pushes the current input source position into a stack.
		/*!
		 * The position can be restored again later with pop_position() .
		 */
		virtual void push_position();

		//! Pops a previous pushed input source position
		/*!
		 * <p>And, of course, seeks to said position.</p>
		 * A position can be stored with push_position() .
		 */
		virtual void pop_position();


	private:
		std::stack<PHYSFS_sint64> m_Stack;
		std::string m_Filename;
		PHYSFS_file *m_PhysFile;
		PHYSFS_uint64 m_Filesize;
	};

#endif
