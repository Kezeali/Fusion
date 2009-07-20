
#include "FusionCommon.h"

// Class
#include "FusionScriptedEntity.h"

// Fusion

#include "scriptstring.h"


namespace FusionEngine
{

	ResourceParam::ResourceParam(const std::string &type)
		: m_Type(type)
	{}

	ResourceParam::~ResourceParam()
	{
	}

	const std::string &ResourceParam::GetType() const
	{
		return m_Type;
	}

	ImageResourceParam::ImageResourceParam(const ResourcePointer<CL_Sprite> &resource)
		: ResourceParam("Image")
	{
		m_Renderable.reset(new Renderable(resource));
	}

	void ImageResourceParam::SetPosition(const Vector2 &position)
	{
		m_Renderable->SetPosition(position);
	}

	ScriptedEntity::ScriptedEntity()
		: m_DefaultPosition(0.f, 0.f),
		m_DefaultAngle(0.f)
	{}

	ScriptedEntity::ScriptedEntity(ScriptObject self, const std::string &name)
		: Entity(name),
		m_ScriptObject(self),
		m_DefaultPosition(0.f, 0.f),
		m_DefaultAngle(0.f)
	{}

	void ScriptedEntity::SetSyncProperties(const ScriptedEntity::PropertiesMap &properties)
	{
		m_SyncedProperties = properties;
	}

	void ScriptedEntity::SetStreamedResources(const ScriptedEntity::ResourcesMap &resources)
	{
		m_Streamed = resources;
	}

	std::string ScriptedEntity::GetType() const
	{
		return m_ScriptObject.GetScriptObject()->GetObjectType()->GetName();
	}

	const Vector2 &ScriptedEntity::GetPosition()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("const Vector2 &GetPosition()");
		if (f.ok())
		{
			void *r = f();
			if (r == NULL)
				return **static_cast<const Vector2**>( r );
		}

		return m_DefaultPosition;
	}

	float ScriptedEntity::GetAngle()
	{
		ScriptUtils::Calling::Caller f = m_ScriptObject.GetCaller("void Update(float)");
		if (f.ok())
		{
			void *r = f();
			if (r == NULL)
				return *static_cast<float*>( r );
		}

		return m_DefaultAngle;
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

	void ScriptedEntity::DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser)
	{
		std::istringstream stateStream(state.data, std::ios::binary);

		// Check that is the expected type of data
		bool isLocalData;
		stateStream >> isLocalData;
		if (isLocalData != local)
			return;

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
	}

}
