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

#include "PrecompiledHeaders.h"

#include "FusionArchetypeResourceEditor.h"

#include "FusionArchetypeFactory.h"
#include "FusionConsole.h"
#include "FusionPhysFSIOStream.h"
#include "FusionResourceManager.h"

namespace FusionEngine
{

	ArchetypeResourceEditor::ArchetypeResourceEditor()
		: m_DoneEditing(true)
	{}

	void ArchetypeResourceEditor::SetEntityInspectorExecutor(const ArchetypeResourceEditor::OpenEntityInspectorCallback_t& fn)
	{
		m_OpenInspector = fn;
	}

	void ArchetypeResourceEditor::SetErrorCallback(const std::function<void (const std::string&)>& fn)
	{
		m_ErrorCallback = fn;
	}

	void ArchetypeResourceEditor::SetResource(const ResourceDataPtr& resource, const Vector2& offset)
	{
		m_Resource.SetTarget(resource);
		// This might be nice, but also might suck: (commented out)
		//m_Resource->AddChangeListener(std::bind(&ArchetypeResourceEditor::Save, this));
		m_OpenInspector(m_Resource->GetArchetype(), std::bind(&ArchetypeResourceEditor::Finish, this));
	}

	void ArchetypeResourceEditor::Finish()
	{
		if (m_Resource.IsLoaded())
		{
			try
			{
				IO::PhysFSStream file(m_Resource.GetTarget()->GetPath(), IO::Write);
				m_Resource->Save(file);
			}
			catch (CL_Exception& e)
			{
				SendToConsole("Failed to save archetype resource '" + m_Resource.GetTarget()->GetPath() + "': " + e.what());
				if (m_ErrorCallback)
					m_ErrorCallback("Failed to save archetype resource '" + m_Resource.GetTarget()->GetPath() + "': " + e.what());
			}

			m_Resource.Release();
		}
		m_DoneEditing = true;
	}

	void ArchetypeResourceEditor::CancelEditing()
	{
		Finish();
	}

	bool ArchetypeResourceEditor::IsDoneEditing() const
	{
		return m_DoneEditing;
	}

}
