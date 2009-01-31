/*
  Copyright (c) 2007 Fusion Project Team

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

#ifndef Header_FusionEngine_Command
#define Header_FusionEngine_Command

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	class InputState
	{
	public:
		InputState()
			: m_Down(false), m_Changed(false), m_Value(0.0f)
		{}
		bool m_Down;
		bool m_Changed; // If the button was pressed / released / both since the last command
		float m_Value;
	};

	class Command
	{
	protected:
		typedef std::tr1::shared_ptr<InputState> InputStatePtr;
		typedef std::tr1::unordered_map<std::string, InputStatePtr> InputStateMap;

		InputStateMap m_InputStates;

		ScriptObject m_ScriptCommand;

	public:
		Command()
		{
		}

		Command(InputStateMap::size_type size)
		{
			m_InputStates.rehash(size);
		}

		//! Copy
		Command(const Command& other)
		{
			m_InputStates = other.m_InputStates;
		}

		ScriptObject GetScriptCommand()
		{
			return m_ScriptCommand;
		}

	};
	//typedef std::map<std::string, InputState> Command;

}

#endif
