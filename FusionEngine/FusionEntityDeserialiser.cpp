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


	File Author(s):

		Elliot Hayward

*/

#include "Common.h"

#include "FusionEntityDeserialiser.h"
#include "FusionEntityManager.h"


namespace FusionEngine
{

	EntityDeserialiser::EntityDeserialiser(EntityManager *manager, const IDTranslator &id_translator)
		: m_Manager(manager),
		m_Impl(NULL),
		m_Translator(id_translator)
	{
	}

	EntityDeserialiser::EntityDeserialiser(const EntityDeserialiseImpl *impl, const IDTranslator &id_translator)
		: m_Manager(NULL),
		m_Impl(impl),
		m_Translator(id_translator)
	{
	}

	EntityPtr EntityDeserialiser::GetEntity(ObjectID id) const
	{
		if (m_Impl != NULL)
			return m_Impl->GetEntity(m_Translator(id));
		else
			return m_Manager->GetEntity(m_Translator(id));
	}

}
