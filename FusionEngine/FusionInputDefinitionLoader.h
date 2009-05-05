/*
  Copyright (c) 2008 Fusion Project Team

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

#ifndef Header_FusionEngine_InputPluginLoader
#define Header_FusionEngine_InputPluginLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceManager.h"
#include "FusionScriptReference.h"
//#include "FusionCommand.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Describes an input as loaded from XML
	 */
	class InputDefinition
	{
	public:
		bool m_Analog;
		bool m_Toggle;
		std::string m_Group;
		std::string m_Description;
		std::string m_UIName;
		std::string m_Name;
	};

	/*!
	 * \brief
	 * Lists command (input) definitions.
	 *
	 * Reads an XML command definition document
	 *
	 * \sa
	 * OpenXml
	 */
	class InputDefinitionLoader
	{
	public:
		//! InputDefinitions listed by shortname
		typedef std::tr1::unordered_map<std::string, InputDefinition> InputDefinitionMap;

	public:
		InputDefinitionLoader() {}
		InputDefinitionLoader(const ticpp::Document &tidoc);

		//! Loads an input def. file
		void Load(const ticpp::Document &doc);

		//! Unloads an input plugin
		//void DropInputs(const std::string &name);

		//! Unloads everything
		void Clear();

		//! Generates and compiles a Command class
		/*!
		 * The Command class generated will contain properties corresponding
		 * to each command node in the previously parsed XML documents
		 */
		//void CreateCommandClass(ScriptingEngine *eng);

		//ScriptObject CreateCommand(ScriptingEngine *eng, const Command& command);

		const InputDefinitionMap &GetInputDefinitions() const;

	protected:
		InputDefinitionMap m_InputDefinitions;

	protected:
		void tidy();

		void loadGroup(ticpp::Element &groupElm);

	};

}

#endif
