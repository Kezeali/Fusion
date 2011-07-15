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

#include "FusionAngelScriptComponent.h"

namespace FusionEngine
{

	ASScript::ASScript()
		: m_ReloadScript(false),
		m_ModuleBuilt(false)
	{
	}

	ASScript::~ASScript()
	{
	}

	void ASScript::OnSiblingAdded(const std::shared_ptr<IComponent>& com)
	{
		//if (auto scriptCom = dynamic_cast<ASScript*>(com.get()))
		//{
		//}
	}

	void ASScript::OnSiblingRemoved(const std::shared_ptr<IComponent>& com)
	{
	}

	void ASScript::SynchroniseParallelEdits()
	{
		IScript::SynchroniseInterface();
	}

	void ASScript::FireSignals()
	{
		IScript::FireInterfaceSignals();
	}

	bool ASScript::SerialiseContinuous(RakNet::BitStream& stream)
	{
		return true;
	}

	void ASScript::DeserialiseContinuous(RakNet::BitStream& stream)
	{
	}

	bool ASScript::SerialiseOccasional(RakNet::BitStream& stream, const bool force_all)
	{
		return m_DeltaSerialisationHelper.writeChanges(force_all, stream,
			GetScriptPath(), std::string());
	}

	void ASScript::DeserialiseOccasional(RakNet::BitStream& stream, const bool all)
	{
		std::bitset<DeltaSerialiser_t::NumParams> changes;
		std::string unused;
		m_DeltaSerialisationHelper.readChanges(stream, all, changes,
			m_Path, unused);

		if (changes[PropsIdx::ScriptPath])
			m_ReloadScript = true;
	}

	std::string ASScript::GetScriptPath() const
	{
		return m_Path;
	}

	void ASScript::SetScriptPath(const std::string& path)
	{
		m_Path = path;
		m_ReloadScript = true;

		m_DeltaSerialisationHelper.markChanged(PropsIdx::ScriptPath);
	}

}
