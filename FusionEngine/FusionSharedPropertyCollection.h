/*
*  Copyright (c) 2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#ifndef H_FusionSharedPropertyCollection
#define H_FusionSharedPropertyCollection

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionSharedPropertyTypes.h"

namespace FusionEngine
{

	//! Thrown when something attempts to assign a value of the wrong type to a SharedProperty
	class InvalidPropertyAssignment : public Exception
	{
	public:
		InvalidPropertyAssignment(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};

	struct SharedPropertyDef
	{
		std::string name;
	};

	struct SharedProperty
	{
		SharedProperty(const std::string& _name, const uint32_t _change_flag)
			: name(_name),
			change_flag(_change_flag),
			changed(false)
		{}

		std::string name;
		PropertyVariant value;
		uint32_t change_flag;
		bool changed;
	};

	class SharedPropertyUser
	{
	public:
		PropertyVariant GetProperty(std::string name);
		std::unordered_map<std::string, size_t> m_PropertyIndices;
		// ... etc
	};

	//! Stores shared properties
	class PropertyCollection
	{
	public: 
		PropertyCollection();

		//! Adds a prop
		/*
		* \return The index of the added prop
		*/
		size_t AddProperty(const std::string& name);

		void Set(const size_t index, const PropertyVariant& value);
		PropertyVariant Get(const size_t index);

		void Serialise(CL_IODevice& device);
		void SerialiseChanges(CL_IODevice& device);

		void Deserialise(CL_IODevice& device);
	private:
		std::vector< SharedProperty > m_Properties;
		uint32_t m_ChangedProperties;
	};

}

#endif
