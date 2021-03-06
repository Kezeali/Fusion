/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionCellStreamTypes
#define H_FusionCellStreamTypes

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

namespace RakNet
{
	class BitStream;
}

namespace FusionEngine
{

	typedef std::basic_istream<char> ICellStream;
	typedef std::basic_ostream<char> OCellStream;
	typedef std::basic_iostream<char> IOCellStream;

}

#endif
