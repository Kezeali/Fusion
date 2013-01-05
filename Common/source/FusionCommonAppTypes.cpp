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

#include "PrecompiledHeaders.h"

#include "FusionCommonAppTypes.h"

#include <angelscript.h>

namespace FusionEngine { namespace Scripting
{

#define FSN_REGISTER_APPTYPE(type, id) DefineAppType<type>(id, engine->GetObjectTypeById(id)->GetName());


	void RegisterCommonAppTypes(asIScriptEngine* engine)
	{
		DefineAppType<bool>(asTYPEID_BOOL, "bool");
		DefineAppType<std::int8_t>(asTYPEID_INT8, "int8");
		DefineAppType<std::int16_t>(asTYPEID_INT16, "int16");
		DefineAppType<std::int32_t>(asTYPEID_INT32, "int32");
		DefineAppType<std::int64_t>(asTYPEID_INT64, "int64");
		DefineAppType<std::uint8_t>(asTYPEID_UINT8, "uint8");
		DefineAppType<std::uint16_t>(asTYPEID_UINT16, "uint16");
		DefineAppType<std::uint32_t>(asTYPEID_UINT32, "uint32");
		DefineAppType<std::uint64_t>(asTYPEID_UINT64, "uint64");
		DefineAppType<float>(asTYPEID_FLOAT, "float");
		DefineAppType<double>(asTYPEID_DOUBLE, "double");
	}

} }