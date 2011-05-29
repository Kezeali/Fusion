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

#ifndef H_FusionEngine_InputPluginLoader
#define H_FusionEngine_InputPluginLoader

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionResourceManager.h"

#include <memory>
#include <string>

#include "FusionXML.h"

namespace FusionEngine
{

	/*!
	 * \brief
	 * Describes an input as loaded from XML
	 */
	class InputDefinition
	{
	public:
		bool Analog;
		bool Toggle;
		std::string Group;
		std::string Description;
		std::string UIName;
		std::string Name;

		// Index used for network communication, etc
		size_t Index;
	};

	typedef std::shared_ptr<InputDefinition> InputDefinitionPtr;

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
		typedef std::tr1::unordered_map<std::string, InputDefinitionPtr> InputDefinitionMap;
		typedef std::vector<InputDefinitionPtr> InputDefinitionArray;

	public:
		InputDefinitionLoader() {}
		InputDefinitionLoader(const ticpp::Document &tidoc);

		//! Loads an input def. file
		void Load(const ticpp::Document &doc);

		//! Unloads everything
		void Clear();

		const InputDefinitionArray &GetInputDefinitions() const;

		bool IsDefined(const std::string &input_name) const;

		bool IsDefined(size_t input_index) const;

		const InputDefinition &GetInputDefinition(const std::string &input_name) const;
		const InputDefinition &GetInputDefinition(size_t input_index) const;

	protected:
		InputDefinitionMap m_InputDefinitions;
		InputDefinitionArray m_InputDefinitionsByIndex;

	protected:
		void loadGroup(ticpp::Element &groupElm);

	};

}

#endif
