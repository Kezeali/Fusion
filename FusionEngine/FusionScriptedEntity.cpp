
#include "FusionCommon.h"

// Class
#include "FusionScriptedEntity.h"

// Fusion

#include "scriptstring.h"


namespace FusionEngine
{

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

	std::string ScriptedEntity::SerializeState(bool local) const
	{
		std::stringstream stateStream;

		// Notify the deserializer whether this created in local mode
		stateStream << local;

		for (PropertiesMap::const_iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const Property &prop = it->second;
			if (!local && prop.localOnly)
				continue; // this property is only serialized to disc

			const std::string &propName = it->first;
			for (int i = 0; i < m_ScriptObject.GetScriptObject()->GetPropertyCount(); i++)
			{
				if (std::strcmp( m_ScriptObject.GetScriptObject()->GetPropertyName(i), propName.c_str() ) == 0)
				{
					int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId(i);
					void *prop = m_ScriptObject.GetScriptObject()->GetPropertyPointer(i);

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
					if (!isPrimative)
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
			}
		}

		return stateStream.str();
	}

	void ScriptedEntity::DeserializeState(const std::string& state, bool local)
	{
		std::stringstream stateStream(state, std::ios::binary | std::ios::in | std::ios::out);

		// Check that is the expected type of data
		bool isLocalData;
		stateStream >> isLocalData;
		if (isLocalData == local)

		for (PropertiesMap::iterator it = m_SyncedProperties.begin(), end = m_SyncedProperties.end(); it != end; ++it)
		{
			const std::string &propName = it->first;
			for (int i = 0; i < m_ScriptObject.GetScriptObject()->GetPropertyCount(); i++)
			{
				if (std::strcmp( m_ScriptObject.GetScriptObject()->GetPropertyName(i), propName.c_str() ) == 0)
				{
					int typeId = m_ScriptObject.GetScriptObject()->GetPropertyTypeId(i);
					void *prop = m_ScriptObject.GetScriptObject()->GetPropertyPointer(i);

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
						if (typeId == ScriptingEngine::getSingletonPtr()->GetStringTypeId())
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
					}

				}
			}
		}

	}

}
