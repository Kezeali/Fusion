/*
  Copyright (c) 2006-2010 Fusion Project Team

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

#include "FusionStableHeaders.h"

#include "FusionStateManager.h"

#include "FusionExceptionFactory.h"

namespace FusionEngine
{

	SystemsManager::SystemsManager()
	{
	}

	SystemsManager::~SystemsManager()
	{
		Clear();
	}

	bool SystemsManager::SetExclusive(const SystemPtr &system)
	{
		// Remove all the current states
		Clear();

		// Add the new state if it managed to init.
		//SharedState state_spt(state);
		m_Systems.push_back(system);

		return true;
	}

	bool SystemsManager::AddSystem(const SystemPtr &system)
	{
		system->Initialise();
		m_Systems.push_back(system);
		return true;
	}

	void SystemsManager::RemoveSystem(const SystemPtr &system)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			if ((*it) == system)
			{
				m_Systems.erase(it);
				system->CleanUp();
				break;
			}
		}
	}

	void SystemsManager::Clear()
	{
		if (!m_Systems.empty())
		{
			for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
			{
				SystemPtr &system = *it;
				system->CleanUp();
			}
			m_Systems.clear();
		}
	}

	bool SystemsManager::Update(float split)
	{
		// If game should have quit, but for some reason update is being called again...
		if (!m_KeepGoing)
			return true; // ... do nothing

		typedef std::list<SystemPtr> SystemList;
		SystemList systemsToAdd;
		SystemList systemsToRemove;

		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			SystemPtr &system = *it;

			// Check state messages
			while (system->HasMessages())
			{
				// Get the next message
				const SystemMessage&& m = system->PopMessage();
				switch (m.GetType())
				{
				case SystemMessage::ADDSYSTEM:
					systemsToAdd.push_back(m.GetSystem());
					break;
				case SystemMessage::REMOVESYSTEM:
					{
						if (m.GetSystem())
							systemsToRemove.push_back(m.GetSystem());
						else
							FSN_EXCEPT(ExCode::NotImplemented, "SystemsManager::Update", "Removing systems by name is not implemented");
					break;
					}

				case SystemMessage::PAUSE:
					if (m.IncludeSender())
						system->AddFlag(System::PAUSE);
					addFlagFor(m.GetTargets(), System::PAUSE);
					break;
				case SystemMessage::RESUME:
					if (m.IncludeSender())
						system->RemoveFlag(System::PAUSE);
					removeFlagFor(m.GetTargets(), System::PAUSE);
					break;
				case SystemMessage::STEP:
					if (m.IncludeSender())
						system->AddFlag(System::STEP);
					addFlagFor(m.GetTargets(), System::STEP);
					break;
				case SystemMessage::STEPALL:
					addFlagAll(System::STEP);
					break;
				case SystemMessage::PAUSEOTHERS:
					addFlagForOthers(system, m.GetTargets(), System::PAUSE);
					break;
				case SystemMessage::RESUMEOTHERS:
					removeFlagForOthers(system, m.GetTargets(), System::PAUSE);
					break;

				case SystemMessage::HIDE:
					if (m.IncludeSender())
						system->AddFlag(System::HIDE);
					addFlagFor(m.GetTargets(), System::HIDE);
					break;

				case SystemMessage::SHOW:
					if (m.IncludeSender())
						system->RemoveFlag(System::HIDE);
					removeFlagFor(m.GetTargets(), System::HIDE);
					break;

				case SystemMessage::HIDEOTHERS:
					addFlagForOthers(system, m.GetTargets(), System::HIDE);
					break;
				case SystemMessage::SHOWOTHERS:
					removeFlagForOthers(system, m.GetTargets(), System::HIDE);
					break;


				case SystemMessage::QUIT:
					m_KeepGoing = false;
					break;
				}

				if (!m_KeepGoing)
					break; // Stop checking messages if quit message has been received
			}

			// Received a quit message
			if (!m_KeepGoing)
			{
				Clear();
				break;
			}

			// If the system isn't paused or a step message was just received:
			if (!system->CheckFlag(System::PAUSE) || system->CheckFlag(System::STEP))
			{
				//if (!system->IsInitialised())
				//{
				//	if (system->Initialise())
				//		system->AddFlag(System::INITIALISED);
				//}
				system->Update(split);
				// Clear step flag (in case it has been added, since we have now done the requested 'step')
				system->RemoveFlag(System::STEP);
			}
		}

		for (SystemList::iterator it = systemsToAdd.begin(), end = systemsToAdd.end(); it != end; ++it)
			AddSystem(*it);
		for (SystemList::iterator it = systemsToRemove.begin(), end = systemsToRemove.end(); it != end; ++it)
			RemoveSystem(*it);

		return true;
	}

	void SystemsManager::Draw()
	{
		if (!m_KeepGoing)
			return;

		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			SystemPtr &system = *it;
			if (!system->CheckFlag(System::HIDE))
				system->Draw();
		}
	}

	bool SystemsManager::KeepGoing() const
	{
		return m_KeepGoing;
	}

	void SystemsManager::setFlagsAll(int flags)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->SetFlags(flags);
		}
	}

	void SystemsManager::addFlagAll(System::StateFlags flag)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->AddFlag(flag);
		}
	}

	void SystemsManager::removeFlagAll(System::StateFlags flag)
	{
		for (SystemArray::iterator it = m_Systems.begin(), end = m_Systems.end(); it != end; ++it)
		{
			(*it)->RemoveFlag(flag);
		}
	}

	void SystemsManager::addFlagFor(const StringVector &targets, System::StateFlags flag)
	{
		for (StringVector::const_iterator target_it = targets.begin(), targets_end = targets.end(); target_it != targets_end; ++target_it)
		{
			for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
				system_it != systems_end; ++system_it)
			{
				if (*target_it == (*system_it)->GetName())
					(*system_it)->AddFlag(flag);
			}
		}
	}

	void SystemsManager::removeFlagFor(const StringVector &targets, System::StateFlags flag)
	{
		for (StringVector::const_iterator target_it = targets.begin(), targets_end = targets.end(); target_it != targets_end; ++target_it)
		{
			for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
				system_it != systems_end; ++system_it)
			{
				if (*target_it == (*system_it)->GetName())
					(*system_it)->RemoveFlag(flag);
			}
		}
	}

	void SystemsManager::addFlagForOthers(const SystemPtr &excluded_system, const StringVector &excluded_targets, System::StateFlags flag)
	{
		for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
			system_it != systems_end; ++system_it)
		{
			SystemPtr &system = *system_it;
			if (system != excluded_system)
			{
				// Check for the current systems's name in the exclusions
				bool excluded = false;
				for (StringVector::const_iterator exclude_it = excluded_targets.begin(), targets_end = excluded_targets.end();
					exclude_it != targets_end; ++exclude_it)
				{
					if (system->GetName() == *exclude_it)
						excluded = true;
				}
				if (!excluded)
					system->AddFlag(flag);
			}
		}
	}

	void SystemsManager::removeFlagForOthers(const SystemPtr &excluded_system, const StringVector &excluded_targets, System::StateFlags flag)
	{
		for (SystemArray::iterator system_it = m_Systems.begin(), systems_end = m_Systems.end();
			system_it != systems_end; ++system_it)
		{
			SystemPtr &system = *system_it;
			if (system != excluded_system)
			{
				// Check for the current systems's name in the exclusions
				bool excluded = false;
				for (StringVector::const_iterator exclude_it = excluded_targets.begin(), targets_end = excluded_targets.end();
					exclude_it != targets_end; ++exclude_it)
				{
					if (system->GetName() == *exclude_it)
						excluded = true;
				}
				if (!excluded)
					system->RemoveFlag(flag);
			}
		}
	}

}
