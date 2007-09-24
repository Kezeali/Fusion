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


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_EntityFactory
#define Header_FusionEngine_EntityFactory

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

/// Inherited
#include "FusionSingleton.h"

namespace FusionEngine
{

	//! Creates entities
	/*!
	 * Creates entities by name based on the definitions in the
	 * currently loaded entity files.
	 */
	class EntityFactory : public Singleton<EntityFactory>
	{
	public:
		//! Maps tags to entity definitions
		typedef std::map<std::string, EntityBuilder*> EntityBuilderList;

		//! Creates a specific type of entity
		class EntityBuilder
		{
		public:
			//! Constructor
			EntityBuilder();
			//! Constructor
			EntityBuilder(const std::string& name);

		public:
			void AddResource(const std::string& tag, ResourcePointer* resource);
			void RemoveResource(const std::string& tag);

			Entity* BuildEntity();
		};

	public:
		//! Constructor
		EntityFactory();

		//! Destructor
		~EntityFactory();

	public:
		Entity* BuildEntity(const std::string& name);

	protected:
		//vars

	protected:
		//methods

	};

}

#endif
