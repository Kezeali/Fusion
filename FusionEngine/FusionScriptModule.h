/*
  Copyright (c) 2009-2011 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptModule
#define Header_FusionEngine_ScriptModule

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// External
#include <ScriptUtils/Calling/Caller.h>

// Fusion
#include "FusionBoostSignals2.h"
#include "FusionScriptReference.h"


namespace FusionEngine
{

	//! Build Event (for build signals)
	struct BuildModuleEvent
	{
		enum EventType
		{
			PreBuild,
			PostBuild
		};

		EventType type;
		ScriptManager *manager;
		const char* module_name;
	};

	//! Stores a module pointer
	class Module
	{
	public:
		typedef boost::signals2::signal<void (BuildModuleEvent&)> BuildModuleSignalType;
		typedef BuildModuleSignalType::slot_type BuildModuleSlotType;

		//typedef std::tr1::shared_ptr<BuildModuleSignalType> BuildModuleSignalTypePtr;

	public:
		Module();
		Module(asIScriptModule *module);

		~Module();

	public:
		ScriptObject CreateObject(const std::string& name);
		ScriptObject CreateObject(int type_id);

		ScriptUtils::Calling::Caller GetCaller(const std::string &decl);

		boost::signals2::connection ConnectToBuild(BuildModuleSlotType slot);

		int AddCode(const std::string &section_name, const std::string &script);
		int Build();

		const char* GetName() const;

		asIScriptModule *GetASModule() const { return m_Module; }

		//! Returns a list of names of script sections that have been added to this module
		const StringVector &GetSectionNames() const;

		BuildModuleSignalType SigBuildModule;

	protected:
		asIScriptModule *m_Module;

		StringVector m_SectionNames;
		bool m_Built;
	};

}

#endif
