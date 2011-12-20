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

#include "FusionStableHeaders.h"

#include "FusionSharedPropertyCollection.h"

namespace FusionEngine
{

	struct SerialisingVisitor : public boost::static_visitor<>
	{
		SerialisingVisitor(CL_IODevice& _device) : device( _device )
		{}
		template <typename T>
		void operator() (const T& val)
		{
			static_assert(std::is_fundamental<T>::value, "The given type can't be serialised");

			if (device.is_little_endian() == CL_Endian::is_system_big())
			{
				T outVal = val;
				CL_Endian::swap(&outVal, sizeof(T));
				device.write(&outVal, sizeof(T));
			}
			else
				device.write(&val, sizeof(T));
		}

		template <>
		void operator()<std::string> (const std::string& val)
		{
			device.write_string_a(val);
		}

		template <>
		void operator()<Vector2> (const Vector2& val)
		{
			if (device.is_little_endian() == CL_Endian::is_system_big())
			{
				(*this)(val.x);
				(*this)(val.y);
			}
			else
				device.write(&val, sizeof(Vector2));
		}

		template <>
		void operator()<CL_Colorf> (const CL_Colorf& val)
		{
			if (device.is_little_endian() == CL_Endian::is_system_big())
			{
				(*this)(val.a);
				(*this)(val.r);
				(*this)(val.g);
				(*this)(val.b);
			}
			else
				device.write(&val, sizeof(CL_Colorf));
		}

		CL_IODevice& device;
	};

	struct DeserialisingVisitor : public boost::static_visitor<>
	{
		DeserialisingVisitor(CL_IODevice& _device) : device( _device )
		{}
		template <typename T>
		void operator() (T& val)
		{
			static_assert(std::is_fundamental<T>::value, "The given type can't be deserialised");

			T inVal;
			int actualLen = device.read(&inVal, sizeof(T)); // read into a temp, to avoid changing the output if read fails
			if (actualLen != sizeof(T))
				return; // source didn't contain expected data (should throw an exception here)

			if (device.is_little_endian() == CL_Endian::is_system_big())
				CL_Endian::swap(&inVal, sizeof(T));

			val = inVal;
		}

		template <>
		void operator()<std::string> (std::string& val)
		{
			val = device.read_string_a();
		}

		template <>
		void operator()<Vector2> (Vector2& val)
		{
			if (device.is_little_endian() == CL_Endian::is_system_big())
			{
				(*this)(val.x);
				(*this)(val.y);
			}
			else
			{
				Vector2 inVal;
				int actualLen = device.read(&inVal, sizeof(Vector2)); // read into a temp, to avoid changing the output if read fails
				if (actualLen != sizeof(Vector2))
					return; // source didn't contain expected data (should throw an exception here)
				val = inVal;
			}
		}

		template <>
		void operator()<CL_Colorf> (CL_Colorf& val)
		{
			if (device.is_little_endian() == CL_Endian::is_system_big())
			{
				(*this)(val.a);
				(*this)(val.r);
				(*this)(val.g);
				(*this)(val.b);
			}
			else
			{
				CL_Colorf inVal;
				int actualLen = device.read(&inVal, sizeof(CL_Colorf)); // read into a temp, to avoid changing the output if read fails
				if (actualLen != sizeof(CL_Colorf))
					return; // source didn't contain expected data (should throw an exception here)
				val = inVal;
			}
		}

		CL_IODevice& device;
	};

	PropertyCollection::PropertyCollection()
		: m_ChangedProperties(0)
	{
	}

	size_t PropertyCollection::AddProperty(const std::string& name)
	{
		const size_t index = m_Properties.size();

		const uint32_t flag = index < 32 ? (1 << index) : 0; // shared properties after the first 32 are never serialised in SerialiseChanges

		m_Properties.push_back(SharedProperty(name, flag));
		return index;
	}

	void PropertyCollection::Set(const size_t index, const PropertyVariant& value)
	{
		auto& prop = m_Properties[index];
#ifdef _DEBUG
		if (prop.value.which() != value.which())
			FSN_EXCEPT(InvalidPropertyAssignment, std::string("Can't assign value of type ") + value.type().name() + " to " + prop.name);
#endif
		prop.value = value;
		prop.changed = true;

		m_ChangedProperties &= prop.change_flag;
	}

	void PropertyCollection::Serialise(CL_IODevice& device)
	{
		device.write_uint8(0); // first bit indicates that this is a full serialisation (not just changes), other 7 are wasted :( (may be used in the future, tho)
		SerialisingVisitor vis(device);
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			boost::apply_visitor(vis, it->value);
		}
	}

	void PropertyCollection::SerialiseChanges(CL_IODevice& device)
	{
		m_ChangedProperties &= 1 << 31; // first bit is reserved to indicate that this is a Changes serialisation: make sure it's set
		device.write_uint32(m_ChangedProperties);
		SerialisingVisitor vis(device);
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			if (m_ChangedProperties & it->change_flag)
				boost::apply_visitor(vis, it->value);
		}
	}

	void PropertyCollection::Deserialise(CL_IODevice& device)
	{
		m_ChangedProperties = 0;
		m_ChangedProperties = device.read_uint8() << 31;
		bool fullData = false;
		if (m_ChangedProperties & 1 << 31) // the first bit indicates that this is changes data: read the rest of the flags
		{
			auto back = &m_ChangedProperties + 1;
			device.read(back, 3);
		}
		else
			fullData = true;

		DeserialisingVisitor vis(device);
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			if (fullData || m_ChangedProperties & it->change_flag)
				boost::apply_visitor(vis, it->value);
		}

		m_ChangedProperties = 0;
	}

}
