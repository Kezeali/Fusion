/*
*  Copyright (c) 2010 Fusion Project Team
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

#include "FusionEditorMoveAction.h"

#include "FusionEditorMapEntity.h"
#include "FusionEntity.h"

namespace FusionEngine
{

	MoveAction::MoveAction(EditorMapEntityPtr map_entity, const Vector2& offset)
		: m_MapEntity(map_entity),
		m_Offset(offset)
	{
		setTitle();
	}

	MoveAction::MoveAction(EditorMapEntityPtr map_entity, const Vector2& from, const Vector2& to)
		: m_MapEntity(map_entity),
		m_Offset(to - from)
	{
		setTitle();
	}

	void MoveAction::setTitle()
	{
		// Make a human-readable representation of the Vector2
		std::stringstream vector_string;
		vector_string << "(" << m_Offset.x << ", " << m_Offset.y << ")";

		m_Title = "Move " + std::string(m_MapEntity->hasName ? m_MapEntity->entity->GetName() : "an Entity") + " by " + vector_string.str();
	}

	void MoveAction::undoAction()
	{
		m_MapEntity->entity->SetPosition( m_MapEntity->entity->GetPosition() - m_Offset );
	}

	void MoveAction::redoAction()
	{
		m_MapEntity->entity->SetPosition( m_MapEntity->entity->GetPosition() + m_Offset );
	}

}
