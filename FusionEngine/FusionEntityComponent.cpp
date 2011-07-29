/*
*  Copyright (c) 2011 Fusion Project Team
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

#include "FusionStableHeaders.h"

#include "FusionEntityComponent.h"

#include "FusionThreadSafeProperty.h"

namespace FusionEngine
{

	void IComponent::AddProperty(IComponentProperty* prop)
	{
		FSN_ASSERT(std::find(m_Properties.begin(), m_Properties.end(), prop) == m_Properties.end());
		m_Properties.push_back(prop);
		m_ChangedProperties->push(prop);
	}

	void IComponent::OnPropertyChanged(IComponentProperty* prop)
	{
		FSN_ASSERT(m_ChangedProperties);
		m_ChangedProperties->push(prop);
	}

	void IComponent::SynchronisePropertiesNow()
	{
		for (auto it = m_Properties.begin(), end = m_Properties.end(); it != end; ++it)
		{
			auto prop = *it;
			prop->Synchronise();
		}
	}

}