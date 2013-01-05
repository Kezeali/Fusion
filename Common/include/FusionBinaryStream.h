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

#include <ClanLib/core.h>

namespace FusionEngine
{

	namespace IO
	{
		namespace Streams
		{
			// TODO: endian-ness conversion
			template <typename CharT>
			class BinaryStreamReader
			{
			public:
				typedef CharT char_type;

				BinaryStreamReader(std::basic_istream<char_type>* stream)
					: m_Stream(stream),
					m_LittleEndian(!clan::Endian::is_system_big())
				{
				}

				template <typename T>
				bool Read(T& out)
				{
					if (m_Stream->read(reinterpret_cast<char_type*>(&out), sizeof(out)))
					{
						if (m_LittleEndian == clan::Endian::is_system_big())
						{
							clan::Endian::swap(&out, sizeof(out));
						}
						return true;
					}
					else
					{
						m_Stream->clear();
						return false;
					}
				}

				std::string ReadString()
				{
					std::string value;
					std::string::size_type length;
					Read(length);
					if (length > 0)
					{
						value.resize(length);
						if (m_Stream->read(&value[0], length))
							return std::move(value);
						else
						{
							value.clear();
						}
					}
					return std::move(value);
				}

				template <typename T>
				T ReadValue()
				{
					static_assert(std::is_fundamental<T>::value, "Must actually be a basic type");
					T value;
					if (m_Stream->read(reinterpret_cast<char_type*>(&value), sizeof(value)))
					{
						// If neccessary, convert from source endianness to native endianness
						if (m_LittleEndian == clan::Endian::is_system_big())
						{
							clan::Endian::swap(&value, sizeof(value));
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
				std::basic_istream<char_type>* m_Stream;
				bool m_LittleEndian;
			};

			template <typename CharT>
			class BinaryStreamWriter
			{
			public:
				typedef CharT char_type;

				BinaryStreamWriter(std::basic_ostream<char_type>* stream)
					: m_Stream(stream),
					m_LittleEndian(!clan::Endian::is_system_big())
				{
				}
				
				template <typename T>
				bool WriteImpl(const T& value)
				{
					if (m_Stream->write(reinterpret_cast<const char_type*>(&value), sizeof(value)))
					{
						return true;
					}
					else
					{
						m_Stream->clear();
						return false;
					}		
				}

				template <typename T>
				bool Write(const T& val)
				{
					// If neccessary, convert from source endianness to native endianness
					if (m_LittleEndian == clan::Endian::is_system_big())
					{
						T swappedVal = val;
						clan::Endian::swap(&swappedVal, sizeof(swappedVal));
						return WriteImpl(swappedVal);
					}
					else
						return WriteImpl(val);
				}

				bool WriteString(const std::string& str)
				{
					// Write string length then value
					if (Write(str.length()) && m_Stream->write(str.data(), str.length()))
						return true;
					else
						return false;
				}

				template <typename T, typename VT>
				bool WriteAs(VT out)
				{
					T casted = T(out);
					// If neccessary, convert from source endianness to native endianness
					if (m_LittleEndian == clan::Endian::is_system_big())
					{
						clan::Endian::swap(&casted, sizeof(casted));
					}
					if (m_Stream->write(reinterpret_cast<char_type*>(&casted), sizeof(casted)))
					{
						return true;
					}
					else
					{
						m_Stream->clear();
						return false;
					}
				}

			private:
				std::basic_ostream<char_type>* m_Stream;
				bool m_LittleEndian;
			};

			typedef BinaryStreamReader<char> CellStreamReader;
			typedef BinaryStreamWriter<char> CellStreamWriter;

		}
	}

}

#endif
