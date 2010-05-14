
#include "FusionStableHeaders.h"

#include "FusionScriptedEntity.h"

#include <ScriptUtils/Inheritance/TypeTraits.h>

#include "FusionCommon.h"
#include "FusionExceptionFactory.h"
#include "FusionResourceManager.h"
#include "scriptstring.h"

namespace FusionEngine
{

	int ScriptedEntity::s_EntityTypeId = -1;
	int ScriptedEntity::s_ScriptEntityTypeId = -1;

	const type_info &ToCppType(int type_id, ScriptManager *engine)
	{
		if (engine == NULL)
			engine = ScriptManager::getSingletonPtr();

		if (type_id == asTYPEID_BOOL)
			return typeid(bool);
		// Integer types
		else if (type_id == asTYPEID_INT8)
			return typeid(int8_t);
		else if (type_id == asTYPEID_INT16)
			return typeid(int16_t);
		else if (type_id == asTYPEID_INT32)
			return typeid(int32_t);
		else if (type_id == asTYPEID_INT64)
			return typeid(int64_t);
		// ... unsigned
		else if (type_id == asTYPEID_UINT8)
			return typeid(uint8_t);
		else if (type_id == asTYPEID_UINT16)
			return typeid(uint16_t);
		else if (type_id == asTYPEID_UINT32)
			return typeid(uint32_t);
		else if (type_id == asTYPEID_UINT64)
			return typeid(uint64_t);
		// Floating point types
		else if (type_id == asTYPEID_FLOAT)
			return typeid(float);
		else if (type_id == asTYPEID_DOUBLE)
			return typeid(double);

		// Pointers / handles
		else if (type_id & asTYPEID_APPOBJECT)
		{
			if (engine == NULL)
				FSN_EXCEPT(ExCode::InvalidArgument, "CppType", "Can't get application-defined type without a valid ScriptingManager");

			return typeid(ScriptedEntity*);
		}
		FSN_EXCEPT(ExCode::InvalidArgument, "ToCppType", "Unknown Angelscript type");
	}

	// Get
	template <typename T>
	void accessPropValueOfType(boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		cpp_obj = *(T*)obj->GetAddressOfProperty(property_index);
	}
	//Set
	template <typename T>
	void accessPropValueOfType(const boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		*(T*)obj->GetAddressOfProperty(property_index) = boost::any_cast<T>( cpp_obj );
	}

	// Get (ScirptedEntity specialization)
	template <>
	void accessPropValueOfType<ScriptedEntity*>(boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		asIScriptObject *value = *static_cast<asIScriptObject**>( obj->GetAddressOfProperty(property_index) );
		cpp_obj = ScriptedEntity::GetAppObject(value);
	}
	// Set (ScriptedEntity specialization)
	template <>
	void accessPropValueOfType<ScriptedEntity*>(const boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		ScriptedEntity *value = boost::any_cast<ScriptedEntity*>( cpp_obj );
		*(asIScriptObject**)obj->GetAddressOfProperty(property_index) = ScriptedEntity::GetScriptObject(value);
	}

	// Decides the correct type, then calls either of the overloaded methods above on that type
	template <typename boost_any>
	void accessScriptPropValue(boost_any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		// Make sure the type is correct
		BOOST_MPL_ASSERT(( std::tr1::is_convertible< std::tr1::remove_const<boost_any>, boost::any > ));

		int type_id = obj->GetPropertyTypeId(property_index);

		if (type_id == asTYPEID_BOOL)
			accessPropValueOfType<bool>(cpp_obj, obj, property_index);
		// Integer types
		else if (type_id == asTYPEID_INT8)
			accessPropValueOfType<int8_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_INT16)
			accessPropValueOfType<int16_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_INT32)
			accessPropValueOfType<int32_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_INT64)
			accessPropValueOfType<int64_t>(cpp_obj, obj, property_index);
		// ... unsigned
		else if (type_id == asTYPEID_UINT8)
			accessPropValueOfType<uint8_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_UINT16)
			accessPropValueOfType<uint16_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_UINT32)
			accessPropValueOfType<uint32_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_UINT64)
			accessPropValueOfType<uint64_t>(cpp_obj, obj, property_index);
		// Floating point types
		else if (type_id == asTYPEID_FLOAT)
			accessPropValueOfType<float>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_DOUBLE)
			accessPropValueOfType<double>(cpp_obj, obj, property_index);

		// Pointers / handles
		else if (type_id & asTYPEID_APPOBJECT)
		{
			ScriptManager *engine = NULL; // TODO: engine param?
			if (engine == NULL)
				engine = ScriptManager::getSingletonPtr();
			FSN_ASSERT(engine == NULL);

			if (type_id & ScriptedEntity::s_ScriptEntityTypeId)
			{
				accessPropValueOfType<ScriptedEntity*>(cpp_obj, obj, property_index);
			}
			else if (type_id & ScriptedEntity::s_EntityTypeId)
			{
				accessPropValueOfType<Entity*>(cpp_obj, obj, property_index);
			}
			else if (type_id & engine->GetStringTypeId())
			{
				if (type_id & asTYPEID_OBJHANDLE)
					accessPropValueOfType<CScriptString*>(cpp_obj, obj, property_index);
				else
					accessPropValueOfType<CScriptString>(cpp_obj, obj, property_index);
			}
			else if (type_id & engine->GetVectorTypeId())
			{
				if (type_id & asTYPEID_OBJHANDLE)
					accessPropValueOfType<Vector2*>(cpp_obj, obj, property_index);
				else
					accessPropValueOfType<Vector2>(cpp_obj, obj, property_index);
			}
		}
	}

	// Returns 1 for exact type, 2 for implicitly convertable, 0 for types that require explicit conversion / non-convertable
	char IsEquivilantType(const boost::any &cpp_obj, int type_id)
	{
		if (cpp_obj.type() == ToCppType(type_id))
			return 1;

		// Integer types
		else if (type_id >= asTYPEID_INT8 && type_id <= asTYPEID_INT64 &&
			(cpp_obj.type() == typeid(int8_t) ||
			cpp_obj.type() == typeid(int16_t) ||
			cpp_obj.type() == typeid(int32_t) ||
			cpp_obj.type() == typeid(int64_t)))
			return 2;
		// Unsigned Integer types
		else if (type_id >= asTYPEID_UINT8 && type_id <= asTYPEID_UINT64 &&
			(cpp_obj.type() == typeid(uint8_t) ||
			cpp_obj.type() == typeid(uint16_t) ||
			cpp_obj.type() == typeid(uint32_t) ||
			cpp_obj.type() == typeid(uint64_t)))
			return 2;
		// Float types
		else if (type_id == asTYPEID_FLOAT || type_id == asTYPEID_DOUBLE &&
			(cpp_obj.type() == typeid(float) ||
			cpp_obj.type() == typeid(double)))
			return 2;

		return 0;
	}

	ResourceDescription::ResourceDescription()
		: m_Priority(0),
		m_ScriptPropertyIndex(-1)
	{}

	ResourceDescription::~ResourceDescription()
	{}
	
	void ResourceDescription::SetType(const std::string &type)
	{
		m_Type = type;
	}

	const std::string &ResourceDescription::GetType() const
	{
		return m_Type;
	}

	void ResourceDescription::SetPropertyName(const std::string &name)
	{
		m_ScriptPropertyName = name;
	}

	void ResourceDescription::SetResourceName(const std::string &name)
	{
		m_ResourceName = name;
	}

	void ResourceDescription::SetPriority(int priority)
	{
		m_Priority = priority;
	}

	void ResourceDescription::ParseTags(const std::string &tags)
	{
		typedef std::string::size_type str_size_t;

		if (tags.empty())
			return;

		str_size_t
			tokenBegin = 0,
			tokenEnd = tags.find(','),
			contentEnd = 0,
			contentLength = 0;
		std::string token;

		// Not finding a delimiter indicates that there is only one tag
		if (tokenEnd == std::string::npos)
		{
			m_Tags.insert(tags);
			return;
		}

		while (tokenEnd != std::string::npos)
		{
			// trim whitespace
			tokenBegin = tags.find_first_not_of(" \t", tokenBegin);
			for (contentEnd = tokenEnd-1; contentEnd > tokenBegin; --contentEnd)
				if (tags[contentEnd] != ' ' && tags[contentEnd] != '\t') break;
			contentLength = contentEnd+1 - tokenBegin;

			if (contentLength > 0)
			{
				token.assign(tags, tokenBegin, contentLength);
				
				m_Tags.insert(token);
			}

			tokenBegin = tokenEnd + 1;
			tokenEnd = tags.find(',', tokenBegin);
		}
	}

	void ResourceDescription::SetPropertyIndex(int index)
	{
		m_ScriptPropertyIndex = index;
	}

	const std::string &ResourceDescription::GetPropertyName() const
	{
		return m_ScriptPropertyName;
	}

	const std::string &ResourceDescription::GetResourceName() const
	{
		return m_ResourceName;
	}

	int ResourceDescription::GetPriority() const
	{
		return m_Priority;
	}

	const TagStringSet &ResourceDescription::GetTags() const
	{
		return m_Tags;
	}

	bool ResourceDescription::HasTag(const std::string &tag) const
	{
		return m_Tags.find(tag) != m_Tags.end();
	}

	int ResourceDescription::GetPropertyIndex() const
	{
		return m_ScriptPropertyIndex;
	}

	ScriptedEntity::ScriptedEntity()
		: PhysicalEntity(),
		m_DefaultPosition(0.f, 0.f),
		m_DefaultAngle(0.f)
	{}

	ScriptedEntity::ScriptedEntity(ScriptObject self, const std::string &name)
		: PhysicalEntity(name),
		m_ScriptObject(self),
		m_DefaultPosition(0.f, 0.f),
		m_DefaultAngle(0.f)
	{}

	ScriptedEntity::~ScriptedEntity()
	{
	}

	void ScriptedEntity::SetPath(const std::string &path)
	{
		m_Path = path;
	}

	void ScriptedEntity::SetSyncProperties(const ScriptedEntity::PropertiesArray &properties)
	{
		m_SyncedProperties = properties;
	}

	unsigned int ScriptedEntity::GetPropertyCount() const
	{
		return m_SyncedProperties.size();
	}

	std::string ScriptedEntity::GetPropertyName(unsigned int index) const
	{
		int propIndex = getScriptPropIndex(index);
		return m_ScriptObject.GetScriptObject()->GetPropertyName(propIndex);
	}

	boost::any ScriptedEntity::GetPropertyValue(unsigned int index) const
	{
		boost::any value;
		int propIndex = getScriptPropIndex(index);
		accessScriptPropValue(value, m_ScriptObject.GetScriptObject(), propIndex);
		return value;
	}

	void ScriptedEntity::SetPropertyValue(unsigned int index, const boost::any &value)
	{
		int propIndex = getScriptPropIndex(index);
		accessScriptPropValue(value, m_ScriptObject.GetScriptObject(), propIndex);
	}

	EntityPtr ScriptedEntity::GetPropertyEntity(unsigned int index, unsigned int array_index) const
	{
		asIScriptObject *scriptEntity = *static_cast<asIScriptObject**>( GetAddressOfProperty(index, array_index) );
		return EntityPtr(ScriptedEntity::GetAppObject(scriptEntity));
	}

	void ScriptedEntity::SetPropertyEntity(unsigned int index, unsigned int array_index, const EntityPtr &value)
	{
		asIScriptObject **scriptEntity = static_cast<asIScriptObject**>( GetAddressOfProperty(index, array_index) );
		(*scriptEntity) = ScriptedEntity::GetScriptObject(value.get());
	}

	unsigned int ScriptedEntity::GetPropertyArraySize(unsigned int index) const
	{
		int propIndex = getScriptPropIndex(index);
		if (m_ScriptObject.GetScriptObject()->GetPropertyTypeId(propIndex) & asTYPEID_SCRIPTARRAY)
		{
			asIScriptArray *array = static_cast<asIScriptArray*>( m_ScriptObject.GetScriptObject()->GetAddressOfProperty(propIndex) );
			return array->GetElementCount();
		}
		else
			return 0;
	}

	int ScriptedEntity::GetPropertyType(unsigned int index) const
	{
		int propIndex = getScriptPropIndex(index);

		int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId(propIndex);

		int propType = pt_none;

		// Add the array flag
		if (typeId & asTYPEID_SCRIPTARRAY)
		{
			propType |= pt_array_flag;
			typeId = ScriptManager::getSingleton().GetEnginePtr()->GetObjectTypeById(typeId)->GetSubTypeId();
		}

		// Basic types
		switch (typeId)
		{
		case asTYPEID_BOOL:
			propType |= pt_bool; break;
		case asTYPEID_INT8:
			propType |= pt_int8; break;
		case asTYPEID_INT16:
			propType |= pt_int16; break;
		case asTYPEID_INT32:
			propType |= pt_int32; break;
		case asTYPEID_INT64:
			propType |= pt_int64; break;
		case asTYPEID_UINT8:
			propType |= pt_uint8; break;
		case asTYPEID_UINT16:
			propType |= pt_uint16; break;
		case asTYPEID_UINT32:
			propType |= pt_uint32; break;
		case asTYPEID_UINT64:
			propType |= pt_uint64; break;
		case asTYPEID_FLOAT:
			propType |= pt_float; break;
		case asTYPEID_DOUBLE:
			propType |= pt_double; break;
		}
		// If a basic type was detected, return the property type
		if (propType != pt_none && propType != pt_array_flag)
			return propType;
		// Otherwise, check for a application type
		else if (typeId & asTYPEID_APPOBJECT)
		{
			ScriptManager *man = ScriptManager::getSingletonPtr();
			// Check for entity type (this is seperated from the following types because it is always a pointer type)
			if ((typeId & ~asTYPEID_OBJHANDLE) == s_EntityTypeId)
				propType |= pt_entity;

			else
			{
				if ((typeId & ~asTYPEID_OBJHANDLE) == man->GetStringTypeId())
					propType |= pt_string;
				else if ((typeId & ~asTYPEID_OBJHANDLE) == man->GetVectorTypeId())
					propType |= pt_vector;

				// Add the pointer flag
				if (typeId & asTYPEID_OBJHANDLE)
					propType |= pt_pointer_flag;
			}

			return propType;
		}

		// notice that propType isn't returned here as it could be a loose pt_array_flag
		//  (for an array of an unsupported type) and returning that would be useless
		return pt_none;
	}

	void* ScriptedEntity::GetAddressOfProperty(unsigned int index, unsigned int array_index) const
	{
		int propIndex = getScriptPropIndex(index);
		if (m_ScriptObject.GetScriptObject()->GetPropertyTypeId(propIndex) & asTYPEID_SCRIPTARRAY)
		{
			asIScriptArray *array = static_cast<asIScriptArray*>( m_ScriptObject.GetScriptObject()->GetAddressOfProperty(propIndex) );
			return array->GetElementPointer(array_index);
		}
		else
			return m_ScriptObject.GetScriptObject()->GetAddressOfProperty(propIndex);
	}

	void ScriptedEntity::EnumReferences(asIScriptEngine *engine)
	{
		engine->GCEnumCallback((void*)m_ScriptObject.GetScriptObject());
	}

	void ScriptedEntity::ReleaseAllReferences(asIScriptEngine *engine)
	{
		m_ScriptObject.Release();
		//engine->ReleaseScriptObject((void*)m_ScriptObject.GetScriptObject(), m_ScriptObject.GetTypeId());
	}

	std::string ScriptedEntity::GetType() const
	{
		return m_ScriptObject.GetScriptObject()->GetObjectType()->GetName();
	}

	void ScriptedEntity::Spawn()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void Spawn()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::Update(float split)
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void Update(float)");
		if (f.ok())
		{
			f(split);
		}
	}

	void ScriptedEntity::Draw()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void Draw()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::OnStreamIn()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void OnStreamIn()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::OnStreamOut()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void OnStreamOut()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::SerialiseState(SerialisedData &state, bool local) const
	{
		CL_IODevice_Memory dev;
		//std::ostringstream stateStream(std::ios::binary);

		// Notify the deserialiser that this created in local/non-local mode
		//stateStream << local;
		dev.write_uint8(local ? 1 : 0);

		dev.write_uint32(state.mask);

		unsigned int index = 0;
		for (PropertiesArray::const_iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &propertyDesc = *it;
			if (!local && propertyDesc.localOnly)
				continue; // this property is only serialised to disc

			if (!state.IsIncluded(index++))
				continue; // This component has been excluded from the state by the caller

			const std::string &propName = propertyDesc.name;
			// Make sure the prop index is valid
			if (propertyDesc.scriptPropertyIndex >= m_ScriptObject.GetScriptObject()->GetPropertyCount())
				continue;

			int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId( propertyDesc.scriptPropertyIndex );
			void *prop = m_ScriptObject.GetScriptObject()->GetAddressOfProperty( propertyDesc.scriptPropertyIndex );

			// Check for primative types
			bool isPrimative = true;
			switch (typeId)
			{
			case asTYPEID_BOOL:
				//stateStream << *static_cast<bool*>( prop );
				dev.write_uint8( *static_cast<bool*>(prop) ? 1 : 0 );
				break;
			case asTYPEID_INT32:
				//stateStream << *static_cast<int*>( prop );
				dev.write_int32( *static_cast<int*>(prop) );
				break;
			case asTYPEID_UINT32:
				//stateStream << *static_cast<unsigned int*>( prop );
				dev.write_uint32( *static_cast<unsigned int*>(prop) );
				break;
			case asTYPEID_FLOAT:
				//stateStream << *static_cast<float*>( prop );
				dev.write_float( *static_cast<float*>(prop) );
				break;
			case asTYPEID_DOUBLE:
				//stateStream << *static_cast<double*>( prop );
				dev.write( static_cast<double*>(prop), sizeof(double) );
				break;
			default:
				isPrimative = false;
				break;
			}
			if (!isPrimative) // Check for non-primative types:
			{
				if (typeId & ScriptManager::getSingletonPtr()->GetVectorTypeId())
				{
					//stateStream << *static_cast<Vector2*>( prop );
					Vector2 *value = static_cast<Vector2*>( prop );
					dev.write_float(value->x);
					dev.write_float(value->y);
				}
				else if (typeId & ScriptManager::getSingletonPtr()->GetStringTypeId())
				{
					CScriptString *value = static_cast<CScriptString*>( prop );
					std::string::size_type length = value->buffer.length();

					//// Write the length
					//stateStream << length;
					//// Write the value
					//stateStream.write(value->buffer.c_str(), length);

					dev.write_int32(length); // Write the length
					dev.write(value->buffer.c_str(), length); // Write the value
				}
				// Entity
				else if (typeId & s_EntityTypeId)
				{
					Entity *value = *static_cast<Entity**>(prop);
					//stateStream << value->GetID();
					ObjectID id = value->GetID();
					dev.write((void*)&id, sizeof(ObjectID));
				}
			}
		}

		//state.data = stateStream.str();
		const CL_DataBuffer &data = dev.get_data();
		state.data.assign(data.get_data(), (std::string::size_type)data.get_size());
	}

	inline bool isExcluded(unsigned int mask, unsigned int index)
	{
		// Check the mask for the component bit
		return (mask & (1 << index)) == 0;
	}

#define SKIP_EXCLUDED if (isExcluded(mask, index++)) continue

	size_t ScriptedEntity::DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
	{
		//std::istringstream stateStream(state.data, std::ios::binary);
		CL_DataBuffer buffer(state.data.data(), state.data.size());
		CL_IODevice_Memory dev(buffer);

		bool fileLocal = dev.read_uint8() == 1;
		if (local != fileLocal)
			FSN_EXCEPT(ExCode::InvalidArgument, "ScriptedEntity::DeserialiseState", "The given state was saved in a different local mode to the one requested");

		uint32 fileMask = dev.read_uint32();
		uint32 mask = state.mask & fileMask;
		//if ((state.mask & fileMask) != state.mask)
		//	FSN_EXCEPT(ExCode::InvalidArgument, "ScriptedEntity::DeserialiseState", "The given state does not contain all the requested properties");

		unsigned int index = 0;
		for (PropertiesArray::iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &propertyDesc = *it;
			if (!local && propertyDesc.localOnly)
				continue; // This property is only serialised to disc (local-mode), and local is disabled

			const std::string &propName = propertyDesc.name;
			// Make sure the prop index is valid
			if (propertyDesc.scriptPropertyIndex >= m_ScriptObject.GetScriptObject()->GetPropertyCount())
				continue;

			int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId( propertyDesc.scriptPropertyIndex );
			void *prop = m_ScriptObject.GetScriptObject()->GetAddressOfProperty( propertyDesc.scriptPropertyIndex );

			// Check to see if the property is a primative type
			bool isPrimative = true;
			switch (typeId)
			{
			case asTYPEID_BOOL:
				{
					bool value;
					//stateStream >> value;
					value = dev.read_uint8() == 1;
					SKIP_EXCLUDED;
					new(prop) bool(value);
				}
				break;
			case asTYPEID_INT32:
				{
					int value;
					//stateStream >> value;
					value = dev.read_int32();
					new(prop) int(value);
				}
				break;
			case asTYPEID_UINT32:
				{
					unsigned int value;
					//stateStream >> value;
					value = dev.read_uint32();
					SKIP_EXCLUDED;
					new(prop) unsigned int(value);
				}
				break;
			case asTYPEID_FLOAT:
				{
					float value;
					//stateStream >> value;
					value = dev.read_float();
					SKIP_EXCLUDED;
					new(prop) float(value);
				}
				break;
			case asTYPEID_DOUBLE:
				{
					double value;
					//stateStream >> value;
					dev.read(&value, sizeof(double));
					SKIP_EXCLUDED;
					new(prop) double(value);
				}
				break;
			}
			// If the property isn't primative, check for other known types
			if (!isPrimative)
			{
				if (typeId & ScriptManager::getSingletonPtr()->GetVectorTypeId())
				{
					Vector2 value;
					//stateStream >> value;
					value.x = dev.read_float();
					value.y = dev.read_float();
					SKIP_EXCLUDED;
					new(prop) Vector2(value);
				}
				else if (typeId & ScriptManager::getSingletonPtr()->GetStringTypeId())
				{
					std::string value;
					std::string::size_type length;
					// Read the length
					//stateStream >> length;
					length = dev.read_uint32();
					value.resize(length);
					// Read the value
					//stateStream.read(&value[0], length);
					dev.read(&value[0], length);

					SKIP_EXCLUDED;
					new(prop) CScriptString(value);
				}
				else if (typeId & s_EntityTypeId)
				{
					ObjectID value;
					//stateStream >> value;
					dev.read(&value, sizeof(ObjectID));

					SKIP_EXCLUDED;

					EntityPtr entity = entity_deserialiser.GetEntity(value);
					if (entity.get() != NULL)
						*((Entity**)prop) = entity.get();
				}
			}
		}

		//return stateStream.tellg();
		return dev.get_position();
	}

	asIScriptObject* ScriptedEntity::GetScriptObject(Entity *entity)
	{
		ScriptedEntity *wrapper = dynamic_cast<ScriptedEntity*>( entity );
		if (wrapper != NULL && wrapper->m_ScriptObject.IsValid())
		{
			asIScriptObject *script_obj = wrapper->m_ScriptObject.GetScriptObject();
			script_obj->AddRef();
			//entity->release();
			return script_obj;
		}
		else
			return NULL;
	}

	ScriptedEntity* ScriptedEntity::GetAppObject(asIScriptObject *script_obj)
	{
		ScriptedEntity *appObject = NULL;
		ScriptUtils::Calling::Caller f(script_obj, "Entity@ _getAppObject()");
		if (f.ok())
		{
			Entity *ptr = *static_cast<Entity**>( f() );
			appObject = dynamic_cast<ScriptedEntity*>( ptr );
		}

		return appObject;
	}

	
	Entity* Fixture_GetEntity(b2Fixture &obj)
	{
		return (Entity*)obj.GetBody()->GetUserData();
	}

	void PhysicalEntity_ApplyForce(const Vector2 &force, PhysicalEntity *obj)
	{
		b2Body *body = obj->GetBody();
		body->ApplyForce(
			b2Vec2(force.x * s_SimUnitsPerGameUnit, force.y * s_SimUnitsPerGameUnit),
			body->GetWorldCenter());
	}

	Vector2* PhysicalEntity_GetWorldVector(const Vector2 &vector, PhysicalEntity *obj)
	{
		b2Body *body = obj->GetBody();
		b2Vec2 v = body->GetWorldVector(b2Vec2(vector.x, vector.y));
		return new Vector2(v.x, v.y);
	}

	Vector2* PhysicalEntity_GetWorldPoint(const Vector2 &point, PhysicalEntity *obj)
	{
		b2Body *body = obj->GetBody();
		b2Vec2 p = body->GetWorldPoint(b2Vec2(point.x, point.y));
		return new Vector2(p.x, p.y);
	}

	void ScriptedEntity::Register(asIScriptEngine* engine)
	{
		s_EntityTypeId = engine->GetTypeIdByDecl("Entity");

		int r;
		r = engine->RegisterObjectMethod("Entity",
			"void applyForce(const Vector &in, const Vector &in)",
			asMETHODPR(PhysicalEntity, ApplyForce, (const Vector2&, const Vector2&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Entity",
			"void applyForce(const Vector &in)",
			asFUNCTIONPR(PhysicalEntity_ApplyForce, (const Vector2&, PhysicalEntity *obj), void), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Entity",
			"void applyTorque(float)",
			asMETHODPR(PhysicalEntity, ApplyTorque, (float), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);

		r = engine->RegisterObjectMethod("Entity",
			"Vector@ getWorldVector(const Vector &in)",
			asFUNCTIONPR(PhysicalEntity_GetWorldVector, (const Vector2&, PhysicalEntity *obj), Vector2*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Entity",
			"Vector@ getWorldPoint(const Vector &in)",
			asFUNCTIONPR(PhysicalEntity_GetWorldPoint, (const Vector2&, PhysicalEntity *obj), Vector2*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);

		// Fixture method
		//r = engine->RegisterObjectMethod("Fixture", "Entity@ getEntity() const", asFUNCTIONPR(Fixture_GetEntity, (b2Fixture&), Entity*), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

	void ScriptedEntity::SetScriptEntityTypeId(int id)
	{
		s_ScriptEntityTypeId = id;
	}

	inline int ScriptedEntity::getScriptPropIndex(unsigned int entity_prop_index) const
	{
		FSN_ASSERT(entity_prop_index < m_SyncedProperties.size());
		return m_SyncedProperties[entity_prop_index].scriptPropertyIndex;
	}

}
