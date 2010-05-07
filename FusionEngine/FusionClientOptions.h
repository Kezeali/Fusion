/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#ifndef Header_FusionEngine_ClientOptions
#define Header_FusionEngine_ClientOptions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionSingleton.h"

#include "FusionPrerequisites.h"
#include "FusionXML.h"

#include <ClanLib/Core/System/mutex.h>

namespace FusionEngine
{

	/*!
	 * \brief
	 * Loads / saves options files.
	 *
	 * \remarks Threadsafe
	 */
	class ClientOptions
	{
	public:
		//! Constructor
		ClientOptions(const std::string &type = "clientoptions");
		//! Constructor +file
		ClientOptions(const std::string &filename, const std::string &type);
		//! Clears controls
		~ClientOptions();

	public:
		typedef std::unordered_map<std::string, std::string> VarMap;

		// Each player has their own var map
		typedef std::vector<VarMap> PlayerVarMapArray;

	public:
		VarMap m_Variables;
		PlayerVarMapArray m_PlayerVariables;

	public:
		//! Saves to the most recently loaded file.
		bool Save();
		//! Saves the current options to a file
		bool SaveToFile(const std::string &filename);
		//! Loads a set of options from a file
		/*!
		* \param filename
		* The config file to open.
		*
		* \param default_if_missing
		* Looks for the default config for the given file under '/default-[filename]'.
		*/
		bool LoadFromFile(const std::string &filename, bool default_if_missing = true);


		bool SetOption(const std::string& name, const std::string& value);
		void SetMultipleOptions(const std::tr1::unordered_map<std::string, std::string>& pairs);

		bool SetPlayerOption(unsigned int player, const std::string& name, const std::string& value);

		bool GetOption(const std::string &name, std::string *val) const;
		bool GetOption(const std::string &name, int *ret) const;
		std::string GetOption_str(const std::string &name) const;
		bool GetOption_bool(const std::string &name) const;

		bool GetPlayerOption(unsigned int player, const std::string& name, std::string *val) const;

	protected:
		mutable CL_Mutex m_Mutex;

		std::string m_Type;
		//! Last opened options file
		std::string m_LastFile;

		void insertVarMapIntoDOM(ticpp::Element* parent, const VarMap &vars);
		void loadPlayerOptions(const ticpp::Element *const opts_root);
		//void loadKeys(const ticpp::Element &keysroot);

	};

}

#endif
