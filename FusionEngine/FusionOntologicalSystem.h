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

#include "FusionPrerequisites.h"

#include "FusionState.h"
#include "FusionPacketHandler.h"

#include "FusionPlayerRegistry.h"
#include "FusionRenderer.h"
#include "FusionScriptModule.h"

namespace FusionEngine
{

	//! System that manages Entities
	class OntologicalSystem : public System, public PacketHandler
	{
	public:
		//! An array of viewports
		typedef std::vector<ViewportPtr> ViewportArray;

	public:
		//! Constructor
		OntologicalSystem(
			ClientOptions *options,
			Renderer *renderer,
			InputManager *input_manager,
			NetworkSystem *network_system);
		//! Destructor
		virtual ~OntologicalSystem();

		//! Returns the name of the system ("Entities"
		virtual const std::string &GetName() const;

		//! Initialises the system (creates EntityManager, MapLoader, etc.)
		virtual bool Initialise();
		//! Cleans up the system when it is unused (deletes EntityManager object, etc.)
		virtual void CleanUp();

		//! Updates the entity managers
		virtual void Update(float split);
		//! Runs the renderer, mainly
		virtual void Draw();

		void HandlePacket(IPacket *packet);

		//! Adds a viewport
		void AddViewport(ViewportPtr viewport);
		//! Removes the given viewport
		void RemoveViewport(ViewportPtr viewport);

		//! Removes all viewports
		void RemoveAllViewports();

		//! Returns the list of viewports
		ViewportArray &GetViewports();

		EntityManager * const GetEntityManager();

		// Might be useful, but I don't think this is how it should work
		//  Rather, Bodies should have a OwnerID much like entities do
		//  This is important for the authority manager
		//void MapBodyToEntity(PhysicsBodyPtr &body, EntityPtr &entity);
		//EntityPtr GetBodyEntity(PhysicsBodyPtr body) const;

		//! Sets the script module to use
		void SetModule(const ModulePtr &module);
		//! Called when the module is built (slot function)
		void OnModuleRebuild(BuildModuleEvent& ev);

		//! Quits the game
		/*!
		* Intended to be called from script
		*/
		void Quit();
		//! Saves the game
		/*!
		* Intended to be called from script
		*/
		void Save(const std::string &filename);
		//! Saves the game
		/*!
		* Intended to be called from script
		*/
		void Load(const std::string &filename);
		//! Pauses the game
		void Pause();
		//! Resumes the game
		void Resume();

		//! Adds a player (no callback)
		/*!
		* Intended to be called from script.
		* <br>
		* Note that this is for adding local players only, net
		* players are added to the PlayerRegistry automatically
		* when they join the peer-network.
		*
		* \returns
		* The index of the player added
		*/
		unsigned int AddPlayer();
		//! Adds a player
		/*!
		* Intended to be called from script.
		* <br>
		* Note that this is for adding local players only, net
		* players are added to the PlayerRegistry automatically
		* when they join the peer-network.
		*
		* \returns
		* The index of the player added
		*
		* \param[in] callback_obj
		* The script object to call the callback method on.
		*
		* \param[in] callback_decl
		* The declaration of the script function to call when
		* a Net-Index has been supplied (by the arbitor) for
		* the given player.
		*/
		unsigned int AddPlayer(asIScriptObject *callback_obj, const std::string &callback_decl);
		//! Adds a player
		/*!
		* Intended to be called from script.
		* Note that Entities owned by the given player will NOT be
		* automatically removed (since this isn't necessarily
		* desireable) - the Entity that calls this method should
		* remove owned entities as desired.
		*
		* \param[in] index
		* The local-index of the player to remove.
		*/
		void RemovePlayer(unsigned int index);

		//! Sets a callback which will be called when any player is added
		void SetAddPlayerCallback(asIScriptObject *callback_obj, const std::string &callback_decl);
		//! Sets a callback which will be called when the given player is added
		void SetAddPlayerCallback(unsigned int player, asIScriptObject *callback_obj, const std::string &callback_decl);


		static const size_t s_MaximumSplitScreenViewports = 4;
		enum SplitType
		{
			HorizontalFirst,
			VerticalFirst,
			AlwaysQuaters
		};

		//! Sets the area to be divided up to create split screen viewports
		void SetSplitScreenArea(const CL_Rectf &area);

		void SetSplitScreenType(SplitType type);

		//! Creates a new split-screen viewport and shrinks existing split-screen viewports to fit
		/*!
		* \param[in] player
		* This is simply an ID to make removing / retrieving this viewport easier
		*/
		ViewportPtr AddSplitScreenViewport(unsigned int player);
		//! Removes the split-screen viewport for the given ID
		void RemoveSplitScreenViewport(unsigned int player);

		typedef std::tr1::array<int, s_MaximumSplitScreenViewports> PlayerOrderArray;
		//! Sets the order of splitscreen viewports - this will change the order that they are layed out on screen
		void SetSplitScreenOrder(const PlayerOrderArray &player_order);
		//! Sets the order of splitscreen viewports
		/*!
		* Script version of SetSplitScreenOrder()
		*/
		void SetSplitScreenOrder(asIScriptArray *player_order);

		//! Returns Renderer::GetContextWidth()
		int GetScreenWidth() const;
		//! Returns Renderer::GetContextWidth()
		int GetScreenHeight() const;

		//! Enables the physics debug visualization on the given viewport
		void EnablePhysicsDebugDraw(ViewportPtr viewport);
		//! Counterpart to OntologicalSystem::EnablePhysicsDebugDraw()
		void DisablePhysicsDebugDraw();

		void onGetNetIndex(unsigned int local_idx, ObjectID net_idx);

		//! Registers script types relating to the System
		static void Register(asIScriptEngine *engine);

	protected:
		EntitySynchroniser *m_EntitySyncroniser;
		StreamingManager *m_Streaming;
		EntityFactory *m_EntityFactory;
		EntityManager *m_EntityManager;
		GameMapLoader *m_MapLoader;

		//PhysicsWorld *m_PhysicsWorld;
		PhysicalWorld *m_PhysWorld;

		ViewportArray m_Viewports;
		typedef std::pair<ViewportPtr, unsigned int> SplitScreenViewport;
		typedef std::vector<SplitScreenViewport> SplitScreenViewportArray;
		SplitScreenViewportArray m_SplitScreenViewports;
		CL_Rectf m_SplitScreenArea;
		SplitType m_SplitType;

		Renderer *m_Renderer;
		InputManager *m_InputManager;
		NetworkSystem *m_NetworkSystem;

		typedef std::tr1::shared_ptr<Editor> EditorPtr;
		EditorPtr m_Editor;

		//! Basic script callback data
		struct CallbackDecl
		{
			asIScriptObject *object;
			std::string method;
			//! Default constructor
			CallbackDecl()
				: object(NULL)
			{}
			//! Constructor
			CallbackDecl(asIScriptObject *obj, std::string method)
				: object(obj), method(method)
			{}
		} m_AddPlayerCallbacks[s_MaxLocalPlayers];
		CallbackDecl m_AddAnyPlayerCallback;

		ObjectID m_NextPlayerIndex;
		std::deque<ObjectID> m_FreePlayerIndicies;

		ClientOptions *m_Options;

		ModulePtr m_Module;
		bsig2::connection m_ModuleConnection;

		std::string m_StartupEntity;
		std::string m_StartupMap;

		bool createScriptCallback(CallbackDecl &out, asIScriptObject *callback_obj, const std::string &callback_decl);

		ObjectID getNextPlayerIndex();
		void releasePlayerIndex(ObjectID net_index);

		void fitSplitScreenViewports();
	};

}

#endif
