/*
  Copyright (c) 2006 Fusion Project Team

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
*/

#include "FusionStableHeaders.h"

#include "FusionState.h"


#include "FusionStateMessage.h"
#include "FusionException.h"

namespace FusionEngine
{

	System::~System()
	{
		for (MessageList::iterator it = m_Messages.begin(), end = m_Messages.end(); it != end; ++it)
			delete *it;
		m_Messages.clear();
	}

	SystemMessage *System::PopMessage()
	{
		if (m_Messages.empty())
			return NULL;

		SystemMessage *ret = m_Messages.front();
		m_Messages.pop_front();

		return ret;
	}

	void System::PushMessage(SystemMessage *m)
	{
		m_Messages.push_back(m);
	}

	bool System::IsDependency(const std::string &system_name)
	{
		return m_Dependencies.find(system_name) != m_Dependencies.end();
	}

	void System::AddDependency(const std::string &system_name)
	{
		m_Dependencies.insert(system_name);
	}

	void System::RemoveDependency(const std::string &system_name)
	{
		m_Dependencies.erase(system_name);
	}

	void System::SetFlags(unsigned char flags)
	{
		m_StateFlags = flags;
	}

	void System::AddFlag(System::StateFlags flag)
	{
		m_StateFlags |= flag;
	}

	void System::RemoveFlag(System::StateFlags flag)
	{
		m_StateFlags = m_StateFlags & ~((unsigned char)flag);
	}

	bool System::CheckFlag(System::StateFlags flag)
	{
		return (m_StateFlags & flag) == flag;
	}

	void System::SetBlocking(bool blocking)
	{
		m_Blocking = blocking;
	}

	bool System::IsBlocking() const
	{
		return m_Blocking;
	}

}
