/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionArchetypeFactory
#define H_FusionArchetypeFactory

#include "FusionPrerequisites.h"

#include "FusionVectorTypes.h"
#include "FusionTypes.h"

#include "FusionComponentFactory.h"

#include <string>
#include <map>
#include <memory>

#include <boost/signals2/signal.hpp>

namespace FusionEngine
{

	class Archetype;

	//! Used to define archetypes
	class ArchetypeFactory
	{
	public:
		ArchetypeFactory();
		virtual ~ArchetypeFactory();

		void SetEditable(bool value) { m_Editable = value; }

		//! Returns an existing archetype
		EntityPtr GetArchetype(const std::string& type_id) const;

		//! Creates a new archetype
		EntityPtr CreateArchetype(ComponentFactory* factory, const std::string& type_id, const std::string& transform_type);

		//! Makes the given entity an instance of the given archetype
		virtual EntityPtr MakeInstance(ComponentFactory* factory, const std::string& type_id, const Vector2& pos, float angle);

		//! Defines the given archetype using the given entity
		void DefineArchetypeFromEntity(ComponentFactory* factory, const std::string& type_id, const EntityPtr& entity);

	private:
		struct ArchetypeData
		{
		public:
			EntityPtr Archetype;
			// TODO: rename the 'Archetype' class 'Archetypes::PropertyDefinition'
			std::shared_ptr<FusionEngine::Archetype> Definition;
			boost::signals2::signal<void (RakNet::BitStream&)> SignalChange;

			ArchetypeData() {}
			explicit ArchetypeData(EntityPtr entity) : Archetype(entity) {}
			ArchetypeData(ArchetypeData&& other) : Archetype(std::move(other.Archetype)) {}
		private:
			ArchetypeData(const ArchetypeData&) {}
			ArchetypeData& operator=(const ArchetypeData&) {}
		};
		std::map<std::string, ArchetypeData> m_Archetypes;

		bool m_Editable;
	};

	class EditorArchetypeFactory : public ArchetypeFactory
	{
	public:
		EntityPtr MakeInstance(ComponentFactory* factory, const std::string& type_id, const Vector2& pos, float angle);
	};

}

#endif
