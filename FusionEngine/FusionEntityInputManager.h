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

#ifndef Header_FusionEngine_EntityCommandManager
#define Header_FusionEngine_EntityCommandManager

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

#include "FusionInputHandler.h"
#include "FusionEntity.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Manages commands over time.
	 *
	 * Commands are much like quake engine usercmds or Zen Networked Physics' 'Move' class (by Glenn Fiedler)
	 */
	class EntityCommandManager
	{
	public:
		//! Player+input_name mapped to filter+control_name
		typedef std::map<std::string, std::string> InputKeyMap;
		typedef std::vector<EntityInput> CommandsList;
		typedef std::vector<CommandsList> PlayerCommandsLists;

	public:
		//! Basic constructor.
		EntityCommandManager(FusionInput* input, unsigned int command_backup_length = 500);

	public:
		//! Stores the current commands at the given index
		void Store(int sequence_number, float sample_time);
		//! Retreives a command from the given index, for the given player
		EntityInput Retreive(int sequence_number, unsigned int player);

		//! Adds an input to the EntityInput struct
		void AddInput(const std::string& input_name);

		/*!
		 * Player is the player for whom this applies, button name is the key to ask the 
		 * InputHandler about, and input_name is the name of the input at the script level
		 * (e.g. Thrust, PrimaryFire). input_name is also the identifier used in the 
		 * scriptstruct used to pass the input to scripts. input_name must already exist
		 * (i.e. AddInput(input_name, analog) must have been called at some point in the past)
		 */
		void MapKey(const std::string& input_name, unsigned int player, const std::string& control_name);

	protected:
		FusionInput* m_Input;

		InputKeyMap m_Inputs;
		PlayerCommandsLists m_PlayerCommands;

		unsigned int m_BackupLength;
		
	};

}

#endif
