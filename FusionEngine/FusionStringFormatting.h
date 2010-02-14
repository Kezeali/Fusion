/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionEngine_StringFormatting
#define Header_FusionEngine_StringFormatting

#if _MSC_VER > 1000
#pragma once
#endif

#include <sstream>

//! Stupid string-stream based formatting macro
/*!
 * Credit goes to mrree: 
 * http://stackoverflow.com/questions/303562/c-format-macro-inline-ostringstream
 * \remarks
 * Use FusionEngine#makestring instead.
 */
#define FSN_STREAMSTR(x) \
	( (dynamic_cast<std::ostringstream &>( \
		std::ostringstream().seekp( 0, std::ios_base::cur ) << x ) \
	).str() )


namespace FusionEngine
{

	//! Converts a stream into a string
	/*!
	 * Credit goes to eJames and litb: see posts on stackoverflow -
	 * http://stackoverflow.com/questions/303562/c-format-macro-inline-ostringstream
	 */
	template <typename Elem>
	class basic_makestring
	{
	public:
		//! Operator overloads conversion to std::string
		std::basic_stringstream<Elem> stream;
		operator std::basic_string<Elem>() const { return stream.str(); }

		//! Stream input operator
		template<class T>
		basic_makestring<Elem>& operator<<(T const& other) { stream << other; return *this; }
	};

	// Original class from stackoverflow post by eJames
	//class MakeString
	//{
	//public:
	//	std::stringstream stream;
	//	operator std::string() const { return stream.str(); }

	//	template<class T>
	//	MakeString& operator<<(T const& other) { stream << other; return *this; }
	//};

	//! Converts a stringstream into a string
	typedef basic_makestring<char> makestring;
	//! Converts a wstringstream into a wstring
	typedef basic_makestring<wchar_t> makewstring;

}

#endif