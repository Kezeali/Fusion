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

#ifndef H_FusionPolygonResourceEditor
#define H_FusionPolygonResourceEditor

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionResource.h"

#include "FusionResourceEditor.h"
#include "FusionEditorPolygonTool.h"

#include "FusionPolygonLoader.h"
#include "FusionConsole.h"
#include "FusionVirtualFileSource_PhysFS.h"

namespace FusionEngine
{

	class PolygonResourceEditor : public ResourceEditor
	{
	public:
		PolygonResourceEditor();
		virtual ~PolygonResourceEditor() {}

		void SetPolygonToolExecutor(PolygonToolExecutor_t fn);

		void SetResource(const ResourceDataPtr& resource);

		void Finish();

		void CancelEditing();

		bool IsDoneEditing() const;

		void OnPolygonToolDone(const std::vector<Vector2>& verts);

		bool m_DoneEditing;

		PolygonToolExecutor_t m_PolygonEditorCb;
		ResourceDataPtr m_Resource;
		std::unique_ptr<b2PolygonShape> m_EditedShape;
	};

	inline PolygonResourceEditor::PolygonResourceEditor()
		: m_DoneEditing(true)
	{}

	inline void PolygonResourceEditor::SetPolygonToolExecutor(PolygonToolExecutor_t fn)
	{
		m_PolygonEditorCb = fn;
	}

	void PolygonResourceEditor::OnPolygonToolDone(const std::vector<Vector2>& verts)
	{
		std::vector<b2Vec2> b2Verts;
		b2Verts.reserve(verts.size());
		for (auto it = verts.begin(); it != verts.end(); ++it)
		{
			b2Verts.push_back(b2Vec2(it->x, it->y));
		}

		if (!m_EditedShape)
			m_EditedShape.reset(new b2PolygonShape);

		std::unique_ptr<b2PolygonShape> newShape(new b2PolygonShape);
		newShape->Set(b2Verts.data(), b2Verts.size());
		if (newShape->Validate())
			m_EditedShape.swap(newShape);
		Finish();
	}

	inline void PolygonResourceEditor::SetResource(const ResourceDataPtr& resource)
	{
		m_Resource = resource;
		std::vector<Vector2> verts;
		auto polygon = static_cast<b2PolygonShape*>(m_Resource->GetDataPtr());
		for (int i = 0; i < polygon->GetVertexCount(); ++i)
		{
			Vector2 vert = b2v2(polygon->GetVertex(i));
			verts.push_back(vert);
		}

		m_EditedShape.reset(new b2PolygonShape(*polygon));

		using namespace std::placeholders;
		m_PolygonEditorCb(verts, std::bind(&PolygonResourceEditor::OnPolygonToolDone, this, _1));
	}

	inline void PolygonResourceEditor::Finish()
	{
		if (m_Resource && m_EditedShape)
		{
			try
			{
				CL_VirtualDirectory vdir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
				auto dev = vdir.open_file(m_Resource->GetPath(), CL_File::create_always, CL_File::access_write);
				PolygonResource::Save(dev, *m_EditedShape);
			}
			catch (CL_Exception& e)
			{
				SendToConsole("Failed to save polygon resource '" + m_Resource->GetPath() + "': " + e.what());
			}

			auto oldData = static_cast<b2PolygonShape*>(m_Resource->GetDataPtr());
			m_Resource->SetDataPtr(m_EditedShape.release());
			delete oldData;
			m_Resource->SigReLoaded(m_Resource);
			m_Resource.reset();
		}
		m_DoneEditing = true;
	}

	inline void PolygonResourceEditor::CancelEditing()
	{
		Finish();
	}

	inline bool PolygonResourceEditor::IsDoneEditing() const
	{
		return m_DoneEditing;
	}

}

#endif
