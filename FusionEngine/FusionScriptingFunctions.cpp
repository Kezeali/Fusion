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

/// Fusion
#include "FusionEnvironmentGeneric.h"
#include "FusionShip.h"
#include "FusionProjectile.h"
#include "FusionConsole.h"

namespace FusionEngine
{

	void SCR_DetonateProjectile(ObjectID index)
	{
		GenericEnvironment::getSingletonPtr()->Detonate(index);
	}

	void SCR_DetonateProjectileG(asIScriptGeneric* gen)
	{
		ObjectID index = gen->GetArgDWord(0);
		GenericEnvironment::getSingletonPtr()->Detonate(index);
	}

	void SCR_ApplyEngineForce(ObjectID index)
	{
		GenericEnvironment::getSingletonPtr()->ApplyEngineForce(index);
	}

	void CON_ListProjectiles()
	{
		GenericEnvironment::ProjectileList list =
			GenericEnvironment::getSingletonPtr()->GetProjectileList();

		GenericEnvironment::ProjectileList::iterator it = 
			list.begin();

		for (; it != list.end(); ++it)
		{
			Console::getSingletonPtr()->Add( it->second->ToString() );
		}
	}

	void CON_ListProjectilesG(asIScriptGeneric* gen)
	{
		Console::getSingletonPtr()->Add();
	}

}