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

	class SpriteAnimation;
	class SpriteDefinition2;

	class IDrawable : public IComponent
	{
	public:
		virtual ~IDrawable() {}

		virtual void Update(const float delta) {}
		virtual void Draw(CL_GraphicContext& gc, const Vector2& offset) = 0;

		virtual int GetEntityDepth() const = 0;
		virtual int GetLocalDepth() const = 0;

		virtual Vector2 GetPosition() const = 0;

		virtual bool HasBB() const { return false; }
		virtual CL_Rectf GetBB() { return CL_Rectf(); }
	};

	class CLSprite : public IDrawable, public ISprite
	{
		friend class CLRenderWorld;
		friend class CLRenderTask;
	public:
		typedef boost::mpl::vector<ISprite>::type Interfaces;

		struct PropsOrder { enum Names { Offset = 0, LocalDepth, ImagePath, ReloadImage, AnimationPath, ReloadAnimation }; };
		typedef SerialisationHelper<Vector2, int, std::string, bool, std::string, bool> DeltaSerialiser_t;

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
		
		void SetPosition(const Vector2& value);
		Vector2 GetPosition() const;
		
		// IDrawable
		void Draw(CL_GraphicContext& gc, const Vector2& offset);
		void Update(const float elapsed);

		int GetEntityDepth() const { return m_EntityDepth; }
		int GetLocalDepth() const { return m_LocalDepth; }

		int m_EntityDepth;
		int m_LocalDepth;

		bool HasBB() const { return true; }
		CL_Rectf GetBB()
		{
			CL_Rectf bb;

			CL_Origin origin;
			int x, y;
			m_Sprite.get_alignment(origin, x, y);
		}

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

		boost::signals2::scoped_connection m_ImageLoadConnection;
		ResourcePointer<CL_Texture> m_ImageResource;

		boost::signals2::scoped_connection m_AnimationLoadConnection;
		ResourcePointer<SpriteAnimation> m_AnimationResource;

		void redefineSprite();
		std::unique_ptr<SpriteDefinition2> m_SpriteDef;
		CL_Sprite m_Sprite;

		boost::signals2::connection m_PositionChangeConnection;
		Vector2 m_NewPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		Vector2 m_Offset;

		std::string m_ImagePath;
		bool m_ReloadImage;

		std::string m_AnimationPath;
		bool m_ReloadAnimation;

		bool m_RecreateSprite;

		DeltaSerialiser_t m_SerialisationHelper;
	};

}

#endif
