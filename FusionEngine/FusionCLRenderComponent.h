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

#ifndef H_FusionCLRenderComponent
#define H_FusionCLRenderComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"

#include "FusionRender2DComponent.h"
#include "FusionResourcePointer.h"

#include <boost/signals/connection.hpp>

namespace FusionEngine
{

	class CLSprite : public IComponent, public ISprite
	{
		friend class CLRenderSystem;
	private:
		float ToRender(float sim_coord)
		{
			return sim_coord * s_GameUnitsPerSimUnit;
		}

		Vector2 ToRender(const Vector2& sim_coord)
		{
			return Vector2(ToRender(sim_coord.x), ToRender(sim_coord.y));
		}

		CLSprite();

		void Update(const float elapsed);

		void OnResourceLoaded(ResourceDataPtr data);

		// IComponent
		void SynchroniseParallelEdits();

		bool SerialiseContinuous(RakNet::BitStream& stream);
		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseContinuous(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		void SetPosition(const Vector2& value);
		void SetOffset(const Vector2& value);
		void SetFilePath(const std::string& value);

		boost::signals2::scoped_connection m_ResourceLoadConnection;
		ResourcePointer<CL_Sprite> m_SpriteResource;

		Vector2 m_NewPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		Vector2 m_Offset;

		std::string m_FilePath;
		bool m_Reload;
	};

}

#endif
