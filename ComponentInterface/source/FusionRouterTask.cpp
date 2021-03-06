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

#include "Messaging/FusionRouterTask.h"

#include "FusionDeltaTime.h"
#include "FusionSystemWorld.h"

namespace FusionEngine
{

	using namespace System;

	RouterTask::RouterTask(WorldBase* sysworld, const eastl::string& name)
		: TaskBase(sysworld, name),
		Router(Messaging::Address(name))
	{
	}

	RouterTask::~RouterTask()
	{
	}

	void RouterTask::Update()
	{
		Process(DeltaTime::GetDeltaTime() * 0.9f);
	}

	void RouterTask::ProcessMessage(Messaging::Message message)
	{
		m_SystemWorld->ProcessMessage(message);
	}

}
