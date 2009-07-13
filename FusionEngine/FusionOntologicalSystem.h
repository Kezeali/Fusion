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
#ifndef Header_FusionEngine_OntologicalSystem
#define Header_FusionEngine_OntologicalSystem

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

#include "FusionScriptModule.h"


namespace FusionEngine
{

	class GameAreaLoader;

	class OntologicalSystem : public System
	{
	public:
		OntologicalSystem(InputManager *input_manager);
		virtual ~OntologicalSystem();

		virtual const std::string &GetName() const;

		virtual bool Initialise();
		virtual void CleanUp();

		virtual bool Update(unsigned int split);
		virtual void Draw();

		void SetModule(ModulePtr module);

		void OnModuleRebuild(BuildModuleEvent& ev);

	protected:
		EntityManager *m_EntityManager;
		GameAreaLoader *m_GameLoader;

		PhysicsWorld *m_PhysicsWorld;

		bsig2::connection m_ModuleConnection;
	};

}

#endif
