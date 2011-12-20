/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_LoadingState
#define Header_FusionEngine_LoadingState

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionState.h"

#include "FusionLoadingStage.h"
#include "FusionLoadingException.h"


namespace FusionEngine
{

	//! Shared ptr for stages
	typedef CL_SharedPtr<LoadingStage> SharedStage;

	//! Basic LoadingStage manager
	/*!
	 * Can be used to make an extendable loader. Implementations must define 
	 * FusionState#Initialise(), FusionState#Draw() and FusionState#CleanUp().
	 * FusionState#Update() is defined, and runs the loading stage manager.
	 *
	 * For example, a script-based loader could be implemented which simply runs
	 * a script - the script would initialize and handle LoadingStages.
	 *
	 * \remarks
	 * Just something to think about:
	 * <code>
	 *  if (myPtr)
	 *	{
	 *		//Execute normally
	 *	}
	 *	// if the active stage ptr. is null, something bad has happened (probably...)
	 *	else
	 *	{
	 *		// I like this sort of wording, it gives me that warm fuzzy feeling
	 *		std::string message = CL_String::format(
	 *			"Loading stage object destroyed prematurely.\n\n%1", e.GetError()
	 *			);
	 *		// ... yep, that's some ter-meen-lologgy for ya
	 *		throw Error(Error::INTERNAL_ERROR, message, true);
	 *	}
	 * </code>
	 * hmmm...
	 */
	class LoadingState : public FusionState
	{
	public:
		//! A queue of loading stages to execute
		typedef std::deque<SharedStage> StageQueue;

	public:
		//! Constructor
		LoadingState();

		//! Destructor
		~LoadingState();

	public:
		//! Adds a loading stage to this loader
		void AddLoadingStage(SharedStage stage);

		//! Removes a loading stage from the queue
		void RemoveLoadingStage(SharedStage stage);

		//! Recalculates the progress per stage scale
		/*!
		 * This should be called after all loading stages have been added, and before
		 * Update() is called.
		 *
		 * \todo Remove this, and put the functionality in AddLoadingStage() (that
		 * will be more convinient for scripting)
		 */
		//void RecalculateProgressScale();

		//! Removes all states (including queued)
		void Clear();

		//! Implementation of FusionState#Update()
		bool Update(unsigned int split);


	protected:
		//! Loading stages currently queued
		StageQueue m_Stages;

		//! The stage being executed
		SharedStage m_ActiveStage;

		//! Overall progress of the load
		float m_Progress;
		//! Progress scaler
		/*!
		 * The ratio to which the progress of each stage is scaled before it
		 * is applied to the overall progress.
		 */
		float m_StageProgressScale;

		//! True if the loader has finished (e.g. when all stages are complete)
		bool m_Finished;

	protected:
		//! Initilises the next stage and sets it as active
		/*! 
		 * Does not do anything with the previously active stage, so that stage's
		 * LoadingStage#CleanUp() should be called before this is called.
		 */
		void runNextStage();

	};

}

#endif