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

#ifndef Header_InputSource_PhysFS
#define Header_InputSource_PhysFS

#if _MSC_VER > 1000
#pragma once
#endif

#include <ClanLib/core.h>

#include "PhysFS.h"

//namespace FusionEngine
//{

	class InputSource_PhysFS : public CL_InputSource
	{
	public:
		//: Input Souce File constructor.
		InputSource_PhysFS(const std::string &filename);

		InputSource_PhysFS(const InputSource_PhysFS *source);

		//: Input Souce File destructor
		virtual ~InputSource_PhysFS();

		//! Attributes:
	public:
		//: Returns current position in input source.
		//return: Current position in input source.
		virtual int tell() const;

		//: Returns the size of the input source
		//return: Size of the input source.
		virtual int size() const;

		//! Operations:
	public:
		//: Reads larger amounts of data (no endian and 64 bit conversion):
		//param data: Points to an array where the read data is stored.
		//param size: Number of bytes to read.
		//return: Num bytes actually read.
		virtual int read(void *data, int size);

		//: Opens the input source. By default, it is open.
		virtual void open();

		//: Closes the input source.
		virtual void close();

		//: Make a copy of the current inputsource, standing at the same position.
		//return: The copy of the input source.
		virtual CL_InputSource *clone() const;

		//: Seeks to the specified position in the input source.
		//param pos: Position relative to 'seek_type'.
		//param seek_type: Defines what the 'pos' is relative to. Can be either seek_set, seek_cur og seek_end.
		virtual void seek(int pos, SeekEnum seek_type);

		//: Pushes the current input source position.
		//- <p>The position can be restored again with pop_position.</p>
		virtual void push_position();

		//: Pops a previous pushed input source position (returns to the position).
		virtual void pop_position();

		//: Gets the actual path after relative path translation.
		//static std::string translate_path(const std::string &path);

		//! Implementation:
	private:
		std::stack<PHYSFS_sint64> m_Stack;
		std::string m_Filename;
		PHYSFS_File *m_PhysFile;
		PHYSFS_uint64 m_Filesize;
	};

//}

#endif
