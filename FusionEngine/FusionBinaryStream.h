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

#ifndef H_FusionBinaryStream
#define H_FusionBinaryStream

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"


namespace FusionEngine
{
	
	namespace IO
	{
		namespace Streams
		{
			// TODO: endian-ness conversion
			class BinaryStreamReader
			{
			public:
				BinaryStreamReader(std::istream* stream);

				template <typename T>
				bool Read(T& out)
				{
					if (m_Stream->read(reinterpret_cast<char*>(&out), sizeof(out)))
					{
						if (m_LittleEndian == CL_Endian::is_system_big())
						{
							CL_Endian::swap(&out, sizeof(out));
						}
						return value;
					}
					else
					{
						m_Stream->clear();
						return false;
					}
				}

				template <typename T>
				T ReadValue()
				{
					T value;
					if (m_Stream->read(reinterpret_cast<char*>(&value), sizeof(value)))
					{
						// If neccessary, convert from source endianness to native endianness
						if (m_LittleEndian == CL_Endian::is_system_big())
						{
							CL_Endian::swap(&value, sizeof(value));
						}
						return value;
					}
					else
					{
						m_Stream->clear();
						return T(0);
					}
				}

			private:
				std::istream* m_Stream;
				bool m_LittleEndian;
			};

			class BinaryStreamWriter
			{
			public:
				BinaryStreamWriter(std::ostream* stream);

				template <typename T>
				bool Write(T& out)
				{
					if (!m_Stream->write(reinterpret_cast<char*>(&out), sizeof(out)))
					{
						m_Stream->clear();
						return false;
					}
					else
						return true;
				}

			private:
				std::ostream* m_Stream;
			};
		}
	}

}

#endif
