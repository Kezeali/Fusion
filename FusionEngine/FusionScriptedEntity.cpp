
#include "FusionCommon.h"

// Class
#include "FusionScriptedEntity.h"

// Fusion
#include "FusionResourceManager.h"

#include "scriptstring.h"


namespace FusionEngine
{

	const type_info &ToCppType(int type_id, ScriptingEngine *engine)
	{
		if (engine == NULL)
			engine = ScriptingEngine::getSingletonPtr();

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

	// REVISE--Type should be the actual type of the variable held by the script
	//  engine - i.e. the type that the void* should be casted to.
	template <typename T>
	void getPropValueOfType(boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		cpp_obj = *(T*)obj->GetAddressOfProperty(property_index);
	}
	template <typename T>
	void setPropValueOfType(const boost::any &cpp_obj, asIScriptObject *obj, asUINT property_index)
	{
		*(T*)obj->GetAddressOfProperty(property_index) = boost::any_cast<T>( cpp_obj );
	}

	void ScriptedEntity::accessScriptPropValue(boost::any &cpp_obj, asUINT property_index, bool get) const
	{
		asIScriptObject *obj = m_ScriptObject.GetScriptObject();
		int type_id = obj->GetPropertyTypeId(property_index);

		if (type_id == asTYPEID_BOOL)
			if (get) getPropValueOfType<bool>(cpp_obj, obj, property_index);
			else setPropValueOfType<bool>(cpp_obj, obj, property_index);
		// Integer types
		else if (type_id == asTYPEID_INT8)
			if (get) getPropValueOfType<int8_t>(cpp_obj, obj, property_index);
			else setPropValueOfType<bool>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_INT16)
			if (get) getPropValueOfType<int16_t>(cpp_obj, obj, property_index);
			else setPropValueOfType<int16_t>(cpp_obj, obj, property_index);
		else if (type_id == asTYPEID_INT32)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		else if (type_id == asTYPEID_INT64)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		// ... unsigned
		else if (type_id == asTYPEID_UINT8)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		else if (type_id == asTYPEID_UINT16)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		else if (type_id == asTYPEID_UINT32)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		else if (type_id == asTYPEID_UINT64)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		// Floating point types
		else if (type_id == asTYPEID_FLOAT)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);
		else if (type_id == asTYPEID_DOUBLE)
			cpp_obj = *(bool*)obj->GetAddressOfProperty(property_index);

		// Pointers / handles
		else if (type_id & asTYPEID_APPOBJECT)
		{
			ScriptingEngine *engine = NULL; // TODO: engine param?
			if (engine == NULL)
				engine = ScriptingEngine::getSingletonPtr();
			if (engine == NULL)
				FSN_EXCEPT(ExCode::InvalidArgument, "GetPropertyValue", "Can't get application-defined type without a valid ScriptingManager");

			if (type_id == m_ScriptEntityTypeId)
				cpp_obj = *(ScriptedEntity**)obj->GetAddressOfProperty(property_index);
			else if (type_id == engine->GetStringTypeId())
				cpp_obj = *(CScriptString**)obj->GetAddressOfProperty(property_index);
			else if (type_id == engine->GetVectorTypeId())
				cpp_obj = *(Vector2**)obj->GetAddressOfProperty(property_index);
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

	//void ScriptedEntity::SetStreamedResources(const ScriptedEntity::StreamedResourceMap &resources)
	//{
	//	m_Streamed = resources;
	//}

	//void ScriptedEntity::AddStreamedResource(const std::string &type, const std::wstring &path)
	//{
	//	m_Streamed[path] = type;
	//}

	unsigned int ScriptedEntity::GetPropertiesCount() const
	{
		return m_SyncedProperties.size();
	}

	std::string ScriptedEntity::GetPropertyName(unsigned int index) const
	{
		return m_ScriptObject.GetScriptObject()->GetPropertyName(index);
	}

	boost::any ScriptedEntity::GetPropertyValue(unsigned int index) const
	{
		boost::any value;
		accessScriptPropValue(value, index, true);
		return value;
	}

	void ScriptedEntity::SetPropertyValue(unsigned int index, const boost::any &value)
	{
		void *ptr = m_ScriptObject.GetScriptObject()->GetAddressOfProperty(index);
		
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

	//const Vector2 &ScriptedEntity::GetPosition()
	//{
	//	ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("const Vector@ GetPosition()");
	//	if (f.ok())
	//	{
	//		void *r = f();
	//		if (r != NULL)
	//			return **static_cast<const Vector2**>( r );
	//	}

	//	return m_DefaultPosition;
	//}

	//float ScriptedEntity::GetAngle()
	//{
	//	ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("float GetAngle()");
	//	if (f.ok())
	//	{
	//		void *r = f();
	//		if (r != NULL)
	//			return *static_cast<float*>( r );
	//	}

	//	return m_DefaultAngle;
	//}

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
		//add m_ResourceManager member
		//ResourceManager *res = ResourceManager::getSingletonPtr();
		//if (res != NULL)
		//{
		//	// Stream in resources
		//	for (StreamedResourceMap::iterator it = m_Streamed.begin(), end = m_Streamed.end(); it != end; ++it)
		//	{
		//		res->PreloadResource_Background(it->second, it->first, 1);
		//	}
		//}

		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void OnStreamIn()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::OnStreamOut()
	{
		//ResourceManager *res = ResourceManager::getSingletonPtr();
		//if (res != NULL)
		//{
		//	// Stream in resources
		//	for (StreamedResourceMap::iterator it = m_Streamed.begin(), end = m_Streamed.end(); it != end; ++it)
		//	{
		//		res->UnloadResource_Background(it->first);
		//	}
		//}

		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void OnStreamOut()");
		if (f.ok())
		{
			f();
		}
	}

	void ScriptedEntity::SerialiseState(SerialisedData &state, bool local) const
	{
		std::ostringstream stateStream(std::ios::binary);

		// Notify the deserialiser that this created in local/non-local mode
		stateStream << local;

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
				stateStream << *static_cast<bool*>( prop );
				break;
			case asTYPEID_INT32:
				stateStream << *static_cast<int*>( prop );
				break;
			case asTYPEID_UINT32:
				stateStream << *static_cast<unsigned int*>( prop );
				break;
			case asTYPEID_FLOAT:
				stateStream << *static_cast<float*>( prop );
				break;
			case asTYPEID_DOUBLE:
				stateStream << *static_cast<double*>( prop );
				break;
			default:
				isPrimative = false;
				break;
			}
			if (!isPrimative) // Check for non-primative types:
			{
				if (typeId == ScriptingEngine::getSingletonPtr()->GetVectorTypeId())
				{
					stateStream << *static_cast<Vector2*>( prop );
				}
				if (typeId == ScriptingEngine::getSingletonPtr()->GetStringTypeId())
				{
					CScriptString *value = static_cast<CScriptString*>( prop );
					std::string::size_type length = value->buffer.length();

					// Write the length
					stateStream << length;
					// Write the value
					stateStream.write(value->buffer.c_str(), length);
				}
			}
		}

		state.data = stateStream.str();
	}

	size_t ScriptedEntity::DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
	{
		// Deserialise physics data
		//size_t physicsDataLength = PhysicalEntity::DeserialiseState(state, local, entity_deserialiser);

		std::istringstream stateStream(state.data, std::ios::binary);
		// Seek to after the data that has already been deserialised by the base class
		//stateStream.seekg(physicsDataLength);

		unsigned int index = 0;
		for (PropertiesArray::iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &propertyDesc = *it;
			if (!local && propertyDesc.localOnly)
				continue; // This property is only serialised to disc (local-mode), and local is disabled

			if (!state.IsIncluded(index++))
				continue; // This component was excluded during serialisation

			const std::string &propName = propertyDesc.name;
			// Make sure the prop index is valid
			if (propertyDesc.scriptPropertyIndex >= m_ScriptObject.GetScriptObject()->GetPropertyCount())
				continue;

			int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId( propertyDesc.scriptPropertyIndex );
			void *prop = m_ScriptObject.GetScriptObject()->GetAddressOfProperty( propertyDesc.scriptPropertyIndex );

			// Check to see if the property if a primative type
			bool isPrimative = true;
			switch (typeId)
			{
			case asTYPEID_BOOL:
				{
					bool value;
					stateStream >> value;
					new(prop) bool(value);
				}
				break;
			case asTYPEID_INT32:
				{
					int value;
					stateStream >> value;
					new(prop) int(value);
				}
				break;
			case asTYPEID_UINT32:
				{
					unsigned int value;
					stateStream >> value;
					new(prop) unsigned int(value);
				}
				break;
			case asTYPEID_FLOAT:
				{
					float value;
					stateStream >> value;
					new(prop) float(value);
				}
				break;
			case asTYPEID_DOUBLE:
				{
					double value;
					stateStream >> value;
					new(prop) double(value);
				}
				break;
			}
			// If the property isn't primative, check for other known types
			if (!isPrimative)
			{
				if (typeId == ScriptingEngine::getSingletonPtr()->GetVectorTypeId())
				{
					Vector2 value;
					stateStream >> value;
					new(prop) Vector2(value);
				}
				else if (typeId == ScriptingEngine::getSingletonPtr()->GetStringTypeId())
				{
					std::string value;
					std::string::size_type length;
					// Read the length
					stateStream >> length;
					value.resize(length);
					// Read the value
					stateStream.read(&value[0], length);

					new(prop) CScriptString(value);
				}
				else if (typeId == m_EntityTypeId)
				{
					ObjectID value;
					stateStream >> value;

					EntityPtr entity = entity_deserialiser.GetEntity(value);
					if (entity.get() != NULL)
						*((Entity**)prop) = entity.get();
				}
				else if (typeId == m_ScriptEntityTypeId)
				{
					ObjectID value;
					stateStream >> value;

					EntityPtr entity = entity_deserialiser.GetEntity(value);
					ScriptedEntity *scriptedEntity = dynamic_cast<ScriptedEntity*>( entity.get() );
					if (scriptedEntity != NULL)
						*((asIScriptObject**)prop) = scriptedEntity->m_ScriptObject.GetScriptObject();
				}
			}
		}

		return stateStream.tellg();
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

}
