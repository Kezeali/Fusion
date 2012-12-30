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

#include "PrecompiledHeaders.h"

#include "FusionScriptModule.h"

#include "FusionScriptManager.h"


namespace FusionEngine
{

	Module::Module()
		: m_Module(nullptr),
		m_Built(false)
	{}

	Module::Module(asIScriptModule *module)
		: m_Module(module),
		m_Built(false)
	{
	}

	Module::~Module()
	{
	}

	ScriptUtils::Calling::Caller Module::GetCaller(const std::string &decl)
	{
		ScriptManager *manager = ScriptManager::getSingletonPtr();
		auto context = manager->CreateContext();
		ScriptUtils::Calling::Caller caller(context.get(), m_Module->GetFunctionByDecl(decl.c_str()));
		return caller;
	}

	boost::signals2::connection Module::ConnectToBuild(BuildModuleSlotType slot)
	{
		return SigBuildModule.connect(slot);
	}

	int Module::AddCode(const std::string &name, const std::string &script)
	{
		if (m_Built)
		{
			m_Built = false;
			m_SectionNames.clear();
		}
		m_SectionNames.push_back(name);
		return m_Module->AddScriptSection(name.c_str(), script.c_str(), script.length());
	}

	int Module::Build()
	{
		BuildModuleEvent buildEvent;
		buildEvent.type = BuildModuleEvent::PreBuild;
		buildEvent.manager = ScriptManager::getSingletonPtr();
		buildEvent.module_name = m_Module->GetName();
		SigBuildModule(buildEvent);

		int r = m_Module->Build();

		if( r >= 0 )
		{
			//BuildModuleEvent postbuild;
			buildEvent.type = BuildModuleEvent::PostBuild;
			SigBuildModule(buildEvent);

			m_Built = true;
		}

		return r;
	}

	const char* Module::GetName() const
	{
		return m_Module->GetName();
	}

	const StringVector &Module::GetSectionNames() const
	{
		return m_SectionNames;
	}

}
