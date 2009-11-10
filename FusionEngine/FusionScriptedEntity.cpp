
#include "FusionCommon.h"

// Class
#include "FusionScriptedEntity.h"

// Fusion
#include "FusionResourceManager.h"

#include "scriptstring.h"


namespace FusionEngine
{

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

	int ResourceDescription::GetPropertyIndex() const
	{
		return m_ScriptPropertyIndex;
	}

	/*ImageResourceParam::ImageResourceParam(const ResourcePointer<CL_Sprite> &resource)
		: ResourceParam("Image")
	{
		m_Renderable.reset(new Renderable(resource));
	}

	void ImageResourceParam::SetPosition(const Vector2 &position)
	{
		m_Renderable->SetPosition(position);
	}*/

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

	void ScriptedEntity::SetSyncProperties(const ScriptedEntity::PropertiesMap &properties)
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
		for (PropertiesMap::const_iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &propertyDesc = it->second;
			if (!local && propertyDesc.localOnly)
				continue; // this property is only serialised to disc

			if (!state.IsIncluded(index++))
				continue; // This component has been excluded from the state by the caller

			const std::string &propName = it->first;
			// Make sure the prop index is valid
			if (propertyDesc.scriptPropertyIndex >= m_ScriptObject.GetScriptObject()->GetPropertyCount())
				continue;

			int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId( propertyDesc.scriptPropertyIndex );
			void *prop = m_ScriptObject.GetScriptObject()->GetPropertyPointer( propertyDesc.scriptPropertyIndex );

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
		for (PropertiesMap::iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &propertyDesc = it->second;
			if (!local && propertyDesc.localOnly)
				continue; // This property is only serialised to disc (local-mode), and local is disabled

			if (!state.IsIncluded(index++))
				continue; // This component was excluded during serialisation

			const std::string &propName = it->first;
			// Make sure the prop index is valid
			if (propertyDesc.scriptPropertyIndex >= m_ScriptObject.GetScriptObject()->GetPropertyCount())
				continue;

			int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId( propertyDesc.scriptPropertyIndex );
			void *prop = m_ScriptObject.GetScriptObject()->GetPropertyPointer( propertyDesc.scriptPropertyIndex );

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
