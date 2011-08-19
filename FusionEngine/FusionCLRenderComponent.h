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

#include <boost/signals2/connection.hpp>

#include <ClanLib/display.h>

namespace FusionEngine
{

	class SpriteAnimation;
	class SpriteDefinition2;

	class IDrawable : public IComponent
	{
	public:
		virtual ~IDrawable() {}

		virtual void Update(unsigned int tick, const float delta, const float alpha) {}
		//virtual void Interpolate(const float alpha) {}
		virtual void Draw(CL_GraphicContext& gc, const Vector2& offset) = 0;

		virtual int GetEntityDepth() const = 0;
		virtual int GetLocalDepth() const = 0;

		virtual Vector2 GetPosition() const = 0;

		virtual bool HasAABB() const { return false; }
		virtual CL_Rectf GetAABB() { return CL_Rectf(); }
	};

	class CLSprite : public IDrawable, public ISprite
	{
		friend class CLRenderWorld;
		friend class CLRenderTask;
	public:
		FSN_LIST_INTERFACES((ISprite))

		struct PropsIdx { enum Names : size_t {
			Offset = 0, LocalDepth,
			ImagePath, ReloadImage, AnimationPath, ReloadAnimation,
			AlignmentOrigin, AlignmentOffset, RotationOrigin, RotationOffset,
			Colour,
			Alpha,
			Scale, BaseAngle,
			NumProps
		}; };
		typedef SerialisationHelper<
			Vector2, int, // offset, depth
			std::string, bool, std::string, bool, // image path, animation path
			CL_Origin, Vector2i, CL_Origin, Vector2i, // Alignment, rotation hotspot
			CL_Colorf, // colour
			float, // alpha
			Vector2, float> // scale, base-angle
			DeltaSerialiser_t;
		static_assert(PropsIdx::NumProps == DeltaSerialiser_t::NumParams, "Must define names for each param in the SerialisationHelper");

		CLSprite();
		virtual ~CLSprite();

	private:
		float ToRender(float sim_coord)
		{
			return sim_coord * s_GameUnitsPerSimUnit;
		}

		Vector2 ToRender(const Vector2& sim_coord)
		{
			return Vector2(ToRender(sim_coord.x), ToRender(sim_coord.y));
		}
		
		void SetPosition(const Vector2& value);
		Vector2 GetPosition() const;

		void SetAngle(float angle);
		
		// IDrawable
		void Draw(CL_GraphicContext& gc, const Vector2& offset);
		void Update(unsigned int tick, const float elapsed, const float alpha);
		//void Interpolate(const float alpha);

		int GetEntityDepth() const { return m_EntityDepth; }
		int GetLocalDepth() const { return m_LocalDepth; }

		int m_EntityDepth;
		int m_LocalDepth;

		bool HasAABB() const { return !m_Sprite.is_null(); }
		CL_Rectf GetAABB()
		{
			return m_AABB;
		}

		CL_Rectf m_AABB;

		// IComponent
		std::string GetType() const { return "CLSprite"; }

		void OnSiblingAdded(const std::shared_ptr<IComponent>& component);

		void SynchroniseParallelEdits();
		void FireSignals();

		bool SerialiseOccasional(RakNet::BitStream& stream, const bool force_all);
		void DeserialiseOccasional(RakNet::BitStream& stream, const bool all);

		// ISprite
		void SetOffset(const Vector2& offset);
		void SetLocalDepth(int value);
		//void SetInterpolate(bool value);
		
		void SetImagePath(const std::string& value);
		const std::string &GetImagePath() const;
		void SetAnimationPath(const std::string& value);
		const std::string &GetAnimationPath() const;

		void SetAlignmentOrigin(CL_Origin origin);
		CL_Origin GetAlignmentOrigin() const;

		void SetAlignmentOffset(const Vector2i& offset);
		Vector2i GetAlignmentOffset() const;

		void SetRotationOrigin(CL_Origin origin);
		CL_Origin GetRotationOrigin() const;

		void SetRotationOffset(const Vector2i& offset);
		Vector2i GetRotationOffset() const;

		mutable CL_Colorf m_Colour; // Since CL_Sprite::get_color returns by value (for no good reason)
		void SetColour(const CL_Colorf& val);
		const CL_Colorf &GetColour() const;
		
		void SetAlpha(float val);
		float GetAlpha() const;

		void SetScale(const Vector2& val);
		Vector2 GetScale() const;

		void SetBaseAngle(float val);
		float GetBaseAngle() const;

		bool IsAnimationFinished() const;

		boost::signals2::scoped_connection m_ImageLoadConnection;
		ResourcePointer<CL_Texture> m_ImageResource;

		boost::signals2::scoped_connection m_AnimationLoadConnection;
		ResourcePointer<SpriteAnimation> m_AnimationResource;

		void redefineSprite();
		std::unique_ptr<SpriteDefinition2> m_SpriteDef;
		CL_Sprite m_Sprite;

		int m_AnimationFrame; // Calculated whenever Update is called
		int m_InterpAnimationFrame;
		int m_LastAnimationFrame;

		float m_DeltaTime;
		float m_ElapsedTime;

		//bool m_Interpolate;

		bool m_PositionSet; // Set to true when SetPosition is called for the first time (to init m_LastPosition)
		bool m_AngleSet;
		unsigned int m_InterpTick; // Tick for which interpolation needs to happen (i.e. when the sim values changed)
		unsigned int m_CurrentTick;

		CL_Font m_DebugFont;

		ThreadSafePropertyConnection m_PositionChangeConnection;
		Vector2 m_NewPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		Vector2 m_Offset;

		ThreadSafePropertyConnection m_AngleChangeConnection;
		float m_NewAngle;

		float m_Angle;
		float m_InterpAngle;
		float m_LastAngle;

		ThreadSafePropertyConnection m_DepthChangeConnection;

		std::string m_ImagePath;
		bool m_ReloadImage;

		std::string m_AnimationPath;
		bool m_ReloadAnimation;

		bool m_RecreateSprite;

		DeltaSerialiser_t m_SerialisationHelper;
	};

}

#endif
