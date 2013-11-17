/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include <boost/thread/mutex.hpp>

#include "FusionSingleton.h"
#include <tbb/concurrent_hash_map.h>

namespace FusionEngine
{

	class SpriteAnimation;
	class SpriteDefinition;

	class IDrawable : public EntityComponent
	{
	public:
		virtual ~IDrawable() {}

		virtual void Update(unsigned int tick, const float delta, const float alpha) {}
		//virtual void Interpolate(const float alpha) {}
		virtual void Draw(clan::Canvas& canvas, const Vector2& offset) = 0;

		virtual int GetEntityDepth() const = 0;
		virtual int GetLocalDepth() const = 0;

		virtual Vector2 GetPosition() const = 0;

		virtual bool HasAABB() const { return false; }
		virtual clan::Rectf GetAABB() { return clan::Rectf(); }
	};

	class SpriteDefinitionCache : Singleton<SpriteDefinitionCache>
	{
	public:
		static std::shared_ptr<SpriteDefinition> GetSpriteDefinition(const ResourcePointer<clan::Texture2D>& texture, const ResourcePointer<SpriteAnimation>& animation);

		typedef tbb::concurrent_hash_map<std::pair<std::string, std::string>, std::weak_ptr<SpriteDefinition>> SpriteDefinitionMap_t;
		SpriteDefinitionMap_t m_SpriteDefinitions;
	};

	class CLSprite : public IDrawable, public ISprite
	{
		friend class CLRenderWorld;
		friend class CLRenderTask;
	public:
		FSN_LIST_INTERFACES((IRenderCom)(ISprite))

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
		void Draw(clan::Canvas& canvas, const Vector2& offset);

		void CreateSpriteIfNecessary(clan::Canvas& canvas);

		void DefineSpriteIfNecessary();

		bool ImageHotReloadEvents(ResourceDataPtr resource, ResourceContainer::HotReloadEvent ev);
		bool AnimationHotReloadEvents(ResourceDataPtr resource, ResourceContainer::HotReloadEvent ev);

		void ImageLoaded(ResourceDataPtr data);
		void AnimationLoaded(ResourceDataPtr data);

		bool RequiresSpriteDefinition() const { return m_RecreateSprite && !m_SpriteDef; }

		void Update(unsigned int tick, const float elapsed, const float alpha);
		//void Interpolate(const float alpha);

		int GetEntityDepth() const { return m_EntityDepth; }
		int GetLocalDepth() const { return m_LocalDepth; }

		int m_EntityDepth;
		int m_LocalDepth;

		bool HasAABB() const { return !m_Sprite.is_null(); }
		clan::Rectf GetAABB()
		{
			return m_AABB;
		}

		clan::Rectf m_AABB;

		// EntityComponent
		std::string GetType() const { return "CLSprite"; }

		void OnSiblingAdded(const ComponentPtr& component);

		void SerialiseOccasional(RakNet::BitStream& stream);
		void DeserialiseOccasional(RakNet::BitStream& stream);

		void OnPostDeserialisation();

		// ISprite
		void SetOffset(const Vector2& offset);
		Vector2 GetOffset() const;
		void SetLocalDepth(int value);
		//void SetInterpolate(bool value);
		
		void SetImagePath(const std::string& value);
		const std::string &GetImagePath() const;
		void SetAnimationPath(const std::string& value);
		const std::string &GetAnimationPath() const;

		void SetAlignmentOrigin(clan::Origin origin);
		clan::Origin GetAlignmentOrigin() const;

		void SetAlignmentOffset(const Vector2i& offset);
		Vector2i GetAlignmentOffset() const;

		void SetRotationOrigin(clan::Origin origin);
		clan::Origin GetRotationOrigin() const;

		void SetRotationOffset(const Vector2i& offset);
		Vector2i GetRotationOffset() const;

		void SetColour(const clan::Colorf& val);
		const clan::Colorf &GetColour() const;
		
		void SetAlpha(float val);
		float GetAlpha() const;

		void SetScale(const Vector2& val);
		Vector2 GetScale() const;

		void SetBaseAngle(float val);
		float GetBaseAngle() const;

		bool IsAnimationFinished() const;
		
		void SetAnimationFrame(int val);
		int GetAnimationFrame() const;

		void SetPlaying(bool val);
		bool IsPlaying() const;

		void SetLooping(bool val);
		bool IsLooping() const;

		void Finish();

		boost::mutex m_Mutex;

		clan::Colorf m_Colour;
		clan::Origin m_AlignmentOrigin;
		Vector2i m_AlignmentOffset;
		clan::Origin m_RotationOrigin;
		Vector2i m_RotationOffset;
		float m_Alpha;
		Vector2 m_Scale;
		float m_BaseAngle;

		bool m_Looping;

		boost::signals2::scoped_connection m_ImageLoadConnection;
		ResourcePointer<clan::Texture2D> m_ImageResource;

		boost::signals2::scoped_connection m_AnimationLoadConnection;
		ResourcePointer<SpriteAnimation> m_AnimationResource;

		void redefineSprite();
		//std::unique_ptr<SpriteDefinition> m_SpriteDef;
		std::shared_ptr<SpriteDefinition> m_SpriteDef;
		clan::Sprite m_Sprite;

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

		clan::Font m_DebugFont;

		SyncSig::HandlerConnection_t m_PositionChangeConnection;
		Vector2 m_NewPosition; // Set by SetPosition

		Vector2 m_Position; // NewPosition is copied here when Update is called
		Vector2 m_InterpPosition;
		Vector2 m_LastPosition;

		Vector2 m_Offset;

		SyncSig::HandlerConnection_t m_AngleChangeConnection;
		float m_NewAngle;

		float m_Angle;
		float m_InterpAngle;
		float m_LastAngle;

		SyncSig::HandlerConnection_t m_DepthChangeConnection;

		std::string m_ImagePath;
		bool m_ReloadImage;

		std::string m_AnimationPath;
		bool m_ReloadAnimation;

		bool m_RecreateSprite;
	};

}

#endif
