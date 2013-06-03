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

#ifndef H_FusionSystemTask
#define H_FusionSystemTask

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionSystemType.h"

#include <tbb/concurrent_queue.h>

#include <EASTL/string.h>

#include <boost/intrusive/slist_hook.hpp>

namespace FusionEngine { namespace System
{
	
	class WorldBase;

	//! Task
	class TaskBase : public boost::intrusive::slist_base_hook<>
	{
	public:
		TaskBase(WorldBase* world, const eastl::string& name)
			: m_SystemWorld(world),
			m_Name(name)
		{}
		virtual ~TaskBase() {}

		WorldBase* GetSystemWorld() const { return m_SystemWorld; }

		SystemType GetSystemType() const;

		std::string GetSystemName() const;

		eastl::string GetName() const { return m_Name; }

		virtual void Update() = 0;

		virtual SystemType GetTaskType() const = 0;

		virtual std::vector<std::string> GetDependencies() const { return std::vector<std::string>(); }

		enum PerformanceHint : uint16_t
		{
			LongSerial = 0,
			LongParallel,
			Short,
			NoPerformanceHint,
			NumPerformanceHints
		};
		virtual PerformanceHint GetPerformanceHint() const { return NoPerformanceHint; }

		virtual bool IsPrimaryThreadOnly() const = 0;

	protected:
		WorldBase *m_SystemWorld;
		eastl::string m_Name;
	};

} }

#endif
