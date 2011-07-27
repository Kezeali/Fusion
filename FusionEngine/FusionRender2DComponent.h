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

#ifndef H_FusionRender2DComponent
#define H_FusionRender2DComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionCommon.h"
#include "FusionEntityComponent.h"
#include "FusionThreadSafeProperty.h"

namespace FusionEngine
{

	FSN_BEGIN_COIFACE(IRenderCom)
	public:
		ThreadSafeProperty<Vector2> Offset;
		ThreadSafeProperty<int> LocalDepth;
		//ThreadSafeProperty<bool> Interpolate;

		void SynchroniseInterface()
		{
			if (Offset.Synchronise()) // writeonly
				SetOffset(Offset.Get());
			if (LocalDepth.Synchronise())
				SetLocalDepth(LocalDepth.Get());
			//if (Interpolate.Synchronise())
			//	SetInterpolate(Interpolate.Get());
		}

		void FireInterfaceSignals()
		{
			Offset.FireSignal();
			LocalDepth.FireSignal();
			//Interpolate.FireSignal();
		}

	private:
		virtual void SetOffset(const Vector2& value) = 0;
		
		virtual void SetLocalDepth(int value) = 0;

		//virtual void SetInterpolate(bool value) = 0;
	};

	class ISprite : public IRenderCom
	{
	public:
		static std::string GetTypeName() { return "ISprite"; }
		virtual ~ISprite()
		{}

		ThreadSafeProperty<std::string> ImagePath;
		ThreadSafeProperty<std::string> AnimationPath;

		ThreadSafeProperty<CL_Origin> AlignmentOrigin;
		ThreadSafeProperty<Vector2i> AlignmentOffset;
		ThreadSafeProperty<CL_Origin> RotationOrigin;
		ThreadSafeProperty<Vector2i> RotationOffset;
		ThreadSafeProperty<CL_Colorf> Colour;
		ThreadSafeProperty<float> Alpha;
		ThreadSafeProperty<Vector2> Scale;
		ThreadSafeProperty<float> BaseAngle;

		ThreadSafeProperty<bool, NullWriter<bool>> AnimationFinished;

		void SynchroniseInterface()
		{
			IRenderCom::SynchroniseInterface();

			if ((ImagePath.m_Changed && ImagePath.Synchronise(GetImagePath())) || ImagePath.Synchronise())
				SetImagePath(ImagePath.Get());
			if ((ImagePath.m_Changed && AnimationPath.Synchronise(GetAnimationPath())) || ImagePath.Synchronise())
				SetAnimationPath(AnimationPath.Get());

			FSN_SYNCH_PROP(AlignmentOrigin);
			FSN_SYNCH_PROP(AlignmentOffset);
			FSN_SYNCH_PROP(RotationOrigin);
			FSN_SYNCH_PROP(RotationOffset);
			FSN_SYNCH_PROP(Colour);
			FSN_SYNCH_PROP(Alpha);
			FSN_SYNCH_PROP(Scale);
			FSN_SYNCH_PROP(BaseAngle);

			if (AnimationFinished.m_Changed)
			AnimationFinished.Synchronise(IsAnimationFinished());
		}

		void FireInterfaceSignals()
		{
			IRenderCom::FireInterfaceSignals();
			ImagePath.FireSignal();
			AnimationPath.FireSignal();
			AlignmentOrigin.FireSignal();
			AlignmentOffset.FireSignal();
			RotationOrigin.FireSignal();
			RotationOffset.FireSignal();
			Colour.FireSignal();
			Alpha.FireSignal();
			Scale.FireSignal();
			BaseAngle.FireSignal();
			AnimationFinished.FireSignal();
		}

	private:
		virtual void SetImagePath(const std::string& value) = 0;
		virtual std::string GetImagePath() const = 0;
		virtual void SetAnimationPath(const std::string& value) = 0;
		virtual std::string GetAnimationPath() const = 0;

		virtual void SetAlignmentOrigin(CL_Origin origin) = 0;
		virtual CL_Origin GetAlignmentOrigin() const = 0;

		virtual void SetAlignmentOffset(const Vector2i& offset) = 0;
		virtual Vector2i GetAlignmentOffset() const = 0;

		virtual void SetRotationOrigin(CL_Origin origin) = 0;
		virtual CL_Origin GetRotationOrigin() const = 0;

		virtual void SetRotationOffset(const Vector2i& offset) = 0;
		virtual Vector2i GetRotationOffset() const = 0;

		virtual void SetColour(const CL_Colorf& val) = 0;
		virtual CL_Colorf GetColour() const = 0;

		virtual void SetAlpha(float val) = 0;
		virtual float GetAlpha() const = 0;

		virtual void SetScale(const Vector2& val) = 0;
		virtual Vector2 GetScale() const = 0;

		virtual void SetBaseAngle(float val) = 0;
		virtual float GetBaseAngle() const = 0;

		virtual bool IsAnimationFinished() const = 0;
	};

}

#endif
