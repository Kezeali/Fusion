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

#ifndef H_FusionRender2DComponent
#define H_FusionRender2DComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"
#include "FusionEntityComponent.h"
#include "FusionThreadSafeProperty.h"
#include "FusionViewport.h"

#include <ClanLib/Display/2D/color.h>

namespace FusionEngine
{

	FSN_BEGIN_COIFACE(IRenderCom)
	public:
		FSN_COIFACE_PROPS(IRenderCom,
			((FSN_GET_SET)(Offset)(Vector2))
			((FSN_GET_SET)(LocalDepth)(int)) )

	private:
		virtual void SetOffset(const Vector2& value) = 0;
		virtual Vector2 GetOffset() const = 0;
		
		virtual void SetLocalDepth(int value) = 0;
		virtual int GetLocalDepth() const = 0;

		//virtual void SetInterpolate(bool value) = 0;
	};

	class ISprite : public IRenderCom
	{
	public:
		static std::string GetTypeName() { return "ISprite"; }
		virtual ~ISprite()
		{}

		FSN_COIFACE_PROPS(ISprite,
			((FSN_GET_SET)(ImagePath)(std::string))
			((FSN_GET_SET)(AnimationPath)(std::string))

			((FSN_GET_SET)(AlignmentOrigin)(clan::Origin))
			((FSN_GET_SET)(AlignmentOffset)(Vector2i))
			((FSN_GET_SET)(RotationOrigin)(clan::Origin))
			((FSN_GET_SET)(RotationOffset)(Vector2i))

			((FSN_GET_SET)(Colour)(clan::Colorf))
			((FSN_GET_SET)(Alpha)(float))
			((FSN_GET_SET)(Scale)(Vector2))
			((FSN_GET_SET)(BaseAngle)(float))

			((FSN_IS)(AnimationFinished)(bool))
			
			((FSN_GET_SET)(AnimationFrame)(int))
			((FSN_IS_SET)(Playing)(bool))
			((FSN_IS_SET)(Looping)(bool)) )

		virtual void Finish() = 0;

	private:
		virtual void SetImagePath(const std::string& value) = 0;
		virtual const std::string& GetImagePath() const = 0;
		virtual void SetAnimationPath(const std::string& value) = 0;
		virtual const std::string& GetAnimationPath() const = 0;

		virtual void SetAlignmentOrigin(clan::Origin origin) = 0;
		virtual clan::Origin GetAlignmentOrigin() const = 0;

		virtual void SetAlignmentOffset(const Vector2i& offset) = 0;
		virtual Vector2i GetAlignmentOffset() const = 0;

		virtual void SetRotationOrigin(clan::Origin origin) = 0;
		virtual clan::Origin GetRotationOrigin() const = 0;

		virtual void SetRotationOffset(const Vector2i& offset) = 0;
		virtual Vector2i GetRotationOffset() const = 0;

		virtual void SetColour(const clan::Colorf& val) = 0;
		virtual const clan::Colorf &GetColour() const = 0;

		virtual void SetAlpha(float val) = 0;
		virtual float GetAlpha() const = 0;

		virtual void SetScale(const Vector2& val) = 0;
		virtual Vector2 GetScale() const = 0;

		virtual void SetBaseAngle(float val) = 0;
		virtual float GetBaseAngle() const = 0;

		virtual bool IsAnimationFinished() const = 0;

		virtual void SetAnimationFrame(int val) = 0;
		virtual int GetAnimationFrame() const = 0;

		virtual void SetPlaying(bool val) = 0;
		virtual bool IsPlaying() const = 0;

		virtual void SetLooping(bool val) = 0;
		virtual bool IsLooping() const = 0;
	};

	class ICamera
	{
	public:
		static std::string GetTypeName() { return "ICamera"; }
		virtual ~ICamera()
		{}
		
		enum SyncTypes : uint8_t
		{
			NoSync = 0x00,
			Owned,
			Shared
		};

		FSN_COIFACE_PROPS(ICamera,
			((FSN_GET_SET)(SyncType)(SyncTypes))
			((FSN_IS_SET)(ViewportEnabled)(bool))
			((FSN_GET_SET)(ViewportRect)(clan::Rectf))
			((FSN_IS_SET)(AngleEnabled)(bool)) )

		virtual ViewportPtr GetViewport() const = 0;

	private:
		virtual void SetSyncType(SyncTypes value) = 0;
		virtual SyncTypes GetSyncType() const = 0;

		virtual void SetViewportEnabled(bool value) = 0;
		virtual bool IsViewportEnabled() const = 0;

		virtual void SetViewportRect(const clan::Rectf& value) = 0;
		virtual const clan::Rectf& GetViewportRect() const = 0;

		virtual void SetAngleEnabled(bool value) = 0;
		virtual bool IsAngleEnabled() const = 0;

	};

}

#endif
