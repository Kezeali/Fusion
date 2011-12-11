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

#include "FusionCameraSynchroniser.h"

#include "FusionStreamingManager.h"

namespace FusionEngine
{

	CameraSynchroniser::CameraSynchroniser(StreamingManager* streaming_manager)
		: m_StreamingManager(streaming_manager)
	{
	}

	CameraPtr& CameraSynchroniser::GetCamera(ObjectID entity_id, PlayerID owner)
	{
		auto entry = m_Cameras.find(entity_id);
		if (entry != m_Cameras.end())
			return entry->second;
		else
		{
			auto& cam = m_Cameras[entity_id] = std::make_shared<Camera>();
			m_StreamingManager->AddOwnedCamera(owner, cam);
			return cam;
		}
	}

	void CameraSynchroniser::SetCameraPosition(ObjectID entity_id, const Vector2& new_pos)
	{
		auto entry = m_Cameras.find(entity_id);
		if (entry != m_Cameras.end())
			m_Cameras[entity_id]->SetSimPosition(new_pos);
	}

}
