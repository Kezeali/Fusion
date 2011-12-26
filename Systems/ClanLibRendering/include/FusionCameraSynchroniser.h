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

#ifndef H_FusionCameraSynchroniser
#define H_FusionCameraSynchroniser

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCamera.h"
#include "FusionCameraManager.h"

namespace FusionEngine
{

	class CameraManager;

	//! Stores cameras used by camera components (keeps them alive when the associated components are inactive)
	class CameraSynchroniser
	{
	public:
		//! CTOR
		CameraSynchroniser(CameraManager* streaming_manager);

		//! Remove all cameras
		void Clear();

		//! Gets / creates a camera attached to the given entity id
		CameraPtr& GetCamera(ObjectID entity_id, PlayerID owner);
		//! Removes the camera owned by the entity
		void RemoveCamera(ObjectID entity_id);
		//! Updates the position of the given camera
		void SetCameraPosition(ObjectID entity_id, const Vector2& new_pos);

		CameraManager* GetStreamingManager() const { return m_StreamingManager; }

	private:
		std::map<ObjectID, CameraPtr> m_Cameras;

		CameraManager* m_StreamingManager;
	};

}

#endif
