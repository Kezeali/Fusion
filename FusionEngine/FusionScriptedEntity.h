/*
  Copyright (c) 2009 Fusion Project Team

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

		typedef std::map<std::string, Property> PropertiesMap;
		//typedef std::set<std::string> PropertiesSet;

		//typedef std::map<std::string, ResourceDescriptionPtr> ResourcesMap;
		typedef std::map<std::wstring, std::string> StreamedResourceMap;

	public:
		ScriptedEntity();
		ScriptedEntity(ScriptObject script_self, const std::string &name);
		virtual ~ScriptedEntity();

		void SetPath(const std::string &path);

		void SetSyncProperties(const PropertiesMap &properties);
		//void SetStreamedResources(const StreamedResourceMap &resources);
		//void AddStreamedResource(const std::string &type, const std::wstring &path);

		virtual void EnumReferences(asIScriptEngine *engine);
		virtual void ReleaseAllReferences(asIScriptEngine *engine);

		virtual std::string GetType() const;

		//virtual const Vector2 &GetPosition();
		//virtual float GetAngle();

		virtual void Spawn();
		virtual void Update(float split);
		virtual void Draw();

		virtual void OnStreamIn();
		virtual void OnStreamOut();

		virtual void SerialiseState(SerialisedData &state, bool local) const;
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser);

		static asIScriptObject* GetScriptObject(Entity *entity);
		static ScriptedEntity* GetAppObject(asIScriptObject *script);

		static void Register(asIScriptEngine *engine);

	protected:
		// The (scripted) entity implementation
		ScriptObject m_ScriptObject;

		// The folder where the the entity definition resides - used when calls to ScriptEntity::MakePathAbsolute are made in the entity script
		std::string m_Path;

		PropertiesMap m_SyncedProperties;
		StreamedResourceMap m_Streamed;

		int m_EntityTypeId;
		int m_ScriptEntityTypeId;

		Vector2 m_DefaultPosition;
		float m_DefaultAngle;
	};

	static void RegisterEntityUnwrap(asIScriptEngine *engine)
	{
		int r;
		r = engine->RegisterGlobalFunction("IEntity@ unwrap(Entity@)",
			asFUNCTION(ScriptedEntity::GetScriptObject),
			asCALL_CDECL);
	}

}

#endif