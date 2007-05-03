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

#include "FusionLoadingState.h"

#include "FusionLoadingStage.h"

namespace FusionEngine
{

	LoadingState::LoadingState()
	{
	}

	LoadingState::~LoadingState()
	{
		Clear();
	}

	void LoadingState::AddLoadingStage(SharedStage stage)
	{
		// Add the stage to the queue
		m_Stages.push_back(stage);
	}

	void LoadingState::RemoveLoadingStage(SharedStage stage)
	{
		// Clean up if this stage is running
		if (stage == m_ActiveStage)
		{
			m_ActiveStage->CleanUp();
			m_ActiveStage.release();
		}

		StageQueue::iterator it;
		for (it = m_Stages.begin(); it != m_Stages.end(); ++it)
		{
			// Compare pointers (equiv. operator is overloaded in CL_SharedPtr)
			if ((*it) == stage)
			{
				m_Stages.erase(it);
			}
		}
	}

	void LoadingState::RecalculateProgressScale()
	{
		if (m_Stages.empty())
			throw Error(Error::INTERNAL_ERROR, "Trying to calculate loading stage percentage with no loading stages");
		
		m_StageProgressScale = 1.f / m_Stages.size();
	}

	void LoadingState::Clear()
	{
		// Clean up the currently running stage
		if (m_ActiveStage)
		{
			m_ActiveStage->CleanUp();
			m_ActiveStage.release();
		}

		m_Stages.clear();
	}

	bool LoadingState::Update(unsigned int split)
	{
		// If loader state should have been removed, but for some reason 
		//  Update() is being called again...
		if (m_Finished)
			return true; // ... breakout!


		// Run new stages when necessary
		if (m_ActiveStage.is_null())
		{
			if (m_Stages.empty())
			{
				// Nothing in queue - nothing to do
				_pushMessage(new StateMessage(StateMessage::REMOVESTATE, this));
				m_Finished = true;

				return true;
			}
			else
			{
				// Run next stage
				runNextStage();
			}
		}

		try
		{
			// Update the current stage
			m_Progress = m_ActiveStage->Update(split)*m_StageProgressScale;
		}
		catch (LoadingException e)
		{
			Console::getSingletonPtr()->Add(e.GetError());

			std::string message = CL_String::format(
				"Loading stage '%1' failed to complete:\n\n%2",
				m_ActiveStage->GetName(), e.GetError()
				);
			throw Error(Error::LOADING, message, false);

		}

		if (m_ActiveStage->IsDone())
		{
			// Tell the stage to clean up
			m_ActiveStage->CleanUp();

			runNextStage();
		}

		return true;
	}

	void LoadingState::runNextStage()
	{
		m_ActiveStage = m_Stages.front();
		m_Stages.pop_front();
		if (!m_ActiveStage->Initialise())
		{
			// The stage failed to init.

			// Drop the ptr
			m_ActiveStage.release();

			// Throw an exception
			std::string message = 
				CL_String::format("Loading stage '%1' failed to initialize.", m_ActiveStage->GetName());
			throw Error(Error::INTERNAL_ERROR, message);
		}
	}

}
