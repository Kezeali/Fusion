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

#include "FusionCommon.h"

#include "FusionInputDefinitionLoader.h"

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
				throw FileTypeException("InputDefinitionLoader::LoadInputs", "Not an input declaration file", __FILE__, __LINE__);

			ticpp::Iterator< ticpp::Element > group( "group" );
			for ( group = group.begin(pElem); group != group.end(); group++ )
			{
				loadGroup(*group);
			}

		}
		catch (ticpp::Exception &ex)
		{
			//SendToConsole("Failed to load input plugin: " + std::string(ex.what()));

			throw FileSystemException("InputDefinitionLoader::LoadInputs", ex.what(), __FILE__, __LINE__);
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
				InputDefinition cd;
				cd.m_Group = groupName;
				cd.m_Name = child->GetAttribute("name");
				cd.m_UIName = child->GetAttribute("uiname");
				cd.m_Description = child->GetAttribute("description");

				std::string isAnalog = child->GetAttribute("analog");
				//fe_tolower(isAnalog);
				cd.m_Analog = CL_StringHelp::local8_to_bool(isAnalog);

				std::string isToggle = child->GetAttribute("toggle");
				//fe_tolower(isToggle);
				cd.m_Toggle = CL_StringHelp::local8_to_bool(isToggle);

				m_InputDefinitions[cd.m_Name] = cd;
			}
		}

	}

	void InputDefinitionLoader::Clear()
	{
		m_InputDefinitions.clear();
	}

	const InputDefinitionLoader::InputDefinitionMap &InputDefinitionLoader::GetInputDefinitions() const
	{
		return m_InputDefinitions;
	}

	bool InputDefinitionLoader::IsDefined(const std::string &inputName) const
	{
		return m_InputDefinitions.find(inputName) != m_InputDefinitions.end();
	}

};