#ifndef Header_FusionEngine_FusionState
#define Header_FusionEngine_FusionState

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * The base class for FusionState-s.
	 *
	 * Each state controls a specific task while the game runs. For example there is a
	 * LOADING state, which controls connecting to the server, downloading files, and
	 * finally loading the the requried data.
	 *
	 * \remarks
	 * Each state should require no knowlage of its StateManager, so they are never
	 * provided with access to it. This is because allowing states to modify the state
	 * manager could cause serious problems when running multiple states concurrently.
	 */
	class FusionState
	{
	public:
		FusionState();
		virtual ~FusionState();

		virtual void Initialise();
		virtual void Update();
		virtual void Draw();
		virtual void CleanUp();
	};

}

#endif