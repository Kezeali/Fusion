/*
*  Copyright (c) 2010 Fusion Project Team
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

#ifndef Header_FusionEntityDecoratorInstancer
#define Header_FusionEntityDecoratorInstancer

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <Rocket/Core/DecoratorInstancer.h>

namespace FusionEngine
{

	class EntityDecoratorInstancer : public Rocket::Core::DecoratorInstancer
	{
	public:
		EntityDecoratorInstancer(EntityManager* source, Renderer* renderer);

		//! Instances a decorator given the property tag and attributes from the RCSS file.
		virtual Rocket::Core::Decorator* InstanceDecorator(const EMP::Core::String& name, const Rocket::Core::PropertyDictionary& properties);
		//! Releases the given decorator.
		virtual void ReleaseDecorator(Rocket::Core::Decorator* decorator);

		//! Releases the instancer.
		virtual void Release();

	protected:
		EntityManager* m_EntityManager;
		Renderer* m_Renderer;
	};

}

#endif