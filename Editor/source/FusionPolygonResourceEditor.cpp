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

#include "FusionPolygonResourceEditor.h"

#include "FusionPolygonLoader.h"
#include "FusionConsole.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionResourceManager.h"
#include <ClanLib/core.h>

namespace FusionEngine
{

	PolygonResourceEditor::PolygonResourceEditor()
		: m_DoneEditing(true)
	{}

	void PolygonResourceEditor::SetPolygonToolExecutor(PolygonToolExecutor_t fn)
	{
		m_PolygonEditorCb = fn;
	}

	void PolygonResourceEditor::OnPolygonToolDone(const std::vector<Vector2>& verts)
	{
		if (verts.size() < b2_maxPolygonVertices)
		{
			std::vector<b2Vec2> b2Verts;
			b2Verts.reserve(verts.size());
			for (auto it = verts.begin(); it != verts.end(); ++it)
			{
				b2Verts.push_back(b2Vec2(ToSimUnits(it->x - m_Offset.x), ToSimUnits(it->y - m_Offset.y)));
			}

			if (!m_EditedShape)
				m_EditedShape.reset(new b2PolygonShape);

			std::unique_ptr<b2PolygonShape> newShape(new b2PolygonShape);
			newShape->Set(b2Verts.data(), b2Verts.size());
			if (newShape->Validate())
				m_EditedShape.swap(newShape);
		}
		else
		{
			SendToConsole("Too many verticies in edited polygon");
			if (m_ErrorCallback)
			{
				std::stringstream str; str << b2_maxPolygonVertices;
				m_ErrorCallback("Too many verticies in edited polygon. Reduce the verts to less than " + str.str() + " before saving.");
			}
			// Restart the editor
			auto vertsCopy = verts;
			using namespace std::placeholders;
			m_PolygonEditorCb(vertsCopy, std::bind(&PolygonResourceEditor::OnPolygonToolDone, this, _1));
		}
		Finish();
	}

	void PolygonResourceEditor::SetResource(const ResourceDataPtr& resource, const Vector2& offset)
	{
		m_Resource = resource;
		std::vector<Vector2> verts;
		auto polygon = static_cast<b2PolygonShape*>(m_Resource->GetDataPtr());
		for (int i = 0; i < polygon->GetVertexCount(); ++i)
		{
			const auto& b2vert = polygon->GetVertex(i);
			verts.push_back(Vector2(ToRenderUnits(b2vert.x), ToRenderUnits(b2vert.y)) + offset);
		}

		m_Offset = offset;

		m_EditedShape.reset(new b2PolygonShape(*polygon));

		using namespace std::placeholders;
		m_PolygonEditorCb(verts, std::bind(&PolygonResourceEditor::OnPolygonToolDone, this, _1));
	}

	void PolygonResourceEditor::Finish()
	{
		if (m_Resource && m_EditedShape)
		{
			try
			{
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "/");
				auto dev = vdir.open_file(m_Resource->GetPath(), CL_File::create_always, CL_File::access_write);
				PolygonResource::Save(dev, *m_EditedShape);
			}
			catch (CL_Exception& e)
			{
				SendToConsole("Failed to save polygon resource '" + m_Resource->GetPath() + "': " + e.what());
				if (m_ErrorCallback)
					m_ErrorCallback("Failed to save polygon resource '" + m_Resource->GetPath() + "': " + e.what());
			}

			auto oldData = static_cast<b2PolygonShape*>(m_Resource->GetDataPtr());
			m_Resource->SetDataPtr(m_EditedShape.release());
			delete oldData;
			//m_Resource->SigReLoaded(m_Resource);
			m_Resource.reset();
		}
		m_DoneEditing = true;
	}

	void PolygonResourceEditor::CancelEditing()
	{
		Finish();
	}

	bool PolygonResourceEditor::IsDoneEditing() const
	{
		return m_DoneEditing;
	}

}
