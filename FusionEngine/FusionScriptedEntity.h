/*
  Copyright (c) 2007 Fusion Project Team

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

#include "FusionEntity.h"
#include "FusionScriptReference.h"


namespace FusionEngine
{

	class ResourceParam
	{
	public:
		ResourceParam(const std::string &type);
		virtual ~ResourceParam();

		const std::string &GetType() const;

	protected:
		std::string m_ScriptPropertyName;
		std::string m_Type; // Image, Sound, StreamedSound

		int m_ScriptPropertyIndex;
	};
	typedef std::tr1::shared_ptr<ResourceParam> ResourceParamPtr;

	class ImageResourceParam : public ResourceParam
	{
	public:
		ImageResourceParam(const ResourcePointer<CL_Sprite> &resource);

		void SetPosition(const Vector2 &position);

	protected:
		RenderablePtr m_Renderable;
	};


	/*!
	 * This class acts as a wrapper for the script objects which define
	 * the actual logic for in-game objects, while storing and providing
	 * access to instances of hard-coded classes for said script objects.
	 *
	 * Hopefully, this will help script objects to be created and used
	 * almost as easily as hard-coded objects.
	 */
	class ScriptedEntity : public Entity
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
		};

		typedef std::map<std::string, Property> PropertiesMap;
		//typedef std::set<std::string> PropertiesSet;

		typedef std::map<std::string, ResourceParamPtr> ResourcesMap;

	public:
		ScriptedEntity();
		ScriptedEntity(ScriptObject script_self, const std::string &name);

		void SetSyncProperties(const PropertiesMap &properties);
		void SetStreamedResources(const ResourcesMap &resources);

		virtual std::string GetType() const;

		virtual const Vector2 &GetPosition();
		virtual float GetAngle();

		virtual void Spawn();
		virtual void Update(float split);
		virtual void Draw();

		virtual void OnStreamIn();
		virtual void OnStreamOut();

		virtual void SerialiseState(SerialisedData &state, bool local) const;
		virtual void DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser);

	protected:
		// The actual entity logic (for which this C++ class is simply a wrapper)
		ScriptObject m_ScriptObject;

		// The folder where the the entity definition resides - used when calls to ScriptEntity::MakePathAbsolute are made in the entity script
		std::string m_Path;

		PropertiesMap m_SyncedProperties;
		ResourcesMap m_Streamed;

		int m_EntityTypeId;
		int m_ScriptEntityTypeId;

		Vector2 m_DefaultPosition;
		float m_DefaultAngle;
	};

}

#endif