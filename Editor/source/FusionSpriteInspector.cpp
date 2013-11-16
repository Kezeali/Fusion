/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionSpriteInspector.h"

#include "FusionInspectorTypeUtils.h"

namespace FusionEngine { namespace Inspectors
{

	void SpriteInspector::InitUI()
	{
		/*
		AddProperty("ImagePath", AddTextInput("Image",
			StringSetter_t([](std::string path, ComponentIPtr<ISprite> component) { component->ImagePath.Set(path); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return component->ImagePath.Get(); }),
			35
			));

		AddProperty("AnimationPath", AddTextInput("Animation",
			StringSetter_t([](std::string path, ComponentIPtr<ISprite> component) { component->AnimationPath.Set(path); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return component->AnimationPath.Get(); }),
			35
			));

		std::vector<std::string> originOptions;
		// WARNING: assumes origin_bottom_right is the last value in the enum
		for (int i = 0; i < (int)clan::origin_bottom_right; ++i)
			originOptions.push_back(TypeUtils::OriginToString((clan::Origin)i));

		// Alignment
		AddProperty("AlignmentOrigin", AddSelectInput("AlignmentOrigin",
			originOptions,
			StringSetter_t([](std::string str, ComponentIPtr<ISprite> component) { component->AlignmentOrigin.Set(TypeUtils::StringToOrigin(str)); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return TypeUtils::OriginToString(component->AlignmentOrigin.Get()); })
			));

		AddProperty("AlignmentOffset", AddTextInput("AlignmentOffset",
			IntSetter_t([](int x, ComponentIPtr<ISprite> component) { component->AlignmentOffset.Set(Vector2i(x, component->AlignmentOffset.Get().y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AlignmentOffset.Get().x; })
			));
		AddProperty("AlignmentOffset", AddTextInput("",
			IntSetter_t([](int y, ComponentIPtr<ISprite> component) { component->AlignmentOffset.Set(Vector2i(component->AlignmentOffset.Get().x, y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AlignmentOffset.Get().y; })
			));

		// Rotation Hotspot
		AddProperty("RotationOrigin", AddSelectInput("RotationOrigin",
			originOptions,
			StringSetter_t([](std::string str, ComponentIPtr<ISprite> component) { component->RotationOrigin.Set(TypeUtils::StringToOrigin(str)); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return TypeUtils::OriginToString(component->RotationOrigin.Get()); })
			));

		AddProperty("RotationOffset", AddTextInput("RotationOffset",
			IntSetter_t([](int x, ComponentIPtr<ISprite> component) { component->RotationOffset.Set(Vector2i(x, component->RotationOffset.Get().y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->RotationOffset.Get().x; })
			));
		AddProperty("RotationOffset", AddTextInput("",
			IntSetter_t([](int y, ComponentIPtr<ISprite> component) { component->RotationOffset.Set(Vector2i(component->RotationOffset.Get().x, y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->RotationOffset.Get().y; })
			));

		// Base-Angle
		AddProperty("BaseAngle", AddTextInput("BaseAngle",
			FloatSetter_t([](float angle, ComponentIPtr<ISprite> component) { component->BaseAngle.Set(angle); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->BaseAngle.Get(); })
			));

		// Scale
		AddProperty("Scale", AddTextInput("Scale",
			FloatSetter_t([](float x, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(x, component->Scale.Get().y)); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->Scale.Get().x; })
			));
		AddProperty("Scale", AddTextInput("",
			FloatSetter_t([](float y, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(component->Scale.Get().x, y)); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->Scale.Get().y; })
			));
		
		// Animation frame
		AddProperty("AnimationFrame", AddTextInput("AnimationFrame",
			IntSetter_t([](int frame, ComponentIPtr<ISprite> component) { component->AnimationFrame.Set(frame); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AnimationFrame.Get(); })
			));
			*/
	}

} }
