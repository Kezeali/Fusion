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

#ifndef Header_FusionEngine_Editor
#define Header_FusionEngine_Editor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionState.h"

#include "FusionViewport.h"
#include "FusionInputHandler.h"


namespace FusionEngine
{

	//! Editor system (runs the map editor interface)
	class Editor : public System
	{
	public:
		Editor(InputManager *input, Renderer *renderer, StreamingManager *streaming_manager, EntityManager *ent_manager);
		virtual ~Editor();

	public:
		const std::string &GetName() const;

		bool Initialise();

		void CleanUp();

		void Update(float split);

		void Draw();

		void Enable(bool enable = true);

		void OnRawInput(const RawInput &ev);

		//void ProcessEvent(Rocket::Core::Event& ev);

		//void OnDebugEvent(DebugEvent& ev);

		void StartEditor();
		void StopEditor();

		static void Register(asIScriptEngine *engine);

	protected:
		Renderer *m_Renderer;
		StreamingManager *m_Streamer;
		InputManager *m_Input;
		EntityManager *m_EntityManager;

		boost::signals2::connection m_RawInputConnection;

		ViewportPtr m_Viewport;
		CameraPtr m_Camera;

		Vector2 m_CamVelocity;

		bool m_Enabled;

	};

}

#endif
