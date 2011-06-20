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
#include "FusionSerialisationHelper.h"

#include <boost/signals/connection.hpp>

#include <ClanLib/display.h>

namespace FusionEngine
{

	class IDrawable : public IComponent
	{
	public:
		virtual ~IDrawable() {}

		virtual void Draw(CL_GraphicContext& gc) = 0;

		virtual int GetEntityDepth() const = 0;
		virtual int GetLocalDepth() const = 0;
	};

	class CLSprite : public IDrawable, public ISprite
	{
		friend class CLRenderWorld;
		friend class CLRenderTask;
	public:
		typedef boost::mpl::vector<ISprite>::type Interfaces;

		struct PropsOrder { enum Names { Offset = 0, LocalDepth, FilePath, Reload }; };
		typedef SerialisationHelper<Vector2, int, std::string, bool> DeltaSerialiser_t;

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
		~CLSprite();

		void Update(const float elapsed);

		void OnResourceLoaded(ResourceDataPtr data);
		
		void SetPosition(const Vector2& value);
		
		// IDrawable
		void Draw(CL_GraphicContext& gc);

		int GetEntityDepth() const { return m_EntityDepth; }
		int GetLocalDepth() const { return m_LocalDepth; }

		int m_EntityDepth;
		int m_LocalDepth;

		// IComponent
		std::string GetType() const { return "CLSprite"; }

		void OnSiblingAdded(const std::set<std::string>& interfaces, const std::shared_ptr<IComponent>& component);

		void SynchroniseParallelEdits();

		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// ISprite
		void SetOffset(const Vector2& offset);
		void SetLocalDepth(int value);
		void SetFilePath(const std::string& value);

		boost::signals2::scoped_connection m_ResourceLoadConnection;
		ResourcePointer<CL_Sprite> m_SpriteResource;

		boost::signals2::connection m_PositionChangeConnection;
		Vector2 m_NewPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		Vector2 m_Offset;

		std::string m_FilePath;
		bool m_Reload;

		DeltaSerialiser_t m_SerialisationHelper;
	};

}

#endif
