/*
*  Copyright (c) 2010-2011 Fusion Project Team
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

#ifndef H_FusionFirstCause
#define H_FusionFirstCause

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionScriptModule.h"

#include <boost/signals2/connection.hpp>

class VirtualFileSource_PhysFS;

namespace FusionEngine
{

	//! Creates OntologicalSystem, Editor
	/*!
	* I must note that this class exists, to a slight extent, for my own nomenclatic (or onomastic*) amusement.
	* <p>* shout out to any pedants in the audiance.</p>
	*/
	class FirstCause
	{
	public:
		//! CTOR
		FirstCause(ClientOptions *options, Renderer *renderer, InputManager *input_manager);
		//! DTOR
		/*!
		* Does this method's presence confirm Deism?!
		*/
		virtual ~FirstCause();

		//! Lets there be light (Sorry about this. At least /I/ find it entertaining)
		/*!
		* Creates OntologicalSystem and Editor and adds them to the system manager
		*/
		void Initialise(ModulePtr module);

		//! Called when the initialised module is built
		void OnBuildModule(BuildModuleEvent& ev);

		//! Starts the Ontology or Editor (depending on what was indicated by the options)
		void BeginExistence(SystemsManager *system_manager);

		//! Activates the editor
		void SwitchToEditor();
		//! Activates play mode
		void SwitchToGame();

		std::string ToggleMode(const StringVector &args);

		std::string SwitchTo(const StringVector &args);

		std::string SetStreamingRange(const StringVector &args);

	protected:
		ClientOptions *m_Options;
		Renderer *m_Renderer;
		InputManager *m_InputManager;

		EntitySynchroniser *m_EntitySync;
		StreamingManager *m_Streaming;
		EntityFactory *m_EntityFactory;
		EntityManager *m_EntityManager;
		InstancingSynchroniser *m_InstancingSync;
		GameMapLoader *m_MapLoader;

		std::unique_ptr< VirtualFileSource_PhysFS > m_FileSource;

		PhysicalWorld *m_PhysWorld;

		typedef std::shared_ptr<Editor> EditorPtr;
		EditorPtr m_Editor;

		typedef std::shared_ptr<OntologicalSystem> OntologicalSystemPtr;
		OntologicalSystemPtr m_Ontology;

		boost::signals2::connection m_ModuleConnection;

		bool m_EditorEnabled;

		// ToggleMode checks this var
		bool m_InEditor;
	};

}

#endif
