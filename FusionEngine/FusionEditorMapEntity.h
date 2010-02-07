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

#ifndef Header_FusionEngine_EditorMapEntity
#define Header_FusionEngine_EditorMapEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionGameMapLoader.h"
#include <EMP/Core/DataSource.h>
#include <Rocket/Controls/DataFormatter.h>

#include "FusionPhysicsFixture.h"

#include <boost/lexical_cast.hpp>


namespace FusionEngine
{

	//! EditorMapEntity adds additional data to GameMapLoader#GameMapEntity which is useful for the Editor
	class EditorMapEntity : public GameMapLoader::GameMapEntity, public EMP::Core::DataSource
	{
	public:
		FixturePtr fixture;

		//! Ctor
		EditorMapEntity();
		//! Dtor
		virtual ~EditorMapEntity();

		//! Creates a clickable physics fixture
		void CreateEditorFixture();

		void RefreshProperty(unsigned int index)
		{
			NotifyRowChange("properties", index, 1);
		}

		template <typename T>
		void SetPropertyValue(unsigned int index, unsigned int array_index, T value)
		{
			T *prop = static_cast<T*>(entity->GetAddressOfProperty(index, array_index));
			*prop = value;
			NotifyRowChange("properties", index, 1);
		}

		//! DataSource impl.
		void GetRow(EMP::Core::StringList& row, const EMP::Core::String& table, int row_index, const EMP::Core::StringList& columns);
		//! DataSource impl.
		int GetNumRows(const EMP::Core::String& table);

	private:
		struct VectorTypeInfo
		{
			VectorTypeInfo(size_t _size, const char **_names)
				: size(_size),
				names(_names)
			{
			}
			~VectorTypeInfo()
			{
				//for (size_t i = 0; i < size; ++i)
				//	free(names[i]);
			}
			size_t size;
			const char** names;
		};
		typedef std::map<int, VectorTypeInfo> VectorTypeInfoMap;
		VectorTypeInfoMap m_VectorTypes;

	};
	//! EditorMapEntity ptr
	typedef boost::intrusive_ptr<EditorMapEntity> EditorMapEntityPtr;

	//! Allows a physics fixture to hold a pointer to an EditorMapEntity
	struct MapEntityFixtureUserData : public IFixtureUserData
	{
		GameMapLoader::GameMapEntity *map_entity;

		MapEntityFixtureUserData(GameMapLoader::GameMapEntity *_map_entity)
			: map_entity(_map_entity)
		{
		}
	};

}

#endif
