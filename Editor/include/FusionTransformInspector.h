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

#ifndef H_FusionTransformInspector
#define H_FusionTransformInspector

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionComponentInspector.h"

#include "FusionTransformComponent.h"

#include <Rocket/Controls/ElementFormControlInput.h>

namespace FusionEngine { namespace Inspectors
{

	class TransformInspector : public ComponentInspector
	{
	public:
		TransformInspector(const Rocket::Core::String& tag);

		void SetComponents(const std::vector<ComponentPtr>& components);
		void ReleaseComponents();

	private:
		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> m_XInput;
		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> m_YInput;
		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> m_AngleInput;
		boost::intrusive_ptr<Rocket::Controls::ElementFormControlInput> m_DepthInput;

		std::vector<ComponentIPtr<ITransform>> m_Components;

		void InitUI();

		void ProcessEvent(Rocket::Core::Event& ev);
	};

} }

#endif