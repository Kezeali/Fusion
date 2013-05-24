/*
*  Copyright (c) 2013 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionSystemWorld.h"

#include "FusionComponentSystem.h"
#include "Messaging/FusionRouterTask.h"

namespace FusionEngine
{

	SystemWorldBase::SystemWorldBase(IComponentSystem* system)
		: m_System(system)
	{
		m_RouterTask = new RouterTask(this, Messaging::Address(system->GetName().data(), system->GetName().length()));
	}

	SystemWorldBase::~SystemWorldBase()
	{
		delete m_RouterTask;
	}

	SystemType SystemWorldBase::GetSystemType() const
	{
		return GetSystem()->GetType();
	}

	void SystemWorldBase::SendEngineRequest(EngineRequest request)
	{
		m_RouterTask->ReceiveMessage(Messaging::Message("Engine", 0, request));
	}

	void SystemWorldBase::SendComponentDispatchRequest(ComponentDispatchRequest request)
	{
		m_RouterTask->ReceiveMessage(Messaging::Message("Universe", 0, request));
	}

	void SystemWorldBase::CancelPreparation(const ComponentPtr& component)
	{
		if (component->IsPreparing())
			FSN_EXCEPT(NotImplementedException, "CancelPreparation() isn't implemented by " + GetSystem()->GetName());
	}

	SystemTaskBase* SystemWorldBase::GetTask()
	{
		return m_RouterTask;
	}

	Messaging::Router* SystemWorldBase::GetRouter() const
	{
		return m_RouterTask;
	}

}