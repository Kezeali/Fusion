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

#include "FusionScriptingFunctions.h"

#include "FusionConsole.h"

namespace FusionEngine
{

	void RegisterWeaponMethods()
	{
		if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		{
			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

			r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
			r = m_asEngine->RegisterGlobalFunction("ApplyForce", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
		}
		else
		{
			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );
		}
	}

	void RegisterConsoleMethods()
	{
		if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		{
			r = m_asEngine->RegisterGlobalFunction("GetProjectileList", asFUNCTION(SCR_GetProjectileList), asCALL_CDECL); cl_assert( r >= 0 );
			
			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile", asFUNCTION(SCR_DetonateProjectile), asCALL_CDECL); cl_assert( r >= 0 );

			r = m_asEngine->RegisterGlobalFunction("ApplyEngineForce", asFUNCTION(SCR_ApplyEngineForce), asCALL_CDECL); cl_assert( r >= 0 );
			r = m_asEngine->RegisterGlobalFunction("ApplyForce", asFUNCTION(SCR_ApplyForce), asCALL_CDECL); cl_assert( r >= 0 );
		}
		else
		{
			r = m_asEngine->RegisterGlobalFunction("GetProjectileList", asFUNCTION(SCR_GetProjectileListG), asCALL_CDECL); cl_assert( r >= 0 );


			r = m_asEngine->RegisterGlobalFunction("DetonateProjectile", asFUNCTION(SCR_DetonateProjectileG), asCALL_GENERIC); cl_assert( r >= 0 );
		}
	}

	void CON_ListProjectiles()
	{
		Console::getSingletonPtr()->Add();
	}

	void CON_ListProjectilesG(asIScriptGeneric* gen)
	{
		Console::getSingletonPtr()->Add();
	}

}