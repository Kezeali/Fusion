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

#include "PrecompiledHeaders.h"

#include "FusionInputDefinitionLoader.h"

#include "FusionExceptionFactory.h"

namespace FusionEngine
{

	InputDefinitionLoader::InputDefinitionLoader(const ticpp::Document &tidoc)
	{
		Load(tidoc);
	}

	void InputDefinitionLoader::Load(const ticpp::Document &tidoc)
	{
		try
		{
			ticpp::Document doc(tidoc);
			ticpp::Element* pElem = doc.FirstChildElement();

			if (pElem->Value() != "inputdefinitions")
				FSN_EXCEPT(FileTypeException, "Not an input declaration file");

			ticpp::Iterator< ticpp::Element > group( "group" );
			for ( group = group.begin(pElem); group != group.end(); group++ )
			{
				loadGroup(*group);
			}

		}
		catch (ticpp::Exception &ex)
		{
			//SendToConsole("Failed to load input plugin: " + std::string(ex.what()));

			FSN_EXCEPT(ExCode::IO, ex.what());
		}
	}

	void InputDefinitionLoader::loadGroup(ticpp::Element &groupElm)
	{
		//InputGroup::ind_type grpStart = m_Inputs.size();
		std::string groupName = groupElm.GetAttribute("name");

		ticpp::Iterator< ticpp::Element > child;
		for ( child = child.begin( &groupElm ); child != child.end(); child++ )
		{
			// I can't decide on the name for the definition elements, so
			//  we recognise both here:
			if (child->Value() == "input" || child->Value() == "command")
			{
				InputDefinitionPtr cd(new InputDefinition);
				cd->Group = groupName;
				cd->Name = child->GetAttribute("name");
				cd->UIName = child->GetAttribute("uiname");
				cd->Description = child->GetAttribute("description");

				std::string isAnalog = child->GetAttribute("analog");
				//fe_tolower(isAnalog);
				cd->Analog = clan::StringHelp::local8_to_bool(isAnalog.c_str());

				std::string isToggle = child->GetAttribute("toggle");
				//fe_tolower(isToggle);
				cd->Toggle = clan::StringHelp::local8_to_bool(isToggle.c_str());

				m_InputDefinitions[cd->Name] = cd;

				cd->Index = m_InputDefinitionsByIndex.size();
				m_InputDefinitionsByIndex.push_back(cd);
			}
		}

	}

	void InputDefinitionLoader::Clear()
	{
		m_InputDefinitions.clear();
	}

	const InputDefinitionLoader::InputDefinitionArray &InputDefinitionLoader::GetInputDefinitions() const
	{
		return m_InputDefinitionsByIndex;
	}

	bool InputDefinitionLoader::IsDefined(const std::string &input_name) const
	{
		return m_InputDefinitions.find(input_name) != m_InputDefinitions.end();
	}

	bool InputDefinitionLoader::IsDefined(size_t input_index) const
	{
		return input_index < m_InputDefinitionsByIndex.size();
	}

	const InputDefinition &InputDefinitionLoader::GetInputDefinition(const std::string &input_name) const
	{
		InputDefinitionMap::const_iterator _where = m_InputDefinitions.find(input_name);
		if (_where == m_InputDefinitions.end())
			FSN_EXCEPT(ExCode::IO, "Input named " + input_name + " is not defined");
		return *_where->second;
	}

	const InputDefinition &InputDefinitionLoader::GetInputDefinition(uint16_t input_index) const
	{
		if (input_index >= m_InputDefinitionsByIndex.size())
			FSN_EXCEPT(ExCode::IO, "Input index given is not defined");
		return *m_InputDefinitionsByIndex[input_index];
	}

};