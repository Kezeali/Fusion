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
#include "FusionRenderer.h"
#include "FusionPhysicsBody.h"


namespace FusionEngine
{

	class OntologicalSystem : public System
	{
	public:
		typedef std::vector<ViewportPtr> ViewportArray;

	public:
		OntologicalSystem(Renderer *renderer, InputManager *input_manager, Network *network);
		virtual ~OntologicalSystem();

		virtual const std::string &GetName() const;

		virtual bool Initialise();
		virtual void CleanUp();

		virtual void Update(float split);
		virtual void Draw();

		void AddViewport(ViewportPtr viewport);
		void RemoveViewport(const ViewportPtr &viewport);

		void RemoveAllViewports();

		void MapBodyToEntity(PhysicsBodyPtr &body, EntityPtr &entity);
		EntityPtr GetBodyEntity(const PhysicsBodyPtr &body) const;

		ViewportArray &GetViewports();

		void SetModule(ModulePtr module);

		void OnModuleRebuild(BuildModuleEvent& ev);

	protected:
		EntitySynchroniser *m_EntitySyncroniser;
		EntityManager *m_EntityManager;
		GameMapLoader *m_MapLoader;

		PhysicsWorld *m_PhysicsWorld;

		ViewportArray m_Viewports;

		Renderer *m_Renderer;
		InputManager *m_InputManager;
		Network *m_Network;

		bsig2::connection m_ModuleConnection;

		std::string m_StartupEntity;
	};

}

#endif
