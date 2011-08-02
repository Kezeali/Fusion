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

#ifndef H_FusionAngelScriptComponent
#define H_FusionAngelScriptComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionScriptComponent.h"

#include "FusionSerialisationHelper.h"

#include "FusionScriptModule.h"
#include "FusionScriptReference.h"

class CScriptAny;

namespace FusionEngine
{

	class ASScript : public IComponent, public IScript
	{
		friend class AngelScriptWorld;
		friend class AngelScriptTask;
	public:
		FSN_LIST_INTERFACES((IScript))

		struct PropsIdx { enum Names : size_t {
			ScriptPath,
			Unused,
			NumProps
		}; };
		typedef SerialisationHelper<
			std::string, std::string> // ScriptPath
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		ASScript();
		virtual ~ASScript();

		void Yield();
		void CreateCoroutine(const std::string& functionName);
		void CreateCoroutine(asIScriptFunction* function);
		CScriptAny* GetProperty(unsigned int index);
		bool SetProperty(unsigned int index, void* ref, int typeId);

		void SetScriptObject(asIScriptObject* obj, const std::vector<std::pair<std::string, std::string>>& interface_properties);

		void CheckChangedPropertiesIn();

	private:
		// IComponent
		std::string GetType() const { return "ASScript"; }

		void OnSiblingAdded(const std::shared_ptr<IComponent>& com);
		void OnSiblingRemoved(const std::shared_ptr<IComponent>& com);

		bool SerialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		DeltaSerialiser_t m_DeltaSerialisationHelper;

		// IScript interface
		const std::string& GetScriptPath() const;
		void SetScriptPath(const std::string& path);

		std::string m_Path;
		bool m_ReloadScript;

		bool m_ModuleBuilt;

		ModulePtr m_Module;
		ScriptObject m_ScriptObject; // An instance of the class that the script defines
		std::map<std::string, int> m_ScriptMethods;
		std::vector<boost::intrusive_ptr<asIScriptContext>> m_ActiveCoroutines;

		std::vector<std::shared_ptr<IComponentProperty>> m_ScriptProperties;
	};

}

#endif