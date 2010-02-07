/*
  Copyright (c) 2009-2010 Fusion Project Team

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
*/

#ifndef Header_FusionEngine_ScriptedEntity
#define Header_FusionEngine_ScriptedEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionPhysicalEntity.h"

#include "FusionScriptReference.h"


namespace FusionEngine
{

	static const type_info &ToCppType(int as_type_id, ScriptingEngine *engine = NULL);

	class ResourceDescription
	{
	public:
		ResourceDescription();
		~ResourceDescription();

		void SetType(const std::string &type);
		const std::string &GetType() const;

		void SetPropertyName(const std::string &name);
		virtual void SetResourceName(const std::string &name);
		void SetPriority(int priority);
		void ParseTags(const std::string &tags);

		void SetPropertyIndex(int index);

		const std::string &GetPropertyName() const;
		const std::string &GetResourceName() const;
		int GetPriority() const;
		const TagStringSet &GetTags() const;
		bool HasTag(const std::string &tag) const;

		int GetPropertyIndex() const;

	protected:
		std::string m_ScriptPropertyName;
		std::string m_Type; // Image, Sound, SoundStream
		std::string m_ResourceName;
		int m_Priority;
		TagStringSet m_Tags;

		int m_ScriptPropertyIndex;
	};
	
	typedef std::tr1::unordered_map<std::string, ResourceDescription> ResourcesMap;

	/*!
	 * This class acts as a wrapper for the script objects which define
	 * the actual logic for in-game objects.
	 */
	class ScriptedEntity : public PhysicalEntity
	{
	public:
		//! Stores information useful for serializing & syncing properties
		struct Property
		{
			// Data loaded from XML
			std::string name;
			std::string type;
			bool localOnly;
			bool arbitrated;

			// Index of the property in the script-object
			int scriptPropertyIndex;

			Property() : localOnly(false), arbitrated(false), scriptPropertyIndex(-1)
			{}
		};

		typedef std::vector<Property> PropertiesArray;

		//typedef std::map<std::string, ResourceDescriptionPtr> ResourcesMap;
		typedef std::map<std::wstring, std::string> StreamedResourceMap;

	public:
		ScriptedEntity();
		ScriptedEntity(ScriptObject script_self, const std::string &name);
		virtual ~ScriptedEntity();

		void SetPath(const std::string &path);

		void SetSyncProperties(const PropertiesArray &properties);
		//void SetStreamedResources(const StreamedResourceMap &resources);
		//void AddStreamedResource(const std::string &type, const std::wstring &path);

		virtual unsigned int GetPropertyCount() const;
		virtual std::string GetPropertyName(unsigned int index) const;
		virtual boost::any GetPropertyValue(unsigned int index) const;
		virtual void SetPropertyValue(unsigned int index, const boost::any &value);

		//virtual EntityPtr GetPropertyEntity(unsigned int index) const;
		//virtual void SetPropertyEntity(unsigned int index, const EntityPtr &value);

		//! Returns the size of the given property if it is an array
		virtual unsigned int GetPropertyArraySize(unsigned int index) const;
		virtual int GetPropertyType(unsigned int index) const;
		virtual void* GetAddressOfProperty(unsigned int index, unsigned int array_index) const;

		virtual void EnumReferences(asIScriptEngine *engine);
		virtual void ReleaseAllReferences(asIScriptEngine *engine);

		virtual std::string GetType() const;

		//virtual const Vector2 &GetPosition();
		//virtual float GetAngle();

		virtual void Spawn();
		virtual void Update(float split);
		virtual void Draw();

		//! Called on stream in
		virtual void OnStreamIn();
		//! Called on stream out
		virtual void OnStreamOut();

		//! Writes property data
		virtual void SerialiseState(SerialisedData &state, bool local) const;
		//! Reads property data
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser);

		//! Returns the wrapped script object
		static asIScriptObject* GetScriptObject(Entity *entity);
		//! Returns the wrapper for the given ScriptEntity-typed script object
		static ScriptedEntity* GetAppObject(asIScriptObject *script);

		//! Registers the Entity type
		static void Register(asIScriptEngine *engine);
		//! EntityFactory calls this when the main module is rebuilt
		static void SetScriptEntityTypeId(int id);

		static int s_EntityTypeId;
		static int s_ScriptEntityTypeId;
	protected:
		// The (scripted) entity implementation
		ScriptObject m_ScriptObject;

		// The folder where the the entity definition resides - used when calls to ScriptEntity::MakePathAbsolute are made in the entity script
		std::string m_Path;

		PropertiesArray m_SyncedProperties;
		StreamedResourceMap m_Streamed;
		
		Vector2 m_DefaultPosition;
		float m_DefaultAngle;

		//! Safely gets the script object property index of the given entity property index
		inline int getScriptPropIndex(unsigned int entity_prop_index) const;
	};

	static void RegisterEntityUnwrap(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterGlobalFunction("IEntity@ unwrap(Entity@)",
			asFUNCTION(ScriptedEntity::GetScriptObject),
			asCALL_CDECL); FSN_ASSERT(r >= 0);
	}

}

#endif