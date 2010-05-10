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


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_EntityDeserialiser
#define Header_FusionEngine_EntityDeserialiser

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	struct IDTranslator
	{
		IDTranslator()
			: baseID(0)
		{}

		// From EntityManager startId should be m_NextId-1
		IDTranslator(ObjectID startId)
			: baseID(startId)
		{}

		ObjectID operator() (const ObjectID &from) const
		{
			return from + baseID;
		}

		ObjectID baseID;
	};

	class IEntityRepo
	{
	public:
		virtual ~IEntityRepo() {}
		virtual EntityPtr GetEntity(ObjectID id) const = 0;
	};

	class EntityDeserialiser
	{
	public:
		EntityDeserialiser(const IEntityRepo *manager, const IDTranslator &id_translator = IDTranslator());

		EntityPtr GetEntity(ObjectID serialised_id) const;

	private:
		const IEntityRepo *m_Impl;
		IDTranslator m_Translator;
	};

}

#endif
