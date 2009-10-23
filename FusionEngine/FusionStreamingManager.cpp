#include "FusionCommon.h"

#include "FusionStreamingManager.h"

namespace FusionEngine
{

	const float StreamingManager::s_SmoothTightness = 0.1f;
	const float StreamingManager::s_FastTightness = 0.3f;

	StreamingManager::StreamingManager()
	{
	}

	StreamingManager::~StreamingManager()
	{
	}

	void StreamingManager::SetPlayerCamera(ObjectID net_idx, const CameraPtr &cam)
	{
		StreamingCamera &stCam = m_Cameras[net_idx];

		stCam.Camera = cam;
		stCam.LastPosition.x = cam->GetPosition().x; stCam.LastPosition.y = cam->GetPosition().y;
		stCam.StreamPoint = stCam.LastPosition;
		stCam.Tightness = s_SmoothTightness;
	}

	void StreamingManager::RemovePlayerCamera(ObjectID net_idx)
	{
		m_Cameras.erase(net_idx);
	}

	void StreamingManager::SetRange(float game_units)
	{
		m_Range = game_units;
		m_RangeSquared = game_units * game_units;
	}

	//CL_Rectf StreamingManager::CalculateActiveArea(ObjectID net_idx) const
	//{
	//	CL_Rectf area;

	//	StreamingCameraMap::const_iterator _where = m_Cameras.find(net_idx);
	//	if (_where != m_Cameras.end())
	//	{
	//		const StreamingCamera &stCam = _where->second;

	//		area.left = stCam.StreamPoint.x - m_Range;
	//		area.top = stCam.StreamPoint.y - m_Range;
	//		area.right = stCam.StreamPoint.x + m_Range;
	//		area.bottom = stCam.StreamPoint.y + m_Range;
	//	}

	//	return area;
	//}

	void StreamingManager::ProcessEntity(const EntityPtr &entity) const
	{
		for (StreamingCameraMap::const_iterator it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			if (processEntity(it->second, entity))
				return;
		}

		// The entity wansn't within the streaming area of
		//  any camera - stream out if not already
		if (!entity->IsStreamedOut())
		{
			entity->StreamOut();
		}
	}

	//bool rect_contains_point(float l, float t, float r, float b, float x, float y)
	//{
	//	return x > l && x < r && y > t && y < b;
	//}

	bool StreamingManager::processEntity(const StreamingManager::StreamingCamera &cam, const EntityPtr &entity) const
	{
		const Vector2 &entityPos = entity->GetPosition();
		CL_Rectf camArea(
			cam.StreamPoint.x - m_Range,
			cam.StreamPoint.y - m_Range,
			cam.StreamPoint.x + m_Range,
			cam.StreamPoint.y + m_Range);

		if (entityPos.x > camArea.left && entityPos.x < camArea.right && entityPos.y > camArea.top && entityPos.y < camArea.bottom)
		{
			if (entity->IsStreamedOut())
				entity->StreamIn();
			return true;
		}

		return false;
	}

	void StreamingManager::Update()
	{
		for (StreamingCameraMap::iterator it = m_Cameras.begin(), end = m_Cameras.end(); it != end; ++it)
		{
			StreamingCamera &cam = it->second;

			const CL_Vec2f &camPos = cam.Camera->GetPosition();
			
			Vector2 velocity( camPos.x - cam.LastPosition.x, camPos.y - cam.LastPosition.y );
			// Only interpolate / anticipate movement if the velocity doesn't indicate a jump in position
			if (velocity.squared_length() > m_RangeSquared)
			{
				Vector2 target( camPos.x + velocity.x, camPos.y + velocity.y );

				cam.StreamPoint = cam.StreamPoint + (target-cam.StreamPoint) * cam.Tightness;
			}
			else
				cam.StreamPoint.set(camPos.x, camPos.y);

			// If the velocity has changed, smooth (over sudden changes in velocity) by adjusting interpolation tightness
			if (!v2Equal(cam.LastVelocity, velocity, 0.1f))
				cam.Tightness = s_SmoothTightness;
			else
				cam.Tightness += (s_FastTightness - cam.Tightness) * 0.01f;

			cam.LastPosition.x = camPos.x; cam.LastPosition.y = camPos.y;
			cam.LastVelocity = velocity;
		}
	}

	void StreamingManager_SetPlayerCamera(ObjectID net_idx, Camera *camera, StreamingManager *obj)
	{
		obj->SetPlayerCamera(net_idx, CameraPtr(camera) );
		camera->release();
	}

	void StreamingManager_RemovePlayerCamera(ObjectID net_idx, StreamingManager *obj)
	{
		obj->RemovePlayerCamera( net_idx );
	}

	//void StreamingManager_AddCamera(Camera *camera, StreamingManager *obj)
	//{
	//	obj->AddCamera( CameraPtr(camera) );
	//}

	//void StreamingManager_RemoveCamera(Camera *camera, StreamingManager *obj)
	//{
	//	obj->RemoveCamera( CameraPtr(camera) );
	//}

	void StreamingManager::Register(asIScriptEngine *engine)
	{
		int r;
		RegisterSingletonType<StreamingManager>("StreamingManager", engine);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void setPlayerCamera(uint16, Camera@)",
			asFUNCTION(StreamingManager_SetPlayerCamera), asCALL_CDECL_OBJLAST);
		r = engine->RegisterObjectMethod("StreamingManager",
			"void removePlayerCamera(uint16)",
			asFUNCTION(StreamingManager_RemovePlayerCamera), asCALL_CDECL_OBJLAST);
	}

}
