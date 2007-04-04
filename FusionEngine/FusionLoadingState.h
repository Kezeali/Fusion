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

#include "FusionLoadingTransferCallback.h"

#include <RakNet/RakClientInterface.h>
#include <RakNet/FileListTransfer.h>
#include <RakNet/FileListTransferCBInterface.h>

#include <CEGUI/CEGUI.h>


namespace FusionEngine
{

	//! Basic LoadingStage manager
	/*!
	 * Can be used to make an extendable loader. May be implemented later.
	 * For now, loading states just use an enum and switch-cases to manage loading stages
	 */
	class LoadingState : public FusionState
	{
	public:
		//! A queue of loading stages to execute
		typedef std::deque<LoadingStage> StageQueue;
		//! Shared ptr for stages
		typedef CL_SharedPtr<LoadingStage> SharedStage;

	public:
		//! Constructor
		LoadingState();

		//! Destructor
		~LoadingState();

	public:
		//! Adds a loaing stage to this loader
		void AddLoaingStage(SharedStage stage);

		//! Initialise
		bool Initialise();
		//! Update
		bool Update(unsigned int split);
		//! Draw
		void Draw();
		//! CleanUp
		void CleanUp();


	protected:
		//! Loading stages currently queued
		StageQueue m_LoadingStages;

	};

}

#endif