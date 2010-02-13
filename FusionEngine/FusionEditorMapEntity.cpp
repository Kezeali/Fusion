/*
  Copyright (c) 2010 Fusion Project Team

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

#include "Common.h"

#include "FusionEditorMapEntity.h"

#include "FusionPhysicalEntity.h"
#include "FusionScriptManager.h"
#include "scriptstring.h"

#include <boost/lexical_cast.hpp>

#include <EMP/Core/TypeConverter.h>


namespace FusionEngine
{

	static int s_MaxPrimativeTypeId = Entity::pt_double;
	static const char* vectorComponentNames[] = {"x", "y"};
	static const char* colourComponentNames[] = {"r", "g", "b", "a"};

	EditorMapEntity::EditorMapEntity()
		: EMP::Core::DataSource()
	{
		VectorTypeInfoMap::value_type vectorTypeInfo( Entity::pt_vector, VectorTypeInfo(2, (const char**)&vectorComponentNames) );
		VectorTypeInfoMap::value_type colourTypeInfo( Entity::pt_colour, VectorTypeInfo(4, (const char**)&colourComponentNames) );
		m_VectorTypes.insert(vectorTypeInfo);
		m_VectorTypes.insert(colourTypeInfo);
	}

	EditorMapEntity::~EditorMapEntity()
	{
		if (fixture)
		{
			PhysicalEntity *physicalEntity = dynamic_cast<PhysicalEntity*>( entity.get() );
			if (physicalEntity != NULL && physicalEntity->IsPhysicsEnabled())
				physicalEntity->DestroyFixture(fixture);
		}
	}

	void EditorMapEntity::CreateEditorFixture()
	{
		if (fixture)
			return;

		PhysicalEntity *physicalEntity = dynamic_cast<PhysicalEntity*>( this->entity.get() );
		if (physicalEntity != NULL && physicalEntity->IsPhysicsEnabled())
		{
			b2FixtureDef iconFixtureDef;
			b2PolygonShape *shape = new b2PolygonShape();
			shape->SetAsBox(32.f*s_SimUnitsPerGameUnit, 32.f*s_SimUnitsPerGameUnit);
			iconFixtureDef.shape = shape;
			iconFixtureDef.isSensor = true;

			FixtureUserDataPtr user_data(new MapEntityFixtureUserData(this));
			fixture = physicalEntity->CreateFixture(&iconFixtureDef, "editor", user_data);
		}
	}

	std::string any_to_string(const boost::any &value)
	{
		if (value.type() == typeid(bool))
			return boost::lexical_cast<std::string>( boost::any_cast<bool>(value) );
		// Integer types
		else if (value.type() == typeid(int8_t))
			return boost::lexical_cast<std::string>( boost::any_cast<int8_t>(value) );
		else if (value.type() == typeid(int16_t))
			return boost::lexical_cast<std::string>( boost::any_cast<int16_t>(value) );
		else if (value.type() == typeid(int32_t))
			return boost::lexical_cast<std::string>( boost::any_cast<int32_t>(value) );
		else if (value.type() == typeid(int64_t))
			return boost::lexical_cast<std::string>( boost::any_cast<int64_t>(value) );
		// Unsigned ints
		else if (value.type() == typeid(uint8_t))
			return boost::lexical_cast<std::string>( boost::any_cast<uint8_t>(value) );
		else if (value.type() == typeid(uint16_t))
			return boost::lexical_cast<std::string>( boost::any_cast<uint16_t>(value) );
		else if (value.type() == typeid(uint32_t))
			return boost::lexical_cast<std::string>( boost::any_cast<uint32_t>(value) );
		else if (value.type() == typeid(uint64_t))
			return boost::lexical_cast<std::string>( boost::any_cast<uint64_t>(value) );
		// Floating point types
		else if (value.type() == typeid(float))
			return boost::lexical_cast<std::string>( boost::any_cast<float>(value) );
		else if (value.type() == typeid(double))
			return boost::lexical_cast<std::string>( boost::any_cast<double>(value) );
		// App Classes
		else if (value.type() == typeid(Entity*))
		{
			Entity *ent = boost::any_cast<Entity*>(value);
			return "[" + ent->GetType() + "] " + ent->GetName();
		}
		else if (value.type() == typeid(CScriptString*))
		{
			return boost::any_cast<CScriptString*>(value)->buffer;
		}
		else if (value.type() == typeid(CScriptString))
		{
			return boost::any_cast<CScriptString>(value).buffer;
		}
		else if (value.type() == typeid(Vector2*))
		{
			Vector2 *vec = boost::any_cast<Vector2*>(value);
			return boost::lexical_cast<std::string>(vec->x) + ", " + boost::lexical_cast<std::string>(vec->y);
		}
		else if (value.type() == typeid(Vector2))
		{
			Vector2 vec = boost::any_cast<Vector2>(value);
			return boost::lexical_cast<std::string>(vec.x) + ", " + boost::lexical_cast<std::string>(vec.y);
		}
		else
			return std::string();
	}

	template <typename T>
	inline void to_string(std::string &out, T *value)
	{
		out = boost::lexical_cast<std::string>( *value );
	}

	void to_string(std::string &out, int property_type, void *value)
	{
		switch (property_type)
		{
		case Entity::pt_bool:
			out = *static_cast<bool*>(value) ? "true" : "false";
			break;

		// Integer types
		case Entity::pt_int8:
			to_string( out, static_cast<int8_t*>(value) );
			break;
		case Entity::pt_int16:
			to_string( out, static_cast<int16_t*>(value) );
			break;
		case Entity::pt_int32:
			to_string( out, static_cast<int32_t*>(value) );
			break;
		case Entity::pt_int64:
			to_string( out, static_cast<int64_t*>(value) );
			break;

		// Unsigned ints
		case Entity::pt_uint8:
			to_string( out, static_cast<uint8_t*>(value) );
			break;
		case Entity::pt_uint16:
			to_string( out, static_cast<uint16_t*>(value) );
			break;
		case Entity::pt_uint32:
			to_string( out, static_cast<uint32_t*>(value) );
			break;
		case Entity::pt_uint64:
			to_string( out, static_cast<uint64_t*>(value) );
			break;

		// Floating point types
		case Entity::pt_float:
			to_string( out, static_cast<float*>(value) );
			break;
		case Entity::pt_double:
			to_string( out, static_cast<double*>(value) );
			break;

		// Entity
		case Entity::pt_entity:
			{
				Entity *ent = *static_cast<Entity**>(value);
				out = "[" + ent->GetType() + "] " + ent->GetName();
				break;
			}

			// String
		case Entity::pt_string:
			out = *static_cast<std::string*>(value);
			break;

			// Vector
		case Entity::pt_vector:
			{
				Vector2 *vec = static_cast<Vector2*>(value);
				out = boost::lexical_cast<std::string>(vec->x) + ", " + boost::lexical_cast<std::string>(vec->y);
				break;
			}
		}

		if (property_type & Entity::pt_pointer_flag)
		{
			if (property_type & Entity::pt_string)
				out = **static_cast<std::string**>(value);

			else if (property_type & Entity::pt_vector)
			{
				Vector2 *vec = *static_cast<Vector2**>(value);
				out = boost::lexical_cast<std::string>(vec->x) + ", " + boost::lexical_cast<std::string>(vec->y);
			}
		}
	}

	void EditorMapEntity::GetRow(EMP::Core::StringList& row, const EMP::Core::String& table, int row_index, const EMP::Core::StringList& columns)
	{
		if (row_index < 0)
			return;

		if (table == "properties")
		{
			if ((size_t)row_index >= entity->GetPropertyCount())
				return;

			for (size_t i = 0; i < columns.size(); i++)
			{
				if (columns[i] == "name")
				{
					row.push_back(entity->GetPropertyName(row_index).c_str());
				}
				else if (columns[i] == "index")
				{
					row.push_back( boost::lexical_cast<std::string>(row_index).c_str() );
				}
				else if (columns[i] == "array_index")
				{
					row.push_back("top");
				}
				else if (columns[i] == EMP::Core::DataSource::CHILD_SOURCE)
				{
					int type = entity->GetPropertyType(row_index);
					if (type & Entity::pt_array_flag ||
						(type & Entity::pt_vector) == Entity::pt_vector ||
						(type & Entity::pt_colour) == Entity::pt_colour)
					{
						EMP::Core::String strIndex;
						EMP::Core::TypeConverter<unsigned int, EMP::Core::String>::Convert(row_index, strIndex);
						row.push_back(GetDataSourceName()+"."+strIndex);
					}
				}
			}
		}
		else
		{
			unsigned int index;
			if (!EMP::Core::TypeConverter<EMP::Core::String, unsigned int>::Convert(table, index)) return;
			int type = entity->GetPropertyType(index);

			for (size_t i = 0; i < columns.size(); i++)
			{
				if (columns[i] == "name")
				{
					if (type & Entity::pt_array_flag)
					{
						EMP::Core::String name;
						if (EMP::Core::TypeConverter<unsigned int, EMP::Core::String>::Convert(row_index, name))
							row.push_back(name);
					}
					// Non array types with multiple components (vector, colour)
					else if (type > s_MaxPrimativeTypeId)
					{
						VectorTypeInfoMap::const_iterator _where = m_VectorTypes.find(type & ~Entity::pt_pointer_flag);
						if (_where != m_VectorTypes.end())
						{
							const VectorTypeInfo &info = _where->second;
							if ((size_t)row_index < info.size)
								row.push_back( info.names[(size_t)row_index] );
						}
					}
				}
				else if (columns[i] == "index")
				{
					row.push_back(table);
				}
				else if (columns[i] == "array_index")
				{
					//if (type & Entity::pt_array_flag)
					//{
						EMP::Core::String name;
						if (EMP::Core::TypeConverter<unsigned int, EMP::Core::String>::Convert(row_index, name))
							row.push_back(name);
					//}
					//else if (type > s_MaxPrimativeTypeId)
					//{
					//	VectorTypeInfoMap::const_iterator _where = m_VectorTypes.find(type);
					//	if (_where != m_VectorTypes.end())
					//	{
					//		const VectorTypeInfo &info = _where->second;
					//		if ((size_t)row_index < info.size)
					//			row.push_back(info.names[(size_t)row_index]);
					//	}
					//}
				}
			}
		}
	}

	int EditorMapEntity::GetNumRows(const EMP::Core::String& table)
	{
		if (table == "properties")
		{
			return entity->GetPropertyCount();
		}
		else
		{
			unsigned int index;
			EMP::Core::TypeConverter<EMP::Core::String, unsigned int>::Convert(table, index);
			int type = entity->GetPropertyType(index);

			if (type & Entity::pt_array_flag)
			{
				return entity->GetPropertyArraySize(index);
			}
			else if (type > s_MaxPrimativeTypeId)
			{
				VectorTypeInfoMap::const_iterator _where = m_VectorTypes.find(type);
				if (_where != m_VectorTypes.end())
				{
					const VectorTypeInfo &info = _where->second;
					return info.size;
				}
			}
		}

		return 0;
	}

}
